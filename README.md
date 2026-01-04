# Solar System Simulation

A real-time 3D solar system simulation built with C++ and OpenGL. Features realistic planetary orbits, textures, interactive camera, and planet tracking system.

## Features

- Full 3D rendering of the Sun, 8 planets, and Earth's Moon
- Planet textures with Earth night lights showing cities
- Visible orbital paths for all celestial bodies
- Phong lighting with bloom effect on the Sun
- Background stars
- Free-roaming camera with mouse look
- Click any planet to follow it automatically
- Time control slider to speed up or slow down orbits
- Borderless fullscreen window
- ImGui menu interface

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

## Technical Info

- Graphics: OpenGL 3.3 Core Profile
- Rendering: Forward rendering with Phong lighting
- Post-processing: HDR framebuffer with bloom
- Physics: Simplified circular orbits for visual effect
- UI: ImGui 1.90.1

## License

Open source project for educational and personal use, NO COMMERCIAL PURPOSE.

## Credits

Built with OpenGL, GLFW, GLEW, GLM, ImGui, and stb_image.

Planet textures provided by Solar System Scope (https://www.solarsystemscope.com/textures/) under CC BY 4.0 license.
