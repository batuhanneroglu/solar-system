# Solar System Simulation

A real-time 3D solar system simulation built with C++ and OpenGL. Features realistic planetary orbits, textures, interactive camera, and planet tracking system.

## Features

- Full 3D rendering of the Sun, 8 planets, and Earth's Moon
- Planet textures with Earth night lights showing cities
- Visible orbital paths for all celestial bodies
- Phong lighting with bloom effect on the Sun
- 3000 background stars
- Free-roaming camera with mouse look
- Click any planet to follow it automatically
- Time control slider to speed up or slow down orbits
- Borderless fullscreen window
- ImGui menu interface

## How to Build and Run

### What You Need

Before building, you need to install these libraries on your system:

**Required Libraries:**
- GLFW 3.4 (window and input)
- GLEW 2.1.0 (OpenGL extensions)
- GLM (math library)

**Build Tools:**
- CMake 3.10 or later
- A C++ compiler (MinGW-w64 or Visual Studio)

### Installing Dependencies

**Step 1: Download the libraries**

Create a `C:\Libraries` folder and download these:

1. **GLEW 2.1.0**
   - Download: https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0-win32.zip
   - Extract to: `C:\Libraries\glew-2.1.0`

2. **GLFW 3.4**
   - Download: https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip
   - Extract to: `C:\Libraries\glfw-3.4.bin.WIN64`

3. **GLM**
   - Download: https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
   - Extract to: `C:\Libraries\glm`

After extracting, your folder structure should look like:
```
C:\Libraries\
    ├── glew-2.1.0\
    ├── glfw-3.4.bin.WIN64\
    └── glm\
```

**Step 2: Install build tools**

Choose one of these options:

**Option A: w64devkit (Simpler, Recommended)**
1. Download: https://github.com/skeeto/w64devkit/releases
2. Extract to `C:\w64devkit`
3. Open the `w64devkit.exe` terminal from that folder
4. You now have GCC, CMake, and Make ready to use

**Option B: Visual Studio**
1. Download Visual Studio 2019 or later
2. During installation, select "Desktop development with C++"
3. Make sure CMake is included (it comes with Visual Studio)

### Building the Project

**If using w64devkit:**

1. Open `w64devkit.exe` from `C:\w64devkit`
2. Navigate to the project folder:
   ```bash
   cd /c/path/to/SolarSystemSimulation
   ```
3. Create and enter build directory:
   ```bash
   mkdir build
   cd build
   ```
4. Run CMake and build:
   ```bash
   cmake ..
   mingw32-make
   ```
5. Run the program:
   ```bash
   ./SolarSystemSimulation.exe
   ```

**If using Visual Studio:**

1. Open Command Prompt or PowerShell
2. Navigate to project folder:
   ```bash
   cd C:\path\to\SolarSystemSimulation
   ```
3. Create and enter build directory:
   ```bash
   mkdir build
   cd build
   ```
4. Run CMake and build:
   ```bash
   cmake ..
   cmake --build . --config Release
   ```
5. Run the program:
   ```bash
   Release\SolarSystemSimulation.exe
   ```

**Quick build (if everything is already set up):**

Just double-click the `build.bat` file in the project folder. This will automatically build and run the simulation.

## Controls

**Camera Movement:**
- W/S - Move forward/backward
- A/D - Move left/right  
- Space - Move up
- Shift - Move down
- Mouse - Look around
- Scroll Wheel - Zoom in/out

**Planet Interaction:**
- Left Click - Select planet at crosshair (auto-enables tracking)
- Right Click - Deselect planet (exit tracking mode)
- Escape - Exit tracking mode

**Interface:**
- Tab - Open/close menu

**When menu is open:**
- Time slider - Control simulation speed (0x to 5x)
- Follow mode checkbox - Toggle camera tracking
- Clear selection button - Deselect current planet

## Common Issues

**"Cannot find GLFW" or "Cannot find GLEW" error:**
- Make sure you extracted the libraries to exactly `C:\Libraries\`
- Check that folder names match: `glew-2.1.0`, `glfw-3.4.bin.WIN64`, `glm`
- If you put them somewhere else, edit the paths in `CMakeLists.txt`

**Black screen when running:**
- Your graphics card might not support OpenGL 3.3. Update your graphics drivers.

**"glew32.dll not found" when running:**
- The DLL should be automatically copied to the build folder by CMake
- If not, manually copy it from `C:\Libraries\glew-2.1.0\bin\Release\x64\glew32.dll`

**Compilation errors:**
- Make sure you're using C++17 or later (should be default with modern compilers)
- Try deleting the `build/` folder and building again from scratch

**Slow performance:**
- Lower the star count in `src/main.cpp` (search for 3000, change to 1000)
- Reduce window resolution in `main.cpp` (SCR_WIDTH and SCR_HEIGHT variables)
- Your graphics card might be too old for the bloom effects

## Project Structure

```
SolarSystemSimulation/
├── src/
│   ├── main.cpp              - Main program with rendering loop
│   └── stb_image_impl.cpp    - Image loading implementation
├── include/
│   ├── Camera.h              - Free camera system
│   ├── Sphere.h              - Sphere mesh generation
│   ├── CelestialBody.h       - Planet data structure
│   └── stb_image.h           - Image loading header
├── shaders/
│   ├── vertex_shader.glsl    - Vertex transformation
│   ├── fragment_shader.glsl  - Lighting and texture mapping
│   ├── bloom_shader.glsl     - Bloom post-processing
│   ├── blur_shader.glsl      - Gaussian blur
│   └── screen_vertex.glsl    - Fullscreen quad shader
├── imgui/                     - Dear ImGui library
├── textures/                  - Planet textures (add your own)
├── build/                     - Compiled output (created by build)
├── CMakeLists.txt            - Build configuration
└── build.bat                 - Quick build script
```

## Technical Info

- Graphics: OpenGL 3.3 Core Profile
- Rendering: Forward rendering with Phong lighting
- Post-processing: HDR framebuffer with bloom
- Physics: Simplified circular orbits for visual effect
- UI: Dear ImGui 1.90.1

## License

Open source project for educational and personal use.

## Credits

Built with OpenGL, GLFW, GLEW, GLM, Dear ImGui, and stb_image.

Planet textures provided by Solar System Scope (https://www.solarsystemscope.com/textures/) under CC BY 4.0 license.
