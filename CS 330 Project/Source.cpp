#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "cylinder.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void ProcessMouseScroll(float yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1400;

// Ortho default is false
bool ortho = false;

// camera
Camera camera(glm::vec3(0.0f, 0.5f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

glm::vec3 OrthodUp = -camera.Up;
glm::vec3 OrthodWorldUp = -camera.WorldUp;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CS-330 Project (Diego Bez Zambiazzi)", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("shaderfiles/6.multiple_lights.vs", "shaderfiles/6.multiple_lights.fs");
    Shader lightCubeShader("shaderfiles/6.light_cube.vs", "shaderfiles/6.light_cube.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float boxVertices[] = {
        // Vertex Positions    // Normal vectors      // Texture coords 
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f,     // 0 Base Back Left
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 1.0f,     // 1 Base Front Left
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f,     // 2 Base Front Right 
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 1.0f,     // 3 Base Front Right 
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    1.0f, 0.0f,     // 4 Base Back Right
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,    0.0f, 0.0f,     // 5 Base Back Left

        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 1.0f,     // 6 Front Face Top Left
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 0.0f,     // 7 Front Face Bottom Left
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 0.0f,     // 8 Front Face Bottom Right 
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 0.0f,     // 9 Front Face Bottom Right 
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    1.0f, 1.0f,     // 10 Front Face Top Right
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,    0.0f, 1.0f,     // 11 Front Face Top Left

         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 1.0f,     // 12 Right Face Top Left
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 0.0f,     // 13 Right Face Bottom Left
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,     // 14 Right Face Bottom Right 
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,     // 15 Right Face Bottom Right 
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,    1.0f, 1.0f,     // 16 Right Face Top Right
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,    0.0f, 1.0f,     // 17 Right Face Top Left

         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 1.0f,     // 18 Back Face Top Left
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 0.0f,     // 19 Back Face Bottom Left
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 0.0f,     // 20 Back Face Bottom Right 
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 0.0f,     // 21 Back Face Bottom Right 
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    1.0f, 1.0f,     // 22 Back Face Top Right
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,    0.0f, 1.0f,     // 23 Back Face Top Left

        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,     // 24 Left Face Top Left
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,     // 25 Left Face Bottom Left
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,     // 26 Left Face Bottom Right 
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,     // 27 Left Face Bottom Right 
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,     // 28 Left Face Top Right
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,     // 29 Left Face Top Left

        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,     // 30 Top Face Top Left
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 0.0f,     // 31 Top Face Bottom Left
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 0.0f,     // 32 Top Face Bottom Right 
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 0.0f,     // 33 Top Face Bottom Right 
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    1.0f, 1.0f,     // 34 Top Face Top Right
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,     // 35 Top Face Top Left
    };

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    float planeVertices[] = {
        // Vertex Positions    // Normal vectors      // Texture coords 
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 0.0f,     // 0 Back Left
        -0.5f, -0.5f,  0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 1.0f,     // 1 Front Left
         0.5f, -0.5f,  0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 1.0f,     // 2 Front Right 
         0.5f, -0.5f,  0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 1.0f,     // 3 Front Right 
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,  0.0f,    1.0f, 0.0f,     // 4 Back Right
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,  0.0f,    0.0f, 0.0f,     // 5 Back Left
    };
    unsigned int planeVBO, planeVAO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glBindVertexArray(planeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // positions of the point lights
    glm::vec3 pointLightPositions[] = {
        glm::vec3(5.0f, -0.5f, 0.0f),
        glm::vec3(-5.0f, 0.5f, 1.0f),
    };
    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightWindowVAO;
    glGenVertexArrays(1, &lightWindowVAO);
    glBindVertexArray(lightWindowVAO);

    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float chestLegsVertices[] = {
        // Vertex Positions    // Normal vectors      // Texture coords 
        0.0f,  0.5f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 1.0f,     // 0 Front Face left top
        0.5f,  0.5f,  0.5f,    0.0f, 0.0f,  1.0f,    1.0f, 1.0f,     // 1 Front Face right top
        0.0f, -0.5f,  0.5f,    0.0f, 0.0f,  1.0f,    0.0f, 0.0f,     // 2 Front Face left bottom
        0.0f,  0.5f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 1.0f,     // 3 Left Face right top
        0.0f, -0.5f,  0.5f,   -1.0f, 0.0f,  0.0f,    1.0f, 0.0f,     // 4 Left Face right bottom
        0.0f,  0.5f,  0.0f,   -1.0f, 0.0f,  0.0f,    0.0f, 1.0f,     // 5 Left Face left top
    };
    unsigned int chestLegsVBO, chestLegsVAO;
    glGenVertexArrays(1, &chestLegsVAO);
    glGenBuffers(1, &chestLegsVBO);

    glBindBuffer(GL_ARRAY_BUFFER, chestLegsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chestLegsVertices), chestLegsVertices, GL_STATIC_DRAW);

    glBindVertexArray(chestLegsVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // Cylinder
    static_meshes_3D::Cylinder cylinder(0.25, 30, 1.0, true, true, true);
    unsigned int cylinderVAO, cylinderVBO;

    glGenVertexArrays(1, &cylinderVAO);
    glBindVertexArray(cylinderVAO);
    glGenBuffers(1, &cylinderVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);

    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int marbleDiffuseMap = loadTexture("marble.gif");
    unsigned int marbleSpecularMap = loadTexture("marble-specmap.jpg");
    unsigned int pinkMarbleDiffuseMap = loadTexture("pinkMarble.jpg");
    unsigned int pinkMarbleSpecularMap = loadTexture("pinkMarble-specmap.jpg");
    unsigned int woodDiffuseMap = loadTexture("wood.jpg");
    unsigned int woodSpecularMap = loadTexture("wood-specmap.jpg");
    unsigned int whiteWoodDiffuseMap = loadTexture("white-wood.jpg");
    unsigned int whiteWoodSpecularMap = loadTexture("white-wood-specmap.jpg");
    unsigned int metalDiffuseMap = loadTexture("metal.jpg");
    unsigned int metalSpecularMap = loadTexture("metal-specmap.jpg");
    unsigned int waxDiffuseMap = loadTexture("wax.jpg");
    unsigned int waxSpecularMap = loadTexture("wax-specmap.jpg");
    unsigned int perfumeDiffuseMap = loadTexture("perfume.jpg");
    unsigned int perfumeSpecularMap = loadTexture("perfume-specmap.jpg");
    unsigned int perfumeCapDiffuseMap = loadTexture("perfume-cap.jpg");
    unsigned int perfumeCapSpecularMap = loadTexture("perfume-cap-specmap.jpg");
    unsigned int perfumeFrontDiffuseMap = loadTexture("perfume-front.jpg");
    unsigned int perfumeFrontSpecularMap = loadTexture("perfume-front-specmap.jpg");
    unsigned int greyDiffuseMap = loadTexture("glass.png");
    unsigned int greySpecularMap = loadTexture("glass-specmap.png");
    
    // My own try on making a cylinder
    float cylinderAngleVertices[] = {
        // Vertex Positions              // Normal vectors      // Texture coords 
         0.0f,      0.0f,   0.0f,        0.0f, -1.0f,  0.0f,    0.0f, 0.0f,     // 0 Left point base
         0.97815f,  0.0f,  -0.20791f,    0.0f, -1.0f,  0.0f,    0.9f, 0.2f,     // 1 Depth point base
         1.0f,      0.0f,   0.0f,        0.0f, -1.0f,  0.0f,    1.0f, 0.0f,     // 2 Right point base

         1.0f,      0.0f,   0.0f,        0.20791f, 0.0f,  -0.02185f,    0.0f, 0.0f,     // 3 Side Face left bottom
         0.97815f,  0.0f,  -0.20791f,    0.20791f, 0.0f,  -0.02185f,    1.0f, 0.0f,     // 4 Side Face right bottom
         1.0f,      1.0f,   0.0f,        0.20791f, 0.0f,  -0.02185f,    0.0f, 1.0f,     // 5 Side Face left top
         1.0f,      1.0f,   0.0f,        0.20791f, 0.0f,  -0.02185f,    0.0f, 1.0f,     // 6 Side Face left top
         0.97815f,  1.0f,  -0.20791f,    0.20791f, 0.0f,  -0.02185f,    1.0f, 1.0f,     // 7 Side Face right top
         0.97815f,  0.0f,  -0.20791f,    0.20791f, 0.0f,  -0.02185f,    1.0f, 0.0f,     // 8 Side Face right bottom

         0.0f,      1.0f,   0.0f,        0.0f, 1.0f,  0.0f,    0.0f, 0.0f,     // 9 Left point top
         0.97815f,  1.0f,  -0.20791f,    0.0f, 1.0f,  0.0f,    0.9f, 0.2f,     // 10 Depth point top
         1.0f,      1.0f,   0.0f,        0.0f, 1.0f,  0.0f,    1.0f, 0.0f,     // 11 Right point top
    };
    unsigned int cylinderAngleVBO, cylinderAngleVAO;
    glGenVertexArrays(1, &cylinderAngleVAO);
    glGenBuffers(1, &cylinderAngleVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cylinderAngleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cylinderAngleVertices), cylinderAngleVertices, GL_STATIC_DRAW);

    glBindVertexArray(cylinderAngleVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // My own try on making a glass
    float glassAngleVertices[] = {
        // Vertex Positions              // Normal vectors      // Texture coords 
         0.0f,      0.0f,   0.0f,        0.0f, -1.0f,  0.0f,    0.0f, 0.0f,     // 0 Left point base
         0.97815f,  0.0f,  -0.20791f,    0.0f, -1.0f,  0.0f,    0.9f, 0.2f,     // 1 Depth point base
         1.0f,      0.0f,   0.0f,        0.0f, -1.0f,  0.0f,    1.0f, 0.0f,     // 2 Right point base

         1.0f,      0.0f,   0.0f,        0.20791f, 0.0f,  -0.02185f,    0.0f, 0.0f,     // 3 Side Face left bottom
         0.97815f,  0.0f,  -0.20791f,    0.20791f, 0.0f,  -0.02185f,    1.0f, 0.0f,     // 4 Side Face right bottom
         1.0f,      1.0f,   0.0f,        0.20791f, 0.0f,  -0.02185f,    0.0f, 1.0f,     // 5 Side Face left top
         1.0f,      1.0f,   0.0f,        0.20791f, 0.0f,  -0.02185f,    0.0f, 1.0f,     // 6 Side Face left top
         0.97815f,  1.0f,  -0.20791f,    0.20791f, 0.0f,  -0.02185f,    1.0f, 1.0f,     // 7 Side Face right top
         0.97815f,  0.0f,  -0.20791f,    0.20791f, 0.0f,  -0.02185f,    1.0f, 0.0f,     // 8 Side Face right bottom
    };
    unsigned int glassAngleVBO, glassAngleVAO;
    glGenVertexArrays(1, &glassAngleVAO);
    glGenBuffers(1, &glassAngleVBO);

    glBindBuffer(GL_ARRAY_BUFFER, glassAngleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glassAngleVertices), glassAngleVertices, GL_STATIC_DRAW);

    glBindVertexArray(glassAngleVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // directional light
        lightingShader.setVec3("dirLight.direction", -0.2f, -0.2f, -0.2f);
        lightingShader.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
        lightingShader.setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.3f);
        lightingShader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);
        // point light 1
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        lightingShader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[0].constant", 1.0f);
        lightingShader.setFloat("pointLights[0].linear", 0.22);
        lightingShader.setFloat("pointLights[0].quadratic", 0.20);

        // point light 2
        lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        lightingShader.setVec3("pointLights[1].ambient", 0.3f, 0.3f, 0.3f);
        lightingShader.setVec3("pointLights[1].diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("pointLights[1].constant", 1.0f);
        lightingShader.setFloat("pointLights[1].linear", 0.22);
        lightingShader.setFloat("pointLights[1].quadratic", 0.19);

        // spotLight
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("spotLight.diffuse", 0.6f, 0.6f, 0.6f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09);
        lightingShader.setFloat("spotLight.quadratic", 0.032);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(5.0f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(8.0f)));


        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // Condition if orthographic
        if (ortho) {
            float scale = 200;
            float scaledWidth = (GLfloat)SCR_WIDTH / scale;
            float scaledHeight = (GLfloat)SCR_HEIGHT / scale;
            projection = glm::ortho(-scaledWidth, scaledWidth, scaledHeight, -scaledHeight, -4.0f, 10.0f);
        }
        else {
            projection = glm::perspective(45.0f, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
        }
        lightingShader.setMat4("projection", projection);

        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        lightingShader.setFloat("material.shininess", 100.0f);

        // render boxes
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, woodSpecularMap);
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.15f, 0.0f));
        float angle = -20.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.6f, 0.5f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render chest legs
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, metalDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metalSpecularMap);
        glBindVertexArray(chestLegsVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.53f, -0.5f, -0.03f));
        angle = -20.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.3f, 0.2f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, metalDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metalSpecularMap);
        glBindVertexArray(chestLegsVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.29f, -0.5f, 0.38f));
        angle = -20.0f + 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.3f, 0.2f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, metalDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metalSpecularMap);
        glBindVertexArray(chestLegsVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.53f, -0.5f, 0.03f));
        angle = -20.0f + 180.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.3f, 0.2f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, metalDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metalSpecularMap);
        glBindVertexArray(chestLegsVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.29f, -0.5f, -0.38f));
        angle = -20.0f + 270.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.3f, 0.2f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Metal decor
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, metalDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, metalSpecularMap);
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.15f, 0.0f));
        angle = -20.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.01f, 0.2f, 0.51f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Cylinder top
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, woodSpecularMap);
        glBindVertexArray(cylinderVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.2f, 0.0f));
        angle = -20.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        angle = 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        angle = 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.99f, 1.0f));
        lightingShader.setMat4("model", model);
        cylinder.render();

        // Pink marble box
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pinkMarbleDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pinkMarbleSpecularMap);
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.25f, -0.43f, 0.8f));
        angle = -15.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f, 0.1f, 0.2f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pinkMarbleDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pinkMarbleSpecularMap);
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.25f, -0.37f, 0.8f));
        angle = -15.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.31f, 0.03f, 0.21f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render perfume
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, perfumeDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, perfumeSpecularMap);
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, -0.30f, 0.0f));
        angle = 20.0f * 1.5;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.5f, 0.15f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, perfumeFrontDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, perfumeFrontSpecularMap);
        glBindVertexArray(planeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.93f, -0.30f, 0.105f));
        angle = 20.0f * 1.5;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        angle = 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.35f, 0.1f, 0.45f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, perfumeCapDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, perfumeCapSpecularMap);
        glBindVertexArray(cylinderAngleVAO);
        for (unsigned int i = 0; i < 30; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, -0.05f, 0.0f));
            float angle = 12.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.08f, 0.15f, 0.08f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 12);
        }

        // render white base
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteWoodDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, whiteWoodSpecularMap);
        glBindVertexArray(cubeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-0.5f, -1.5f, 0.0f));
        angle = 5.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f, 2.0f, 3.0f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // render plane
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, marbleDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, marbleSpecularMap);
        glBindVertexArray(planeVAO);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(10.0f, 0.1f, 10.0f));
        lightingShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ====== My cylinder ============

        // render candle
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, waxDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, waxSpecularMap);
        glBindVertexArray(cylinderAngleVAO);
        for (unsigned int i = 0; i < 30; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, -0.5f, 1.0f));
            float angle = 12.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.04f, 0.25f, 0.04f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 12);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, greyDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, greySpecularMap);
        glBindVertexArray(cylinderAngleVAO);
        for (unsigned int i = 0; i < 30; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, -0.25f, 1.0f));
            float angle = 12.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.005f, 0.05f, 0.005f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 12);
        }

        // render glass
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, greyDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, greySpecularMap);
        glBindVertexArray(glassAngleVAO);
        for (unsigned int i = 0; i < 30; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-1.0f, -0.5f, 1.0f));
            float angle = 12.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.09f, 0.30f, 0.09f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 12);
        }

        // ==== LIGHTS ====
        // 
        // also draw the lamp object(s)
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightWindowVAO);

        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[0]);
        angle = 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        angle = 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f, 0.1f, 3.5f));
        lightCubeShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions[1]);
        angle = 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        angle = 90.0f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 0.1f, 2.0f));
        lightCubeShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightWindowVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    float velocity = camera.MovementSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // W: goes forward
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // S: goes backward
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // A: goes left
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // D: goes right
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // E: goes upward
        if (ortho) {
            camera.Position -= camera.Up * velocity;
        }
        else {
            camera.Position += camera.Up * velocity;
        }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // Q: goes downward
        if (ortho) {
            camera.Position += camera.Up * velocity;
        }
        else {
            camera.Position -= camera.Up * velocity;
        }
}

// Key callback to handle key "P" to change to Ortho
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) return; //only handle press events
    if (key == GLFW_KEY_P) {
        camera.WorldUp = OrthodWorldUp;
        OrthodWorldUp = -camera.WorldUp;
        ortho = !ortho;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ProcessMouseScroll(yoffset);
}


// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void ProcessMouseScroll(float yoffset)
{
    // Mouse Scroll changes speed
    camera.MovementSpeed += yoffset;
    if (camera.MovementSpeed < 1.0f)
        camera.MovementSpeed = 1.0f;
    if (camera.MovementSpeed > 50.0f)
        camera.MovementSpeed = 50.0f;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}