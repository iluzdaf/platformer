#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float xPos = 0.0f, yPos = 0.0f;

void processInput(GLFWwindow *window, float deltaTime) {
    float speed = 2.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) yPos += speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) yPos -= speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) xPos -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) xPos += speed;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2D Platformer Start", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    const char* vertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform vec2 offset;
        void main() {
            gl_Position = vec4(aPos + offset, 0.0, 1.0);
        }
    )glsl";

    const char* fragmentShaderSource = R"glsl(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.8, 0.3, 0.2, 1.0);
        }
    )glsl";

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        -0.05f, -0.05f,
         0.05f, -0.05f,
         0.05f,  0.05f,
        -0.05f,  0.05f,
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window, deltaTime);

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        GLint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLoc, xPos, yPos);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}