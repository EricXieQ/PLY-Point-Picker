// Simple OpenGL PLY Viewer with Ray Casting Vertex Selection + Highlight + Line Drawing (Shader Version)
// Dependencies: GLFW, GLAD, GLM

#include "glad/include/glad/glad.h"          // Includes GLAD — needed to load modern OpenGL functions. (OpenGL loader)
#include <GLFW/glfw3.h>                      // Includes GLFW — needed for window management and input handling. (OpenGL framework)
#include <glm/glm.hpp>                       // Includes GLM — a math library for OpenGL, used for matrix and vector operations. (OpenGL Mathematics)
#include <glm/gtc/matrix_transform.hpp>      // Includes GLM transformations — used for creating view and projection matrices. (OpenGL Mathematics Transformations)
#include <glm/gtc/type_ptr.hpp>              // Includes GLM type pointers — used to pass matrices to shaders. (OpenGL Mathematics Type Pointers)
#include <iostream>                          // Includes iostream for console output. (C++ Standard Library Input/Output Stream)
#include <fstream>                           // Includes fstream for file input/output operations. (C++ Standard Library File Stream)
#include <sstream>                           // Includes sstream for string stream operations, useful for reading shader files. (C++ Standard Library String Stream)
#include <vector>                            // Includes vector for dynamic array management. (C++ Standard Library Vector)
#include <cmath>                             // Includes cmath for mathematical functions like sin, cos, etc. (C++ Standard Library Math)

// Global variables for camera control and picking
float yaw = 0.0f, pitch = 0.0f, zoom = 3.0f; // Camera parameters
bool rightMousePressed = false;              // Flag to check if right mouse button is pressed
double lastX = 0.0, lastY = 0.0;             // Last mouse position for camera rotation

struct Vec3 { 
    float x, y, z;    //position in 3D space
};

std::vector<Vec3> vertices;       // Stores vertices from the PLY file
std::vector<int> selectedIndices; // Stores indices of selected vertices for highlighting and line drawing

// Screen dimensions for the OpenGL window
int screenWidth = 1500; 
int screenHeight = 1500;

// Camera matrices
glm::mat4 view;                      // View matrix for camera position and orientation
glm::mat4 projection;                // Projection matrix for perspective projection
glm::vec3 cameraPos;                 // Camera position in world space
glm::mat4 model = glm::mat4(1.0f);   // Model matrix (identity for no transformation)

// Function to load and compile shaders from files
GLuint loadShader(const char* vertexPath, const char* fragmentPath) {

    std::ifstream vShaderFile(vertexPath);          // Open vertex shader file
    std::ifstream fShaderFile(fragmentPath);        // Open fragment shader file
    std::stringstream vShaderStream, fShaderStream; // String streams to read shader files
    vShaderStream << vShaderFile.rdbuf();           // Read vertex shader file into string stream
    fShaderStream << fShaderFile.rdbuf();           // Read fragment shader file into string stream
    std::string vertexCode = vShaderStream.str();   // Convert vertex shader stream to string
    std::string fragmentCode = fShaderStream.str(); // Convert fragment shader stream to string 

    const char* vShaderCode = vertexCode.c_str();   // Convert vertex shader string to C-style string
    const char* fShaderCode = fragmentCode.c_str(); // Convert fragment shader string to C-style string

    GLuint vertex, fragment;   // Shader object IDs
    GLint success;            // Variable to check compilation/linking success
    char infoLog[512];       // Buffer to store error messages
    
    // Compile vertex shaders
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // Link shaders into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader Linking Failed:\n" << infoLog << std::endl;
    }

    // Clean up shaders as they are no longer needed after linking
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return shaderProgram;
}

// Function to load PLY file and extract vertex data
void loadPLY(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    int vertexCount = 0;
    while (std::getline(file, line)) {
        if (line.substr(0, 14) == "element vertex") {
            vertexCount = std::stoi(line.substr(15));
        } else if (line == "end_header") {
            break;
        }
    }
    for (int i = 0; i < vertexCount; ++i) {
        Vec3 v;
        file >> v.x >> v.y >> v.z;
        vertices.push_back(v);
    }
    file.close();
}

// Function to get a ray from mouse coordinates using the current camera view and projection matrices
glm::vec3 getRayFromMouse(GLFWwindow* window, double mouseX, double mouseY) {
    int width, height;
    glfwGetWindowSize(window, &width, &height); // Get current window size

    float x = (2.0f * (float)mouseX) / (float)width - 1.0f;
    float y = 1.0f - (2.0f * (float)mouseY) / (float)height;
    
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(view * model) * ray_eye));
    return ray_world;
}

// Function to pick the nearest vertex based on ray casting
int pickNearestVertex(glm::vec3 rayOrigin, glm::vec3 rayDirection, float maxDist = 0.05f) {
    int pickedIndex = -1;
    float minDist = maxDist;
    for (int i = 0; i < vertices.size(); ++i) {
        glm::vec3 vertex(vertices[i].x, vertices[i].y, vertices[i].z);
        glm::vec3 toVertex = vertex - rayOrigin;
        float t = glm::dot(toVertex, rayDirection);
        glm::vec3 projected = rayOrigin + t * rayDirection;
        float dist = glm::length(projected - vertex);
        if (dist < minDist) {
            minDist = dist;
            pickedIndex = i;
        }
    }
    return pickedIndex;
}

// Callback functions for GLFW events
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Callback for mouse cursor position to handle camera rotation
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rightMousePressed) {
        float dx = xpos - lastX;
        float dy = ypos - lastY;
        yaw += dx * 0.3f;
        pitch += dy * 0.3f;
        
         // 🔒 Clamp pitch to avoid flipping
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        lastX = xpos;
        lastY = ypos;
    }
}

// Callback for mouse scroll to handle zooming
void scroll_callback(GLFWwindow* window, double xoffset, double
     yoffset) {
    zoom -= yoffset * 0.2f;
    if (zoom < 0.1f) zoom = 0.1f;
    if (zoom > 10.0f) zoom = 10.0f;
}

// Callback for mouse button events to handle vertex selection and right-click actions
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        glm::vec3 ray = getRayFromMouse(window ,xpos, ypos);
        int idx = pickNearestVertex(cameraPos, ray);
        std::cout << "selectedIndices.size(): " << selectedIndices.size() << std::endl;
        if (idx != -1) {
            selectedIndices.push_back(idx);
            std::cout << "Selected Vertex: " << idx << " (" << vertices[idx].x << ", " << vertices[idx].y << ", " << vertices[idx].z << ")\n";
            if (selectedIndices.size() > 2)
                // If more than 2 vertices are selected, remove the oldest one
                selectedIndices.erase(selectedIndices.begin());

            if (selectedIndices.size() == 2) {
                std::cout << "Selected 2 vertices, ready to draw line.\n";

                // Calculate distance between the two selected points
                Vec3 p1 = vertices[selectedIndices[0]];
                Vec3 p2 = vertices[selectedIndices[1]];

                float dx = p1.x - p2.x;
                float dy = p1.y - p2.y;
                float dz = p1.z - p2.z;

                float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                std::cout << "Distance between selected points: " << distance << std::endl;
            }
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMousePressed = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            rightMousePressed = false;
        }
    }
}


int main() {
    // Initialize GLFW (OpenGL Framework)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set OpenGL version major "3".x 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set OpenGL version minor 3.3 -> bascically we are using OpenGL framework version 3.3

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // forces developer to use shaders, vertex buffers object (VBO), and vertex array objects(VAO) <- More modern OpenGL

    // Create a windowed mode window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "PLY Picker", NULL, NULL); //NULL to fullscreen, and NUll for shared context to another window 
    
    //make the context as the current window
    glfwMakeContextCurrent(window);

    // Setting up call backs
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    // Initialize GLAD (OpenGL Loader)
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderProgram = loadShader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl"); // Load and compile shaders
    
    //////////////////////////////
    //change the ply file here////
    //////////////////////////////
    loadPLY("3DModel_Custom_copy.ply"); // Load PLY file 

    //Extract vertex data
    std::vector<float> vertexData;
    for (const auto& v : vertices) {
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
        vertexData.push_back(v.z);
    }
    
    // Create Vertex Array Object (VAO) and Vertex Buffer Object (VBO) for vertex data
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Create VAO and VBO for selected vertices
    GLuint selectedVAO, selectedVBO;
    glGenVertexArrays(1, &selectedVAO);
    glGenBuffers(1, &selectedVBO);


    // Main RENDER loop -> If the window is not closed keep running the loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set BACKGROUND color to white
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height); 

        float aspect = (float)width / (float)height;
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

        view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // Set view matrix to look at the origin

        // Orbit camera around the origin using yaw and pitch
        float camX = zoom * sin(glm::radians(yaw)) * cos(glm::radians(pitch)); 
        float camY = zoom * sin(glm::radians(pitch));
        float camZ = zoom * cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraPos = glm::vec3(camX, camY, camZ);

        // Identity model matrix (no rotation)
        view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        model = glm::mat4(1.0f);
        
        // Use shader program and set uniforms
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f); // default point color

        glBindVertexArray(VAO);
        glPointSize(4.0f);
        glDrawArrays(GL_POINTS, 0, vertices.size());

        // Draw selected points (yellow)
        std::vector<float> selectedData;
        for (int idx : selectedIndices) {
            Vec3 v = vertices[idx];
            selectedData.push_back(v.x);
            selectedData.push_back(v.y);
            selectedData.push_back(v.z);
        }

        glBindVertexArray(selectedVAO);
        glBindBuffer(GL_ARRAY_BUFFER, selectedVBO);
        glBufferData(GL_ARRAY_BUFFER, selectedData.size() * sizeof(float), selectedData.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 1.0f, 0.0f); // Highlight color (yellow)
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, selectedIndices.size());

        // Draw line if 2 points selected
        if (selectedIndices.size() == 2) {
            std::vector<float> lineData;
            for (int idx : selectedIndices) {
                Vec3 v = vertices[idx];
                lineData.push_back(v.x);
                lineData.push_back(v.y);
                lineData.push_back(v.z);
            }



            glBufferData(GL_ARRAY_BUFFER, lineData.size() * sizeof(float), lineData.data(), GL_DYNAMIC_DRAW);
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f); // Line color (blue)
            glDrawArrays(GL_LINES, 0, 2);

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // If the window is close delete all the buffers and terminate GLFW
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &selectedVAO);
    glDeleteBuffers(1, &selectedVBO);
    glfwTerminate();
    return 0;
}
