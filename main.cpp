#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Face {
    std::vector<int> vertexIndices;
};
 
// Vertex Shader source code
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 transform;
    void main() {
        gl_Position = transform * vec4(aPos, 1.0);
    }
)glsl";

// Fragment Shader source code
const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
)glsl";

float d = 0.01f;
float degree = 1.0f;
float s = 0.01f;

glm::vec3 translation(0.0f, 0.0f, 0.0f);
float angle = 0.0f;
glm::vec3 scale(1.0f, 1.0f, 1.0f); 

std::vector<Face> faces;
std::vector< glm::vec3 > vertices;

// Prototypes
bool parseOBJ(const std::string& filePath);
void checkCompileErrors(GLuint shader, std::string type);
unsigned int initialize();
void processInput(GLFWwindow* window);

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 0;
    }

    // Set GLFW context version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Triangle with Transform", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    unsigned int shaderProgram = initialize();

    const char* path = "../bottle.obj";
    parseOBJ(path);

    unsigned int VBO[2], VAO[2], EBO;
    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);

    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glm::mat4 transform = glm::mat4(1.0f);

        transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::translate(transform, translation);
        transform = glm::scale(transform, scale);

        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Pass the transformation matrix to the shader
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

bool parseOBJ(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream sstream(line);
        std::string prefix;
        sstream >> prefix;

        if (prefix == "v") {
            glm::vec3 vertex;
            sstream >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } else if (prefix == "f") {
            Face face;
            std::string vertexData;
            while (sstream >> vertexData) {
                std::istringstream vertexStream(vertexData);
                std::string index;
                std::getline(vertexStream, index, '/');  // Only extract vertex index
                face.vertexIndices.push_back(std::stoi(index) - 1);
            }
            faces.push_back(face);
        }
    }

    return true;
}

void checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

unsigned int initialize() {
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return 0;
    }

    // Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    // Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // Shader Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    // Delete the shaders as they're linked into shaderProgram now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Function to process user input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Translation
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        translation.y += d;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        translation.y -= d;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        translation.x -= d;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        translation.x += d;

    // Rotation
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        angle += degree;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        angle -= degree;

    // Scale
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        scale += glm::vec3(s);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        scale -= glm::vec3(s); 
}
