# Wallpaper selector 
> Wallpaper selector writen on c++

Must work on gnome and kde plasma(plasma wasn't tested)
## How to Build
 ```bash
 make all
 ```
## Examples
#### To set using graphical selector:
```cpp
$ wallpaper 
```
#### To set using path to image as argument 
```cpp
$ wallpaper --img /home/$USER/Pictures/wallpapers/anime/20250325_225213.jpg
```

## Usage
```bash
$ wallpaper --help  
Wallpaper selector 


.build/wallpaper [OPTIONS]


OPTIONS:
  -h,     --help              Print this help message and exit 
  -i,     --img TEXT          Image file 
  -d,     --dark              Force set wallpaper for dark theme 
  -w,     --white             Force set wallpaper for white theme mode 
```
