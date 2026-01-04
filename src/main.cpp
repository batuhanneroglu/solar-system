#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include "Camera.h"
#include "Sphere.h"
#include "CelestialBody.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Ayarlar
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Kamera
Camera camera(glm::vec3(0.0f, 150.0f, 400.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Zamanlama
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Zaman hızlandırma (görsel mod - hızlı animasyon)
float timeScale = 0.5f;

// UI ve gezegen takibi
bool showMenu = false;
int selectedPlanetIndex = -1;
bool followMode = false;
glm::vec3 followOffset(0.0f, 20.0f, 50.0f);
float glowPulse = 0.0f;
std::vector<CelestialBody*> celestialBodies;  // Global erişim için

// Mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);
std::string loadShaderSource(const char* filePath);
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath);

// Texture yükleme fonksiyonu
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        std::cout << "Texture yüklendi: " << path << " (" << width << "x" << height << ")" << std::endl;
    } else {
        std::cout << "Texture yüklenemedi: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }
    
    return textureID;
}

// Yörünge çizgisi oluşturma fonksiyonu
void createOrbitLine(float radius, GLuint& VAO, GLuint& VBO, int& vertexCount) {
    const int segments = 200; // Daha düzgün çember için
    vertexCount = segments + 1;
    std::vector<float> vertices;
    
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back(x);
        vertices.push_back(0.0f); // Y = 0 (yatay düzlem)
        vertices.push_back(z);
    }
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

int main() {
    // GLFW başlatma
    if (!glfwInit()) {
        std::cerr << "GLFW başlatılamadı!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // borderless window

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "solar system simulation", NULL, NULL);
    if (!window) {
        std::cerr << "Pencere oluşturulamadı!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW başlatma
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW başlatılamadı!" << std::endl;
        return -1;
    }

    // ImGui başlatma
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Modern ImGui Stili
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.ChildRounding = 12.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowPadding = ImVec2(15, 15);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(12, 8);
    
    // Modern renkler
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.13f, 0.95f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.22f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.35f, 0.50f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.20f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.23f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 0.90f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.35f, 0.50f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.20f, 0.30f, 1.00f);
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // Shader programlarını yükle
    GLuint shaderProgram = createShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    GLuint blurShader = createShaderProgram("shaders/screen_vertex.glsl", "shaders/blur_shader.glsl");
    GLuint bloomShader = createShaderProgram("shaders/screen_vertex.glsl", "shaders/bloom_shader.glsl");

    // Küre geometrisi oluştur
    Sphere sphere(1.0f, 30, 30);

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.vertices.size() * sizeof(float), sphere.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indices.size() * sizeof(unsigned int), sphere.indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // create starfield - points scattered in distant space
    std::vector<float> stars;
    srand(42); // same stars every time
    for (int i = 0; i < 500; i++) {
        // spread stars far from origin
        float x = (rand() % 40000 - 20000) / 10.0f;
        float y = (rand() % 40000 - 20000) / 10.0f;
        float z = (rand() % 40000 - 20000) / 10.0f;
        float brightness = 0.4f + (rand() % 100) / 100.0f * 0.6f;
        
        stars.push_back(x);
        stars.push_back(y);
        stars.push_back(z);
        stars.push_back(brightness);
        stars.push_back(brightness);
        stars.push_back(brightness);
    }

    GLuint starsVAO, starsVBO;
    glGenVertexArrays(1, &starsVAO);
    glGenBuffers(1, &starsVBO);
    
    glBindVertexArray(starsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, starsVBO);
    glBufferData(GL_ARRAY_BUFFER, stars.size() * sizeof(float), stars.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glPointSize(1.5f);

    // Bloom için framebuffer'lar
    GLuint hdrFBO, colorBuffers[2];
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer hatası!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Ping-pong framebuffers for blur
    GLuint pingpongFBO[2], pingpongBuffer[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
    }
    
    // Screen quad VAO
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };
    
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // solar system setup - using miniature scale for visibility
    celestialBodies.clear();

    // sun at center
    celestialBodies.push_back(new CelestialBody(
        "sun",
        1.989e30f,
        6.96e8f,
        12.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f),
        0.5f,
        true
    ));

    // mercury
    celestialBodies.push_back(new CelestialBody(
        "mercury",
        3.285e23f,
        2.4397e6f,
        3.0f,
        glm::vec3(40.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 2.0f),
        glm::vec3(0.7f, 0.7f, 0.7f),
        1.0f,
        false
    ));

    // venus
    celestialBodies.push_back(new CelestialBody(
        "venus",
        4.867e24f,
        6.0518e6f,
        5.0f,
        glm::vec3(70.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.6f),
        glm::vec3(1.0f, 0.8f, 0.6f),
        0.8f,
        false
    ));

    // earth
    celestialBodies.push_back(new CelestialBody(
        "earth",
        5.972e24f,
        6.371e6f,
        5.5f,
        glm::vec3(100.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.3f),
        glm::vec3(0.2f, 0.4f, 0.8f),
        1.2f,
        false
    ));

    // mars
    celestialBodies.push_back(new CelestialBody(
        "mars",
        6.39e23f,
        3.3895e6f,
        4.0f,
        glm::vec3(135.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.9f, 0.3f, 0.1f),
        1.1f,
        false
    ));

    // jupiter
    celestialBodies.push_back(new CelestialBody(
        "jupiter",
        1.898e27f,
        6.9911e7f,
        9.0f,
        glm::vec3(200.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.7f),
        glm::vec3(0.8f, 0.7f, 0.6f),
        1.5f,
        false
    ));

    // saturn
    celestialBodies.push_back(new CelestialBody(
        "saturn",
        5.683e26f,
        5.8232e7f,
        8.0f,
        glm::vec3(280.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.5f),
        glm::vec3(0.9f, 0.8f, 0.6f),
        1.3f,
        false
    ));

    // uranus
    celestialBodies.push_back(new CelestialBody(
        "uranus",
        8.681e25f,
        2.5362e7f,
        6.0f,
        glm::vec3(360.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.4f),
        glm::vec3(0.5f, 0.8f, 0.9f),
        0.9f,
        false
    ));

    // neptune
    celestialBodies.push_back(new CelestialBody(
        "neptune",
        1.024e26f,
        2.4622e7f,
        6.0f,
        glm::vec3(440.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.3f),
        glm::vec3(0.3f, 0.4f, 0.9f),
        0.8f,
        false
    ));

    // moon orbiting earth
    celestialBodies.push_back(new CelestialBody(
        "moon",
        7.342e22f,
        1.7371e6f,
        1.5f,                                // Ay'ın boyutu
        glm::vec3(15.0f, 0.0f, 0.0f),       // Dünya'ya göre başlangıç pozisyonu
        glm::vec3(0.0f, 0.0f, 5.0f),        // Daha hızlı yörünge hareketi
        glm::vec3(0.7f, 0.7f, 0.7f),        // Gri renk
        15.0f,                               // Dünya'ya olan uzaklık (orbitRadius)
        false
    ));

    std::cout << "=== solar system simulation ===" << std::endl;
    std::cout << std::endl;
    std::cout << "loaded celestial bodies:" << std::endl;
    for (const auto& body : celestialBodies) {
        std::cout << "  - " << body->name << " (size: " << body->displayRadius << ")" << std::endl;
    }
    
    // load textures for planets
    std::cout << std::endl;
    std::cout << "loading textures..." << std::endl;
    
    const char* textureFiles[] = {
        "textures/sun.jpg",
        "textures/mercury.jpg",
        "textures/venus.jpg",
        "textures/earth.jpg",
        "textures/mars.jpg",
        "textures/jupiter.jpg",
        "textures/saturn.jpg",
        "textures/uranus.jpg",
        "textures/neptune.jpg"
    };
    
    for (size_t i = 0; i < celestialBodies.size() && i < 9; i++) {
        GLuint tex = loadTexture(textureFiles[i]);
        if (tex != 0) {
            celestialBodies[i]->textureID = tex;
            celestialBodies[i]->hasTexture = true;
        }
    }
    
    // load earth night texture (city lights)
    GLuint earthNightTexture = loadTexture("textures/earth_night.jpg");
    if (earthNightTexture != 0) {
        std::cout << "earth night texture loaded!" << std::endl;
    }
    
    // load moon texture
    GLuint moonTexture = loadTexture("textures/moon.jpg");
    if (moonTexture != 0) {
        celestialBodies[9]->textureID = moonTexture;
        celestialBodies[9]->hasTexture = true;
        std::cout << "moon texture loaded!" << std::endl;
    }
    
    // create orbital paths for visualization
    std::cout << std::endl << "creating orbit lines..." << std::endl;
    
    struct OrbitLine {
        GLuint VAO, VBO;
        int vertexCount;
        float radius;
    };
    
    std::vector<OrbitLine> orbitLines;
    
    // orbital radii for planets (indices 1-8, sun is 0)
    float orbitRadii[] = {40.0f, 70.0f, 100.0f, 135.0f, 200.0f, 280.0f, 360.0f, 440.0f};
    
    for (int i = 0; i < 8; i++) {
        OrbitLine orbit;
        orbit.radius = orbitRadii[i];
        createOrbitLine(orbit.radius, orbit.VAO, orbit.VBO, orbit.vertexCount);
        orbitLines.push_back(orbit);
        std::cout << "  - orbit " << (i+1) << " created (radius: " << orbit.radius << ")" << std::endl;
    }
    
    // moon's orbit around earth
    OrbitLine moonOrbit;
    moonOrbit.radius = 15.0f;
    createOrbitLine(moonOrbit.radius, moonOrbit.VAO, moonOrbit.VBO, moonOrbit.vertexCount);
    orbitLines.push_back(moonOrbit);
    std::cout << "  - moon orbit created (radius: " << moonOrbit.radius << ")" << std::endl;
    
    std::cout << std::endl;
    std::cout << "Kontroller:" << std::endl;
    std::cout << "  W/A/S/D - İleri/Sol/Geri/Sağ hareket" << std::endl;
    std::cout << "  Space/Shift - Yukarı/Aşağı hareket" << std::endl;
    std::cout << "  Mouse - Kamera yönü" << std::endl;
    std::cout << "  +/- - Zaman hızını artır/azalt" << std::endl;
    std::cout << "  ESC - Çıkış" << std::endl;
    std::cout << std::endl;

    // Render döngüsü
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        
        // pulsing glow animation for selected planets
        glowPulse = 0.5f + 0.5f * sin(currentFrame * 3.0f);
        
        // follow mode - camera orbits and looks at selected planet
        if (followMode && selectedPlanetIndex >= 0 && selectedPlanetIndex < celestialBodies.size()) {
            glm::vec3 planetPos = celestialBodies[selectedPlanetIndex]->position;
            
            // calculate desired camera position (behind and above planet)
            float distance = 80.0f;
            float height = 30.0f;
            glm::vec3 desiredPos = planetPos + glm::vec3(0.0f, height, distance);
            
            // smoothly move camera to desired position
            float smoothSpeed = 5.0f * deltaTime;
            camera.Position = glm::mix(camera.Position, desiredPos, smoothSpeed);
            
            // calculate look direction from camera to planet
            glm::vec3 toTarget = planetPos - camera.Position;
            float distToTarget = glm::length(toTarget);
            
            if (distToTarget > 0.01f) {
                toTarget = glm::normalize(toTarget);
                
                // calculate yaw and pitch from direction vector
                float yaw = glm::degrees(atan2(toTarget.z, toTarget.x));
                float pitch = glm::degrees(asin(glm::clamp(toTarget.y, -1.0f, 1.0f)));
                
                // smoothly interpolate camera angles
                camera.Yaw = yaw;
                camera.Pitch = pitch;
                
                // update camera's front vector based on new angles
                camera.updateCameraVectors();
            }
        }

        // update physics for all bodies
        for (size_t i = 0; i < celestialBodies.size(); i++) {
            if (i == 9) {
                // moon orbits earth (earth is at index 3)
                float moonOrbitSpeed = 5.0f * deltaTime * timeScale;
                static float moonOrbitAngle = 0.0f;
                moonOrbitAngle += moonOrbitSpeed;
                
                glm::vec3 earthPos = celestialBodies[3]->position;
                float moonRadius = 15.0f;
                
                celestialBodies[9]->position.x = earthPos.x + moonRadius * cos(moonOrbitAngle);
                celestialBodies[9]->position.y = earthPos.y;
                celestialBodies[9]->position.z = earthPos.z + moonRadius * sin(moonOrbitAngle);
                
                // moon's own rotation
                celestialBodies[9]->rotationAngle += celestialBodies[9]->rotationSpeed * deltaTime * timeScale;
            } else {
                celestialBodies[i]->update(deltaTime * timeScale, celestialBodies);
            }
        }
        
        // ImGui frame başlat
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // HDR framebuffer'a render et
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render sahne başlangıcı
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);  // Siyah uzay
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View/projection transforms
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.Position));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(celestialBodies[0]->position));

        // draw background stars
        glBindVertexArray(starsVAO);
        glm::mat4 starsModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(starsModel));
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 1.0f, 1.0f);
        glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), true);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);
        glUniform1i(glGetUniformLocation(shaderProgram, "isSelected"), false);
        glDrawArrays(GL_POINTS, 0, 3000);

        // draw orbital paths in white
        glm::mat4 orbitModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(orbitModel));
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 1.0f, 1.0f);
        glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), false);
        glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);
        
        // draw planet orbits (indices 0-7)
        for (int i = 0; i < 8; i++) {
            glBindVertexArray(orbitLines[i].VAO);
            glDrawArrays(GL_LINE_LOOP, 0, orbitLines[i].vertexCount);
        }
        
        // draw moon's orbit at earth position (index 8)
        glm::mat4 moonOrbitModel = glm::mat4(1.0f);
        moonOrbitModel = glm::translate(moonOrbitModel, celestialBodies[3]->position);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(moonOrbitModel));
        glBindVertexArray(orbitLines[8].VAO);
        glDrawArrays(GL_LINE_LOOP, 0, orbitLines[8].vertexCount);
        
        // render all celestial bodies
        glBindVertexArray(VAO);
        for (size_t idx = 0; idx < celestialBodies.size(); idx++) {
            auto& body = celestialBodies[idx];
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, body->position);
            model = glm::rotate(model, body->rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(body->displayRadius));

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(body->color));
            glUniform1i(glGetUniformLocation(shaderProgram, "isSun"), body->isSun);
            glUniform1i(glGetUniformLocation(shaderProgram, "isSelected"), idx == selectedPlanetIndex);
            glUniform1f(glGetUniformLocation(shaderProgram, "glowIntensity"), glowPulse);
            
            // apply texture if available
            if (body->hasTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, body->textureID);
                glUniform1i(glGetUniformLocation(shaderProgram, "textureSampler"), 0);
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), true);
                
                // earth gets special night lights texture
                if (idx == 3 && earthNightTexture != 0) {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, earthNightTexture);
                    glUniform1i(glGetUniformLocation(shaderProgram, "nightTexture"), 1);
                    glUniform1i(glGetUniformLocation(shaderProgram, "hasNightTexture"), true);
                } else {
                    glUniform1i(glGetUniformLocation(shaderProgram, "hasNightTexture"), false);
                }
            } else {
                glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), false);
                glUniform1i(glGetUniformLocation(shaderProgram, "hasNightTexture"), false);
            }

            glDrawElements(GL_TRIANGLES, sphere.indices.size(), GL_UNSIGNED_INT, 0);
        }
        
        // apply gaussian blur passes
        bool horizontal = true, first_iteration = true;
        int amount = 4;  // reduced from 10 for performance
        glUseProgram(blurShader);
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            glUniform1i(glGetUniformLocation(blurShader, "horizontal"), horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffer[!horizontal]);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // final render pass with bloom effect
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(bloomShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
        glUniform1i(glGetUniformLocation(bloomShader, "scene"), 0);
        glUniform1i(glGetUniformLocation(bloomShader, "bloomBlur"), 1);
        glUniform1f(glGetUniformLocation(bloomShader, "bloom"), 0.3f);  // 1.0'dan 0.3'e - çok daha az bloom
        glUniform1f(glGetUniformLocation(bloomShader, "exposure"), 1.0f);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // show ui menu after bloom rendering
        if (showMenu) {
            ImGui::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(380, 300), ImGuiCond_FirstUseEver);
            ImGui::Begin("solar system simulation", &showMenu, ImGuiWindowFlags_NoCollapse);
            
            // time control section
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, 1.0f));
            ImGui::Text("TIME CONTROL");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("##timescale", &timeScale, 0.0f, 5.0f, "speed: %.2fx");
            ImGui::PopItemWidth();
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // planet tracking section
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.85f, 1.0f, 1.0f));
            ImGui::Text("PLANET TRACKING");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            
            if (selectedPlanetIndex >= 0 && selectedPlanetIndex < celestialBodies.size()) {
                ImGui::Text("selected planet:");
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.5f, 1.0f));
                ImGui::Text("%s", celestialBodies[selectedPlanetIndex]->name.c_str());
                ImGui::PopStyleColor();
                
                ImGui::Spacing();
                ImGui::Checkbox("follow mode", &followMode);
                ImGui::Spacing();
                
                if (ImGui::Button("clear selection", ImVec2(-1, 0))) {
                    selectedPlanetIndex = -1;
                    followMode = false;
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                ImGui::TextWrapped("click on a planet to select it");
                ImGui::PopStyleColor();
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // info section
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 0.5f, 1.0f));
            ImGui::Text("fps: %.0f", ImGui::GetIO().Framerate);
            ImGui::PopStyleColor();
            
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::Text("controls:");
            ImGui::BulletText("wasd - move camera");
            ImGui::BulletText("space - move up");
            ImGui::BulletText("shift - move down");
            ImGui::BulletText("mouse - look around");
            ImGui::BulletText("scroll - zoom in/out");
            ImGui::BulletText("left click - select planet");
            ImGui::BulletText("right click - deselect planet");
            ImGui::BulletText("esc - quit");
            ImGui::BulletText("tab - toggle menu");
            ImGui::PopStyleColor();
            
            ImGui::End();
        }
        
        // draw crosshair when not in menu
        if (!showMenu) {
            ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH / 2.0f - 20, SCR_HEIGHT / 2.0f - 20));
            ImGui::SetNextWindowSize(ImVec2(40, 40));
            ImGui::Begin("Crosshair", nullptr, 
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 center(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
            
            // draw crosshair
            draw_list->AddLine(
                ImVec2(center.x - 10, center.y), 
                ImVec2(center.x + 10, center.y), 
                IM_COL32(255, 255, 255, 150), 2.0f);
            draw_list->AddLine(
                ImVec2(center.x, center.y - 10), 
                ImVec2(center.x, center.y + 10), 
                IM_COL32(255, 255, 255, 150), 2.0f);
            
            ImGui::End();
            
            // menu hint on left side
            ImGui::SetNextWindowPos(ImVec2(20, 20));
            ImGui::SetNextWindowBgAlpha(0.5f);
            ImGui::Begin("MenuHint", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.9f));
            ImGui::Text("press TAB to show menu");
            ImGui::PopStyleColor();
            ImGui::End();
        }
        
        // ImGui render
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup resources before exit
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    for (auto body : celestialBodies) {
        delete body;
    }
    
    // free orbit line resources
    for (auto& orbit : orbitLines) {
        glDeleteVertexArrays(1, &orbit.VAO);
        glDeleteBuffers(1, &orbit.VBO);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &starsVAO);
    glDeleteBuffers(1, &starsVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(blurShader);
    glDeleteProgram(bloomShader);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (!showMenu) {  // Menü açıkken kamera hareketi yok
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        showMenu = !showMenu;
        if (showMenu) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // reset firstMouse to prevent camera jump when menu closes
            firstMouse = true;
        }
    }
    
    // press escape to exit follow mode
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && followMode) {
        followMode = false;
        selectedPlanetIndex = -1;
        std::cout << "follow mode disabled" << std::endl;
    }
}

// mouse picking - planet selection
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // allow clicking planets anytime (even when menu is closed)
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // if menu is open, let imgui handle it
        if (showMenu) return;
        
        // use screen center when cursor is disabled
        float xpos = SCR_WIDTH / 2.0f;
        float ypos = SCR_HEIGHT / 2.0f;
        
        // convert screen position to normalized device coordinates
        float x = (2.0f * xpos) / SCR_WIDTH - 1.0f;
        float y = 1.0f - (2.0f * ypos) / SCR_HEIGHT;
        
        glm::vec4 rayClip(x, y, -1.0, 1.0);
        
        // View space'e
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
        
        // World space'e
        glm::mat4 view = camera.GetViewMatrix();
        glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
        
        // Ray-sphere intersection test
        float closestDistance = FLT_MAX;
        int closestIndex = -1;
        
        for (size_t i = 0; i < celestialBodies.size(); i++) {
            glm::vec3 sphereCenter = celestialBodies[i]->position;
            float sphereRadius = celestialBodies[i]->displayRadius;
            
            glm::vec3 oc = camera.Position - sphereCenter;
            float a = glm::dot(rayWorld, rayWorld);
            float b = 2.0f * glm::dot(oc, rayWorld);
            float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
            float discriminant = b * b - 4 * a * c;
            
            if (discriminant > 0) {
                float distance = (-b - sqrt(discriminant)) / (2.0f * a);
                if (distance > 0 && distance < closestDistance) {
                    closestDistance = distance;
                    closestIndex = i;
                }
            }
        }
        
        if (closestIndex != -1) {
            selectedPlanetIndex = closestIndex;
            followMode = true;  // automatically enable follow mode
            std::cout << "selected planet: " << celestialBodies[closestIndex]->name << " - follow mode enabled" << std::endl;
        }
    }
    
    // right-click to deselect planet
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !showMenu) {
        if (selectedPlanetIndex != -1) {
            selectedPlanetIndex = -1;
            followMode = false;
            std::cout << "planet deselected - follow mode disabled" << std::endl;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (showMenu) return; // no mouse movement when menu is open
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    // only process camera movement if not in follow mode
    if (!followMode) {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (showMenu) return; // no zoom when menu is open
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

std::string loadShaderSource(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Shader dosyası açılamadı: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader derleme hatası:\n" << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str());
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str());

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program bağlama hatası:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
