#include <dconf/dconf.h>
#include <glib.h>
#include <gio/gio.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include <filesystem>
#include <unistd.h>
#include <pwd.h>
#include <stdexcept>
#include <CLI/CLI.hpp>

#define debug

// Returns the user's home directory as a const char*
const char* homedir() {
    const char* home = std::getenv("HOME");
    if (home != nullptr) {
#ifdef debug
        std::cout << "home method 1\n";
#endif
        return home;
    } else {
#ifdef debug
        std::cout << "home method 2\n";
#endif
        struct passwd* pw = getpwuid(getuid());
        if (pw != nullptr) {
            return pw->pw_dir;
        } else {
            std::cerr << "Home directory environment variable is not available\n";
            return nullptr;
        }
    }
}

// Returns the username as a const char*
const char* username() {
    const char* user = std::getenv("USER");
    if (user == nullptr) {
        struct passwd* pw = getpwuid(getuid());
        if (pw != nullptr) {
            user = pw->pw_name;
        }
    }
    if (user != nullptr) {
        return user;
    } else {
        std::cerr << "Cannot retrieve username\n";
        return nullptr;
    }
}

// Executes a command and returns its output as a string
std::string exec(const char* cmd) {
    char buffer[128];
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    try {
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

// Determines if the theme is dark based on its name
bool isDarkTheme(const std::string& themeName) {
    std::string lowerTheme = themeName;
    for (char& c : lowerTheme) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return (lowerTheme.find("dark") != std::string::npos ||
            lowerTheme.find("night") != std::string::npos ||
            lowerTheme.find("black") != std::string::npos);
}

// Sets the desktop background based on the current theme, updating darkMode
int dconf_set(bool& darkMode, bool force) {
    GError* error = nullptr;
    DConfClient* client = dconf_client_new();
    if (!client) {
        std::cerr << "Error: Failed to initialize DConf client" << std::endl;
        return 1;
    }

    GSettings* settings = g_settings_new("org.gnome.desktop.interface");
    if (!settings) {
        std::cerr << "Error: Failed to initialize GSettings for org.gnome.desktop.interface" << std::endl;
        g_object_unref(client);
        return 1;
    }

    if (!force) {
        gchar* theme = g_settings_get_string(settings, "gtk-theme");
        if (!theme) {
            std::cerr << "Error: Failed to retrieve gtk-theme value" << std::endl;
            g_object_unref(settings);
            g_object_unref(client);
            return 1;
        }
        std::string themeName(theme);
        g_free(theme);
        darkMode = isDarkTheme(themeName);
    }

    const char* key = darkMode ? "/org/gnome/desktop/background/picture-uri-dark" : "/org/gnome/desktop/background/picture-uri";
    GVariant* value = dconf_client_read(client, key);
    if (value != nullptr) {
        const gchar* str_value = g_variant_get_string(value, nullptr);
        std::cout << "Current background: " << str_value << std::endl;
        g_variant_unref(value);
    } else {
        std::cerr << "Error: Failed to read current background key" << std::endl;
    }

    const char* homedr = homedir();
    if (!homedr) {
        std::cerr << "Error: Home directory not available" << std::endl;
        g_object_unref(settings);
        g_object_unref(client);
        return 1;
    }

    std::string config_image = "file://" + std::string(homedr) + (darkMode ? "/.config/background_dark" : "/.config/background");
    GVariant* new_value = g_variant_new_string(config_image.c_str());
    gboolean success = dconf_client_write_sync(client, key, new_value, nullptr, nullptr, &error);
    std::cout<<config_image<<std::endl;
    if (!success) {
        std::cerr << "Error writing key: " << (error ? error->message : "Unknown error") << std::endl;
        if (error) g_error_free(error);
        g_variant_unref(new_value);
    } else {
        std::cout << "Background set to: " << config_image << std::endl;
        g_variant_unref(new_value);
    }

    g_object_unref(settings);
    g_object_unref(client);
    return success ? 0 : 1;
}

int main(int argc, char* argv[]) {
    CLI::App app{"Wallpaper selector"};
    std::string img_file;
    bool dark = false, white = false;

    app.add_option("-i,--img", img_file, "Image file");
    app.add_flag("-d,--dark", dark, "Force dark theme mode");
    app.add_flag("-w,--white", white, "Force white theme mode");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    if (dark && white) {
        std::cerr << "Error: Cannot specify both dark and white modes simultaneously" << std::endl;
        return 1;
    }

    bool darkMode = false;
    bool force = (dark || white);
    if (force) {
        darkMode = dark;
    }

    if (dconf_set(darkMode, force)) {
        return 1;
    }

    const char* homedr = homedir();
    if (!homedr) {
        std::cerr << "Error: Home directory not available" << std::endl;
        return 1;
    }

    std::string config_image = std::string(homedr) + (darkMode ? "/.config/background_dark" : "/.config/background");
    if (!img_file.empty()) {
        try {
            std::filesystem::copy(img_file, config_image, std::filesystem::copy_options::overwrite_existing);
            std::cout << "Copied " << img_file << " to " << config_image << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error copying file: " << e.what() << std::endl;
            return 1;
        }
    } else {
        try {
            std::string image(exec("zenity --file-selection --title='Select an Image' "
                          "--file-filter='Image Files | *.png *.jpg *.jpeg' "
                          "--file-filter='All Files | *'"));
            std::filesystem::copy(image, config_image, std::filesystem::copy_options::overwrite_existing);
            std::cout << "Copied " << image << " to " << config_image << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error copying file: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}