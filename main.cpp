#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec2 vPos;
uniform vec2 pos;
void main() {
    gl_Position = vec4(vPos.x + pos.x, vPos.y + pos.y, 0, 1);
}
)";
const char* fragmentShaderSource = R"(#version 330 core
out vec4 fragColor;
void main() {
    fragColor = vec4(1);
}
)";

GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

GLuint createShaderProgram(GLuint vert, GLuint frag) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    return program;
}

class Shape {
public:
    Shape(const std::vector<float>& vertices) : position(0), vertices(vertices) {
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    ~Shape() {
        glDeleteBuffers(1, &VBO);
    }
    void Draw(GLuint program) {
        glUniform2fv(glGetUniformLocation(program, "pos"), 1, &position[0]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glm::vec2 position;
private:
    GLuint VBO;
    std::vector<float> vertices;
};


Shape* createRect(float width, float height) {
    return new Shape({
        -width/2, -height/2,
        -width/2,  height/2,
         width/2, -height/2,
        
        -width/2,  height/2,
         width/2, -height/2,
         width/2,  height/2,
    });
}

Shape* player1;
Shape* player2;

Shape* ball;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);    
}

GLuint shaderProgram;

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 800, "Pong!", NULL, NULL);
    glfwMakeContextCurrent(window);
    glewInit();

    glViewport(0, 0, 800, 800); 

    glfwSetKeyCallback(window, key_callback);

    GLuint vertShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    shaderProgram = createShaderProgram(vertShader, fragShader);

    glUseProgram(shaderProgram);

    ball = createRect(0.05f, 0.05f);

    player1 = createRect(0.05f, 0.4f);
    player1->position.x = -0.975f;
    player2 = createRect(0.05f, 0.4f);
    player2->position.x = 0.975f;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_W)  == GLFW_PRESS)
            player1->position.y += 0.002f;
        if (glfwGetKey(window, GLFW_KEY_S)  == GLFW_PRESS)
            player1->position.y -= 0.002f;

        player1->position.y = glm::clamp(player1->position.y, -0.8f, 0.8f);

        
        if (glfwGetKey(window, GLFW_KEY_UP)  == GLFW_PRESS)
            player2->position.y += 0.002f;
        if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS)
            player2->position.y -= 0.002f;

        player2->position.y = glm::clamp(player2->position.y, -0.8f, 0.8f);

        player1->Draw(shaderProgram);
        player2->Draw(shaderProgram);

        ball->Draw(shaderProgram);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}