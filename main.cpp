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

struct collision {
    glm::vec2 dist;
    bool colliding;
};

void collide(collision& col, glm::vec2 pos1, glm::vec2 size1, glm::vec2 pos2, glm::vec2 size2) {
    col.colliding = false;
    col.dist.x = pos2.x - pos1.x;
    col.dist.y = pos2.y - pos1.y;
    if (
        pos1.x - size1.x < pos2.x + size2.x &&
        pos1.x + size1.x > pos2.x - size2.x &&
        pos1.y - size1.y < pos2.y + size2.y &&
        pos1.y + size1.y > pos2.y - size2.y
    ) col.colliding = true;
}


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

Shape* ball;

glm::vec2 ball_velocity;
float ballSpeed = 0.02f;

Shape* player1;
Shape* player2;

unsigned int p1_score = 0;
unsigned int p2_score = 0;

void handleScore() {
    ballSpeed = 0.01f;
    ball->position.x = 0;
    ball->position.y = 0;
    ball_velocity.y = 0;
    if (p1_score > p2_score)
        ball_velocity.x = ballSpeed;
    else
        ball_velocity.x = -ballSpeed;
    ballSpeed += 0.005f;
        
}

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
    glfwSwapInterval(1);
    glewInit();

    glViewport(0, 0, 800, 800); 

    glfwSetKeyCallback(window, key_callback);

    GLuint vertShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    shaderProgram = createShaderProgram(vertShader, fragShader);

    glUseProgram(shaderProgram);

    ball = createRect(0.05f, 0.05f);

    collision ballCol;

    player1 = createRect(0.05f, 0.4f);
    player1->position.x = -0.975f;
    player2 = createRect(0.05f, 0.4f);
    player2->position.x = 0.975f;

    handleScore();



    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_W)  == GLFW_PRESS)
            player1->position.y += ballSpeed;
        if (glfwGetKey(window, GLFW_KEY_S)  == GLFW_PRESS)
            player1->position.y -= ballSpeed;

        player1->position.y = glm::clamp(player1->position.y, -0.8f, 0.8f);

        
        if (glfwGetKey(window, GLFW_KEY_UP)  == GLFW_PRESS)
            player2->position.y += ballSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS)
            player2->position.y -= ballSpeed;

        player2->position.y = glm::clamp(player2->position.y, -0.8f, 0.8f);

        ball->position.x += ball_velocity.x;
        ball->position.y += ball_velocity.y;


        if (ball->position.y > 1 || ball->position.y < -1) ball_velocity.y = -ball_velocity.y;
        

        if (ball->position.x<0) {
            collide(ballCol, glm::vec2(-1.1f, player1->position.y), {0.15f, 0.2f}, ball->position, {0.025f, 0.025f});
            if (ballCol.colliding == true) ball_velocity = glm::normalize(ballCol.dist) * glm::vec2(ballSpeed) + glm::vec2(0, ball_velocity.y);
        }
        else {
            collide(ballCol, glm::vec2(1.1f, player2->position.y), {0.15f, 0.2f}, ball->position, {0.025f, 0.025f});
            if (ballCol.colliding == true) ball_velocity = glm::normalize(ballCol.dist) * glm::vec2(ballSpeed) + glm::vec2(0, ball_velocity.y);
        }

        if (ball->position.x > 1) {
            p2_score++;
            handleScore();
        }
        else if (ball->position.x < -1) {
            p1_score++;
            handleScore();
        }

        player1->Draw(shaderProgram);
        player2->Draw(shaderProgram);

        ball->Draw(shaderProgram);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}