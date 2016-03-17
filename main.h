#pragma once

// -------------------------------
// Includes
#include <iostream>
#include <vector>
#include <map>

#include <mach/mach_time.h>

#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

#include <noise/noise.h>
#include <noise/noiseutils.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "Variables.h"
#include "classes/shader.h"
#include "classes/Player.h"
#include "classes/Light.h"
#include "classes/VBO.h"
#include "classes/Chunk.h"

// -------------------------------
// Functions
void Render_Scene(Shader shader);
void Do_Movement(GLfloat deltaTime);
void Init_Text(Shader shader);
void Render_Text(Shader shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
GLuint loadTexture(std::string image);

// -------------------------------
// Structs
struct Character {
    GLuint     TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint     Advance;
};

// -------------------------------
// Globals
GLuint UBO;
GLuint textVAO, textVBO;

GLfloat lastX = SCREEN_WIDTH / 2, lastY = SCREEN_HEIGHT / 2;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool keys[1024];
bool firstMouse = true;

// -------------------------------
// Objects
Player player = Player();

// -------------------------------
// Data
std::map<GLchar, Character> Characters;
std::vector<Chunk> Chunks;

// -------------------------------
// Functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = (GLfloat) xpos;
        lastY = (GLfloat) ypos;
        firstMouse = false;
    }

    GLfloat xOffset = (GLfloat) xpos - lastX;
    GLfloat yOffset = (GLfloat) (lastY - ypos);

    lastX = (GLfloat) xpos;
    lastY = (GLfloat) ypos;

    player.ProcessMouseMovement(xOffset, yOffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    player.ProcessMouseScroll((GLfloat) yoffset);
}