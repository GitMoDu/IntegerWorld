# IntegerWorld

**IntegerWorld** is a C++11 embedded graphics framework for efficient 2D and 3D rendering using integer math. It provides APIs for drawing and shading window-clipped points, lines, triangles, and rectangles, and interfaces with platform-independent targets such as framebuffers and direct draw.

![Tiny demo scene](https://github.com/GitMoDu/IntegerWorld/raw/master/media/tiny_demo_scene_avr.gif)

*Tiny Demo Scene running at ~33 fps on an Arduino AVR328 @ 16 Mhz*


## Library
- Arduino compatible.
- [Integer-only math](https://github.com/GitMoDu/IntegerSignal) for fast rendering with low memory use.

### 3D Engine
- [Co-operative scheduler](https://github.com/arkhipenko/TaskScheduler) compatible render engine.
- Rendering is done with Z-ordered painter's algorithm, with fragment collection and sorting approach.
- Scene shading based on fragment color, position, normal, material and lights.
- Material properties include Emissive, Diffuse, Specular and Metallic.
- Included light sources: Point, Directional and Spotlight.
- Included render objects: TriangleMesh, EdgeLine and PointCloud.

### Rasterizer:
  - Direct 2D and 3D drawing API for lines, triangles and rectangles.
  - Platform-independent output (screen, framebuffer).
  - Window clipping for all primitives, in 2D and 3D.
  - Custom pixel shader support for lines and triangles.


## Dependencies
TaskScheduler: https://github.com/arkhipenko/TaskScheduler

IntegerSignal: https://github.com/GitMoDu/IntegerSignal


