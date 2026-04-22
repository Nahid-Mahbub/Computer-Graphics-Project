# 🛏️ FPS Bedroom — Computer Graphics Project

A fully interactive **3D first-person bedroom** built from scratch using **OpenGL** and **GLUT** in C++. Walk through a furnished room, toggle lights, spin a ceiling fan, watch a pendulum clock swing, and open/close a door — all in real time.

---

## 📸 Preview

> *First-person walkable 3D bedroom scene with furniture, lighting, and animations*

```
Controls:
  W / A / S / D   →   Move around the room
  Mouse           →   Look around (FPS-style)
  1               →   Toggle ceiling light 1
  L               →   Toggle ceiling light 2
  2               →   Toggle lamp light
  3               →   Toggle ceiling fan
  O               →   Open door
  C               →   Close door
  4               →   Toggle AC
  ESC             →   Quit
```

---

## ✅ Project Requirements

| Requirement | Status | Details |
|---|---|---|
| Graphics Primitives | ✅ | `GL_QUADS`, `GL_TRIANGLES`, `GL_POLYGON`, `GL_LINE_STRIP`, Spheres |
| Graphics Algorithms | ✅ | DDA (OpenGL internal), Bresenham-style pendulum arc, custom normal vector algorithm |
| 2D Transformations | ✅ | Translation, Rotation, Scaling on all objects |
| Animated Object | ✅ | Ceiling fan, pendulum clock, door, FPS camera |
| Real-World Theme | ✅ | 3D bedroom interior scene |

---

## 🏗️ Features

### Room Objects
- **Bed** with pillows and headboard
- **Sofa** with decorative throw pillow
- **Wardrobe** with 4 drawers and handles
- **Cupboard** with door panels
- **Dressing table** with mirrors
- **Bedside drawer** with sphere handle
- **Indoor plant** in a pot (random leaf spheres)
- **Ceiling fan** with 4 blades, downrod, and motor hub
- **Pendulum clock** with swinging animation
- **Table lamp** with trapezoid shade
- **AC unit** with slats and airflow
- **Tiled floor** (30×30 checkerboard pattern)
- **Linkin Park poster** on the wall
- **Door** with wood planks, frame, and handle

### Lighting
- 3 independent light sources (ceiling light ×2, lamp)
- Toggle each light individually via keyboard
- Lamp shade uses `GL_EMISSION` to glow when on
- Smooth shading with surface normals (`GL_SMOOTH`)

### Camera
- Full FPS-style movement with WASD keys
- Mouse look with yaw and pitch control
- Delta-time based movement (same speed on any hardware)
- Collision detection — player stays inside room bounds

---

## 🔧 Graphics Primitives Used

| Primitive | Used For |
|---|---|
| `GL_QUADS` | Walls, floor tiles, ceiling, bed frame, furniture panels |
| `GL_TRIANGLES` | Pyramid shape (4 triangular faces) |
| `GL_POLYGON` | Custom arch shape for sofa top (11 vertices) |
| `GL_LINE_STRIP` | Arch outline as connected line segments |
| `glutSolidSphere` | Door knobs, fan motor hub, plant leaves, drawer handles |
| `gluCylinder` | Plant pot, plant trunk |
| `gluDisk` | Plant pot soil surface |

---

## 📐 Transformations Used

### Translation — `glTranslatef(x, y, z)`
Every object in the room is positioned using translation. Bed, sofa, wardrobe, lamp, fan, door — all placed at exact world coordinates.

### Rotation — `glRotatef(angle, ax, ay, az)`
- **Fan blades** → spin around Y-axis using `fanAngle`
- **Door** → opens/closes around Y-axis hinge with translate-rotate-translate trick
- **Sofa pillow** → rotated 15° yaw + 12° lean for a casual look
- **Plant pot** → rotated −90° on X-axis to orient cylinder upright

### Scaling — `glScalef(sx, sy, sz)`
The same base cube is reshaped into every furniture piece:
- `glScalef(5, 2, 0.1)` → thin flat wall
- `glScalef(0.12, 0.6, 0.4)` → wardrobe body
- `glScalef(0.08, 0.5, 0.08)` → thin fan downrod

---

## 🎞️ Animations

### Ceiling Fan
```cpp
fanAngle += 200.0f * dt;   // 200 degrees per second
glRotatef(fanAngle, 0, 1, 0);
```
Toggle with key `3`. Delta-time (`dt`) ensures smooth speed on all machines.

### Pendulum Clock
```cpp
theta += 0.2;   // swing right
z     -= 0.002; // arc path
// reverses at theta >= 210 and theta <= 150
```
Oscillates automatically between 150° and 210° tracing a realistic arc.

### Door
```cpp
doorAngle += 20.0f;  // key O: open
doorAngle -= 20.0f;  // key C: close
```
Uses pivot-correct hinge: translate to hinge → rotate → translate back.

---

## 🧮 Algorithms

### DDA (Digital Differential Analyzer)
OpenGL's internal rasterizer uses DDA to convert every edge defined by `glVertex3f()` calls into screen pixels by incrementally stepping along the line slope.

### Bresenham-style Incremental Arc (Pendulum)
The pendulum traces its arc through discrete incremental steps — `theta` and `z` update by fixed amounts each frame, mirroring Bresenham's philosophy of integer-step approximation.

### Normal Vector Calculation — `getNormal3p()` *(custom implementation)*
```cpp
// Lines 136–143 in main.cpp
GLfloat Ux = x2-x1, Uy = y2-y1, Uz = z2-z1;
GLfloat Vx = x3-x1, Vy = y3-y1, Vz = z3-z1;
glNormal3f(Uy*Vz - Uz*Vy,
           Uz*Vx - Ux*Vz,
           Ux*Vy - Uy*Vx);  // cross product
```
Takes 3 polygon vertices, computes two edge vectors, outputs the perpendicular surface normal. Called before every polygon to enable correct lighting.

---

## 🛠️ Build & Run

### Prerequisites
- OpenGL
- GLUT (or FreeGLUT)
- A C++ compiler (g++, clang++, MSVC)

### Linux / macOS
```bash
g++ main.cpp -o bedroom -lGL -lGLU -lglut -lm
./bedroom
```

### macOS (with Homebrew FreeGLUT)
```bash
g++ main.cpp -o bedroom -framework OpenGL -framework GLUT -Wno-deprecated
./bedroom
```

### Windows (MinGW)
```bash
g++ main.cpp -o bedroom.exe -lopengl32 -lglu32 -lfreeglut
bedroom.exe
```

---

## 📁 Project Structure

```
📦 fps-bedroom
 ┣ 📄 main.cpp        ← entire project source
 ┗ 📄 README.md
```

---

## 🧰 Built With

- **Language** — C++
- **Graphics API** — OpenGL (fixed-function pipeline)
- **Windowing / Input** — GLUT / FreeGLUT
- **Math** — `<math.h>` for sin/cos camera movement

---

## 👤 Author

**[Your Name]**
Department of Computer Science & Engineering
Course: Computer Graphics Lab

---

## 📄 License

This project was created for academic purposes as a Computer Graphics course project.
