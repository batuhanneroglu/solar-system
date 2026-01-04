cmake_minimum_required(VERSION 3.10)
project(SolarSystemSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenGL bulunuyor
find_package(OpenGL REQUIRED)

# GLFW ve GLEW için include ve lib dizinleri
# Bu yolları kendi sisteminize göre düzenlemelisiniz
set(GLFW_INCLUDE_DIR "C:/Libraries/glfw-3.4.bin.WIN64/include")
set(GLFW_LIB_DIR "C:/Libraries/glfw-3.4.bin.WIN64/lib-mingw-w64")
set(GLEW_INCLUDE_DIR "C:/Libraries/glew-2.1.0/include")
set(GLEW_LIB_DIR "C:/Libraries/glew-2.1.0/lib/Release/x64")
set(GLEW_DLL_DIR "C:/Libraries/glew-2.1.0/bin/Release/x64")
set(GLM_INCLUDE_DIR "C:/Libraries/glm")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/imgui")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${GLFW_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

link_directories(
    ${GLFW_LIB_DIR}
    ${GLEW_LIB_DIR}
)

# Kaynak dosyaları
file(GLOB SOURCES "src/*.cpp")

# ImGui kaynak dosyaları
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

# Kütüphaneleri bağla
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw3
    glew32
    gdi32
)

# Shader dosyalarını build dizinine kopyala
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# Texture klasörünü build dizinine kopyala
file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})

# GLEW DLL'i build dizinine kopyala
file(COPY ${GLEW_DLL_DIR}/glew32.dll DESTINATION ${CMAKE_BINARY_DIR})
