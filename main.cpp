#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

#include "main.h"

int main() {
    glfwInit();

    // -------------------------------
    // GLFW config
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Test", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // -------------------------------
    // Callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // -------------------------------
    // OpenGL Config
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // -------------------------------
    // Chunk
    Chunk chunk(glm::vec3(0, 0, 0));
    chunk.Mesh();

    // -------------------------------
    // Textures
    GLuint texture_diff = loadTexture("../images/container2.png");
    GLuint texture_spec = loadTexture("../images/container2_specular.png");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_diff);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_spec);

    // -------------------------------
    // Shaders
    Shader lighting("../shaders/lighting.vert", "", "../shaders/lighting.frag", true, false, true);
    Shader text("../shaders/text.vert", "", "../shaders/text.frag", true, false, true);

    // -------------------------------
    // Uniform Buffer Object
    glGenBuffers(1, &UBO);
    glUniformBlockBinding(lighting.Program, glGetUniformBlockIndex(lighting.Program, "Matrices"), 0);

    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));

    // -------------------------------
    // Lights
    lighting.Use();

    Light::Add_Point_Light(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.05f), glm::vec3(0.7f), glm::vec3(0.9f), 1.0, 0.09, 0.032);
    Light::Upload_Lights(lighting);

    // -------------------------------
    // Material
    glUniform1i(glGetUniformLocation(lighting.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lighting.Program, "material.specular"), 1);
    glUniform1f(glGetUniformLocation(lighting.Program, "material.shininess"), 128.0f);

    // -------------------------------
    // Text
    Init_Text(text);

    double last_fps[AVG_FPS_RANGE] = {0.0};
    int fps_counter = 0;
    int current_FPS = 0;

    while (!glfwWindowShouldClose(window)) {
        // -------------------------------
        // Frame Timing
        GLfloat currentFrame = (GLfloat) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        double fps = (1.0f / deltaTime + 0.5);

        for (int i = 0; i < AVG_FPS_RANGE; i++) {
            if (i < AVG_FPS_RANGE - 1) {
                last_fps[i] = last_fps[i + 1];
            }
            else {
                last_fps[i] = fps;
            }
        }

        if (fps_counter == FPS_UPDATE_FRAME_FREQ) {
            fps_counter = 0;

            double fps_sum = 0.0;

            for (int i = 0; i < AVG_FPS_RANGE; i++) {
                fps_sum += last_fps[i];
            }

            current_FPS = (int) (fps_sum / AVG_FPS_RANGE);
        }
        else {
            fps_counter += 1;
        }

        glfwPollEvents();
        Do_Movement(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Render_Scene(lighting, chunk);

        Render_Text(text, "FPS: " + std::to_string(current_FPS), 30.0f, SCREEN_HEIGHT - 50.0f, 0.5f, glm::vec3(0.2f, 0.8f, 0.2f));

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void Do_Movement(GLfloat deltaTime) {
    if (keys[GLFW_KEY_W]) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (keys[GLFW_KEY_S]) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (keys[GLFW_KEY_A]) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (keys[GLFW_KEY_D]) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void Render_Scene(Shader shader, Chunk chunk) {
    shader.Use();

    // -------------------------------
    // Set Matrices
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f,
                                            1000.0f);
    glm::mat4 model;

    // -------------------------------
    // Upload Uniforms
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camera.Position.x, camera.Position.y,
                camera.Position.z);

    // -------------------------------
    // Render Chunk
    chunk.Draw();
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

        GLuint texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE2);
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
                (GLuint) face->glyph->advance.x
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

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    shader.Use();

    glm::mat4 projection = glm::ortho(0.0f, (float) SCREEN_WIDTH, 0.0f, (float) SCREEN_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(glGetUniformLocation(shader.Program, "text"), 2);
}

void Render_Text(Shader shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
    shader.Use();

    glUniform3f(glGetUniformLocation(shader.Program, "textColor"), color.x, color.y, color.z);

    glActiveTexture(GL_TEXTURE2);
    glBindVertexArray(textVAO);

    std::string::const_iterator c;

    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        GLfloat xPos = x + ch.Bearing.x * scale;
        GLfloat yPos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat text_vertices[6][4] = {
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

GLuint loadTexture(std::string image_path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height;
    unsigned char* image = SOIL_load_image(image_path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

#pragma clang diagnostic pop