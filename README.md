# General pathtracer on CPU

Warning, the project is young and in development, there are BUGS and errors ...

## Applications
- lil_tracer
- brdf_viewer
- envmap_sampling
- convergence

# Packages requirements
- [glm](https://github.com/g-truc/glm)
- [glfw](https://github.com/glfw/glfw)
- [embree3](https://www.embree.org/)
- [imgui](https://github.com/ocornut/imgui)
- [implot](https://github.com/epezent/implot)
- [miniz](https://github.com/richgel999/miniz)
- [nlohmann-json](https://github.com/nlohmann/json)
- [fast_obj](https://github.com/thisistherk/fast_obj)
- [tinyexr](https://github.com/syoyo/tinyexr)

fast_obj and tiny_exr are already inside the depo.

# Windows install

## Install packages using [vcpkg](https://vcpkg.io/en/)

I recommand using vcpkg to download the others packages.
Once vcpkg installed, and available in the path, you can run the following commmand :
```c
./install_packages_windows.bat
```
Note that Embree takes forever to download, compile, and install.

# Run on Windows (powershell)
```
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=./path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

# Linux install

## Install packages using [vcpkg](https://vcpkg.io/en/)

The same as for Windows, you need to install vcpkg. The you can run the following command :
```c
./install_packages_windows.sh
```
or install each packages by hand.

# Run on Linux

```
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=./path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```