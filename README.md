# OpenGL Code to Measure Point-to-Point Distance with PLY File

<p align="center">
  <img src="demo.gif" width="800"/>
</p>

## Controls

Once the program launches, an interactive 3D window will appear.

### Mouse Controls

- **Right Click (hold):** Rotate the 3D model  
- **Scroll Wheel:** Zoom in and out  
- **Left Click:** Select points  

### Measurement

After selecting **two points**:

- A line is drawn between the selected points in the 3D view
> ## 📏 Distance Output
> **The measured Euclidean distance is printed in the terminal window.**

---

## Script 1: OBJ_to_PLY.cpp

This script converts OBJ files into PLY format.

⚠️ **You must change this line inside the script** to match your OBJ file name:

```cpp
std::ifstream objFile("OBJ_FILE_NAME");
```

## Compilation Instructions

### macOS

```bash
clang++ OBJ_to_PLY.cpp -o plyOut
```
### Windows

```bash
clang++ OBJ_to_PLY.cpp -o plyOut.exe
```
### Run the executable

**macOS**
```bash
./plyOut
```

**Windows**
```bash
./plyOut.exe
```
---

## Script 2: `PLY_Mesh_GUI.cpp`

> This script turns the PLY file you uploaded into an interactive OpenGL window showing the point cloud.

You can rotate the model, zoom in/out, and select **two points** to compute their relative distance.

### Import PLY File

You must manually place your **PLY or OBJ file** inside the project directory.

⚠️ If you have an OBJ file, first run to convert into PLY file:

```bash
OBJ_to_PLY.cpp
```

## Compilation Instructions

### macOS

```bash
clang++ -std=c++17 -O2 -g \
PLY_Mesh_GUI.cpp glad/src/glad.c \
-Iglad/include \
-I/usr/local/include -L/usr/local/lib \
-lglfw \
-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo \
-o ply_viewer

```
### Windows

```bash
 clang++ -std=c++17 PLY_Mesh_GUI.cpp glad\src\glad.c -Iglad\include -Iglfw\include -I. -Lglfw\lib-vc2022 -lglfw3dll -lopengl32 -lgdi32 -luser32 -lkernel32 -o ply_viewer.exe
```

These commands will generate an executable named:

- `ply_viewer` on macOS  
- `ply_viewer.exe` on Windows  

### Run the executable

**macOS**
```bash
./ply_viewer
```

**Windows**
```bash
./ply_viewer.exe
```

## Controls

Once the program launches, an interactive 3D window will appear.

### Mouse Controls

- **Right Click (hold):** Rotate the 3D model  
- **Scroll Wheel:** Zoom in and out  
- **Left Click:** Select points  

### Measurement

After selecting **two points**:

- A line is drawn between the selected points in the 3D view
> 📏 **The measured Euclidean distance is printed in the terminal window.**

