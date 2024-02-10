# Outdoor Rendering
Outdoor rendering using OpenGL and compute shaders.

## Description
Render an outdoor scene using OpenGL. The scene includes three types of plants and a slime creature that consumes the plants. The screen offers both a god view and a player view.

Here are some functions in this program:
- masking the transparent part of the plants
- phong shading
- culling the instance outside the view frustum with compute shader


## Getting Started
### Dependencies
- Windows
- It is recommended to use a dedicated graphics card for improved performance.

### Installing
```bash
git clone https://github.com/terrychou911019/OutdoorRendering.git
```

### Executing Program
Execute *"Rendering_Framework.exe"* under the *"Rendering_Framework"* folder.

**Camera View**
![image](https://hackmd.io/_uploads/S15hz0Nja.png)

The left side of the screen is the god view, while the right side is the player view.
- god view - use mouse to drag
- player view - use WASD to control
- slime - it will walk around and consume the plants 

