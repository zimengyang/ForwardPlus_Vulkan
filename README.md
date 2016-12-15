Forward Plus Rendering in Vulkan
================

**Final Project of CIS 565: GPU Programming and Architecture, University of Pennsylvania**

* [Liang Peng](https://github.com/itoupeter), [Zimeng Yang](https://github.com/zimengyang)
* Tested on: Windows 10, i7-4850 @ 2.3GHz 16GB, GT 750M (Personal Laptop)

# Overview

![overview](./img/screenshots/overview.jpg)

### Demo Video (todo)
![video]()

# Debug Views

|Light Heat Map|
|------|
|![heat_map](./img/screenshots/lightHeatMap.jpg)|

|Texture Map|Normal Map|
|------|------|
|![texmap](./img/screenshots/textureMap.jpg)|![norm_map](./img/screenshots/normalMap.jpg)|

|Specular Map|Depth Texture|
|------|------|
|![specularmap](./img/screenshots/specularMap.jpg)|![depthTex](./img/screenshots/depthTexture.jpg)|

|Geometry Normal | After Normal Mapping|
|------|------|
|![geom_norm](./img/screenshots/geomNormal.jpg)|![mapped_norm](./img/screenshots/mappedNormal.jpg)|


# Forward Plus Rendering

### Forward+ Overview
Forward+ improves upon regular forward rendering by first determining which lights are overlapping which area in screen space. During the shading phase, only the lights that are potentially overlapping the current fragment need to be considered.

### Main Procedures

*Forward Plus = forward + light culling*

Basically, in order to implement a light culling, we need to compute Grid Frustums to cull the lights into the screen space tiles.

### Grid Frustums

The screen is divided into a number of square tiles, for example, 16 * 16 screen pixels.

![Tile Frustum](./img/readme/Tile-Frustum1.png)

The above image shows that the camera’s position (eye) is the origin of the frustum and the corner points of the tile denote the frustum corners. With this information, we can compute the planes of the tile frustum.

### Grid Frustums Compute Shader

We need compute shaders to compute and grid frustums for each screen tile.

### Light culling

In the next step of the Forward+ rendering technique is to cull the lights using the grid frustums that were computed in the previous section.

Basic algorithm for light culling:

1. Compute the min and max depth values in view space for the tile
2. Cull the lights and record the lights into a light index list
3. Copy the light index list into global memory

### Frustum culling

**Frustum-sphere** for points lights, **Frustum-cone** for spot lights.


# Vulkan

# Performance Analysis

### Tile Size

![tile_size](./data/tile_size.png)

todo

### Number of Lights

![num_lights](./data/number_of_lights.png)

todo

# Milestones 
### 11/21 - Basic Vulkan Application Framework
  * Vulkan environment setup and initialization
  * Basic rendering pipelines (multiple)
  * Vertex and index buffers (multiple)
  * Texture mapping (multiple) and model loading
  * Basic camera control
  * [Presentation Slides](./img/slides/milestone1.pdf)

### 11/28 - Basic Lighting in Vulkan / Debug Views
  * Debug views: depth, normal
  * Basic lighting: Lambert, Blinn-Phong
  * Compute pipeline in progress
  * [Presentation Slides](./img/slides/milestone2.pdf)

### 12/12 - Forward+
  * Compute pipeline
    * Compute grid frustum
    * Compute light list
  * Light culling
  * Shading
  * [Presentation Slides](./img/slides/milestone3.pdf)

# References:
1. [Vulkan Tutorial](https://vulkan-tutorial.com/)
2. [Vulkan Examples](https://github.com/SaschaWillems/Vulkan)
3. [Forward Plus Rendering](http://www.3dgep.com/forward-plus/)
4. [Graphics Models](http://graphics.cs.williams.edu/data/meshes.xml)
