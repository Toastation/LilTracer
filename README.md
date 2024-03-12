# General pathtracer on CPU

## Applications
- lil_tracer
- brdf_viewer
- envmap_sampling

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

# Windows install

## Install packages using [vcpkg](https://vcpkg.io/en/)

```vcpkg install glm:x64-windows nlohmann-json:x64-windows embree3:x64-windows glfw3:x64-windows imgui[core,opengl3-binding,glfw-binding]:x64-windows implot:x64-windows miniz:x64-windows```
 

## TODO
- [ ] Path integrator
- [ ] Sampling windows
- [ ] Performance evaluation
- [ ] Brdf python binding

