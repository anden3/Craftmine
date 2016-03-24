#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#include "main.h"

#include <glm/gtc/type_ptr.hpp>

#include <SOIL.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "classes/Time.h"
#include "classes/Light.h"

Time t1("Timer 1");

int main() {
    glfwInit();

    // -------------------------------
    // GLFW config
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Test", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // -------------------------------
    // Callbacks
    glfwSetKeyCallback(window, key_proxy);
    glfwSetCursorPosCallback(window, mouse_proxy);
    glfwSetScrollCallback(window, scroll_proxy);
    glfwSetMouseButtonCallback(window, click_proxy);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // -------------------------------
    // OpenGL Config
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // -------------------------------
    // Textures
    unsigned int texture = loadTexture("../images/grass.png");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // -------------------------------
    // Shaders
    Shader lighting("../shaders/shader.vert", "", "../shaders/shader.frag", true, false, true);
    Shader text("../shaders/text.vert", "", "../shaders/text.frag", true, false, true);

    // -------------------------------
    // Uniform Buffer Object
    glGenBuffers(1, &UBO);
    glUniformBlockBinding(lighting.Program, glGetUniformBlockIndex(lighting.Program, "Matrices"), 0);

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glm::mat4 projection = glm::perspective(glm::radians(player.Cam.Zoom), (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 1000.0f);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // -------------------------------
    // Light
    lighting.Use();
    Light::Add_Dir_Light(lighting, glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(0.2f), glm::vec3(0.7f));

    // -------------------------------
    // Material
    glUniform1i(glGetUniformLocation(lighting.Program, "material.diffuse"), 0);

    // -------------------------------
    // Text
    Init_Text(text);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = (float) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        player.Move(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Generate_Chunk();
        Render_Scene(lighting);
        Draw_UI(text, deltaTime);

        glfwSwapBuffers(window);
    }

    t1.Get("all");
    glfwTerminate();
    return 0;
}

void Generate_Chunk() {
    if (ChunkQueue.size() > 0) {
        t1.Add();
        ChunkQueue.back()->Generate();
        ChunkQueue.back()->Mesh();
        t1.Add();

        ChunkMap[ChunkQueue.back()->Position] = ChunkQueue.back();
        ChunkQueue.pop_back();
    }
}

void Render_Scene(Shader shader) {
    shader.Use();

    // -------------------------------
    // Set Matrices
    glm::mat4 view = player.Cam.GetViewMatrix();
    glm::mat4 model;

    // -------------------------------
    // Upload Uniforms
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (toggleWireframe) {
        toggleWireframe = false;
        wireframe = !wireframe;

        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glUniform1i(glGetUniformLocation(shader.Program, "material.diffuse"), 50);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glUniform1i(glGetUniformLocation(shader.Program, "material.diffuse"), 0);
        }
    }

    // -------------------------------
    // Render Chunks
    for (auto const chunk: ChunkMap) {
        chunk.second->vbo.Draw();
    }
}

void Draw_UI(Shader shader, float deltaTime) {
    std::string fps = "FPS: ";
    std::string ram = "RAM: ";

    std::string playerChunk = "Chunk: " + formatVector(player.CurrentChunk, true);
    std::string playerTile = "Tile:      " + formatVector(player.CurrentTile, true);
    std::string playerPos = formatVector(player.WorldPos, false, "    ");

    for (int i = 0; i < AVG_FPS_RANGE; i++) {
        if (i < AVG_FPS_RANGE - 1) {
            last_fps[i] = last_fps[i + 1];
        }
        else {
            last_fps[i] = (1.0f / deltaTime + 0.5);
        }
    }

    if (text_counter == TEXT_UPDATE_FRAME_FREQ) {
        text_counter = 0;

        ram += getMemoryUsage();

        double fps_sum = 0.0;

        for (int i = 0; i < AVG_FPS_RANGE; i++) {
            fps_sum += last_fps[i];
        }

        fps += std::to_string((int) (fps_sum / AVG_FPS_RANGE));

        current_RAM = ram;
        current_FPS = fps;
    }
    else {
        ram = current_RAM;
        fps = current_FPS;

        text_counter++;
    }

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    Render_Text(shader, fps, 30.0f, SCREEN_HEIGHT - 50.0f, 0.5f, TEXT_COLOR);
    Render_Text(shader, ram, 30.0f, SCREEN_HEIGHT - 80.0f, 0.5f, TEXT_COLOR);

    Render_Text(shader, playerChunk, 30.0f, SCREEN_HEIGHT - 120.0f, 0.5f, TEXT_COLOR);
    Render_Text(shader, playerTile, 30.0f, SCREEN_HEIGHT - 150.0f, 0.5f, TEXT_COLOR);

    Render_Text(shader, playerPos, 30.0f, SCREEN_HEIGHT - 200.0f, 0.5f, TEXT_COLOR);

    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

void Init_Text(Shader shader) {
    FT_Library ft;
    FT_Face face;

    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    if (FT_New_Face(ft, "../fonts/Arial.ttf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load Font" << std::endl;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph " << c << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                (unsigned int) face->glyph->advance.x
        };

        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader.Use();

    glm::mat4 projection = glm::ortho(0.0f, (float) SCREEN_WIDTH, 0.0f, (float) SCREEN_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shader.Program, "text"), TEXT_TEXTURE_UNIT);
}

void Render_Text(Shader shader, std::string text, float x, float y, float scale, glm::vec3 color) {
    shader.Use();

    glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE0 + TEXT_TEXTURE_UNIT);
    glBindVertexArray(textVAO);

    std::string::const_iterator c;

    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        float xPos = x + ch.Bearing.x * scale;
        float yPos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float text_vertices[6][4] = {
                {xPos,     yPos + h,   0.0, 0.0},
                {xPos,     yPos,       0.0, 1.0},
                {xPos + w, yPos,       1.0, 1.0},

                {xPos,     yPos + h,   0.0, 0.0},
                {xPos + w, yPos,       1.0, 1.0},
                {xPos + w, yPos + h,   1.0, 0.0}
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(text_vertices), text_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int loadTexture(std::string image_path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height;
    unsigned char* image = SOIL_load_image(image_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);

    return texture;
}

std::string getMemoryUsage() {
    std::vector<std::string> units = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    long memUsage = usage.ru_maxrss;

    while (memUsage >= 1024) {
        memUsage /= 1024;
        unitIndex++;
    }

    return std::to_string(memUsage) + " " + units[unitIndex];
}

std::string formatVector(glm::vec3 vector, bool tuple, std::string separator) {
    std::string result;

    std::string x = std::to_string(int(vector.x));
    std::string y = std::to_string(int(vector.y));
    std::string z = std::to_string(int(vector.z));

    if (tuple) {
        result += "(" + x + separator + y + separator + z + ")";
    }
    else {
        result += "X: " + x + separator + "Y: " + y + separator + "Z: " + z;
    }

    return result;
}

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GL_TRUE);
            return;
        }
        else if (key == GLFW_KEY_I) {
            toggleWireframe = true;
        }
    }

    player.KeyHandler(key, action);
}
void mouse_proxy(GLFWwindow* window, double posX, double posY) {
    player.MouseHandler(posX, posY);
}
void scroll_proxy(GLFWwindow* window, double offsetX, double offsetY) {
    player.ScrollHandler(offsetY);
}
void click_proxy(GLFWwindow* window, int button, int action, int mods) {
    player.ClickHandler(button, action);
}

#pragma clang diagnostic pop