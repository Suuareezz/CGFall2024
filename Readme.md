## README: Spiky Icosphere with Mouse Rotation and Light Control

### Overview
This project implements a **Spiky Icosphere** in OpenGL, allowing the user to rotate the scene using the mouse, control zoom levels, and toggle light movement. The light source can either be automatically rotated around the object or manually moved using keyboard controls.

### Features
- **Interactive 3D spiky icosphere**: A geometrical icosphere with spikes extending from its surface.
- **Mouse-controlled rotation**: Rotate the icosphere in real-time by dragging with the mouse.
- **Zoom controls**: Zoom in and out using keyboard input.
- **Dynamic light movement**: Toggle automatic light movement around the icosphere or manually control the light position using the keyboard.
- **Light indicator**: A yellow sphere representing the light source.

### Dependencies
- OpenGL
- GLUT (OpenGL Utility Toolkit)
- GLEW (if `USEGLEW` is defined)

### Controls
- **Zoom**:
  - `z`: Zoom in
  - `x`: Zoom out
- **Light Movement**:
  - `l`: Toggle automatic light movement
  - `w`: Move light upwards (manual mode)
  - `s`: Move light downwards (manual mode)
  - `a`: Move light left (manual mode)
  - `d`: Move light right (manual mode)
- **Mouse**:
  - Left-click and drag to rotate the icosphere.
  
- **Exit**:
  - `ESC`: Exit the program

### File Descriptions
- `hw5.c`: The main source file containing all logic for rendering the spiky icosphere, handling user input, and managing lighting.
  
### Code Explanation
- **Icosphere Drawing**: The icosphere is drawn by defining 12 vertices and 20 triangular faces. Each triangle is extended outward to create spikes using the normal vector.
- **Lighting**: The light is represented as a small yellow sphere that moves around the object. Automatic movement rotates the light in a circular path, while manual controls adjust the light’s position.
- **Mouse Rotation**: The icosphere can be rotated by clicking and dragging the mouse, allowing for interactive exploration of the object.


## Compilation and Execution

### Windows
If using MinGW on Windows, the Makefile automatically links the required libraries:
```bash
make
./hw5.exe
```

### macOS
On macOS, the Makefile uses the appropriate OpenGL and GLUT frameworks:
```bash
make
./hw5
```

### Linux/Unix
On Linux or other Unix-based systems, the Makefile links against the necessary OpenGL libraries:
```bash
make
./hw5
```

## How to Run
1. Compile the code using the provided `Makefile`.
2. Run the executable (`./hw5`).
3. Use the keyboard controls to interact with the scene and manipulate the light source.

