# IntegerWorld

**IntegerWorld** is a C++11 embedded graphics framework for efficient 2D and 3D rendering using integer math. It provides APIs for drawing and shading window-clipped points, lines, triangles, and rectangles, and interfaces with platform-independent targets such as framebuffers and direct draw.

## Render Features

- Arduino compatible
- Direct 2D and 3D drawing API
- Integer-only math for fast, deterministic rendering
- Window clipping for all primitives
- Custom pixel shader support for lines, triangles and rectangles
- Platform-independent output via `IOutputSurface` interface
