#include "main.h"

#include <sstream>
#include <fstream>

#include <SOIL/SOIL.h>

int main() {
    glfwInit();
    
    Parse_Config();
    
	Init_GL();
	Init_Textures();
    
	Init_Shaders();
    Init_Buffers();
	Init_Rendering();
    
    UI::Init();
    
    player.Init();
    
	std::thread chunkGeneration(BackgroundThread);
    
	while (!glfwWindowShouldClose(Window)) {
		double currentFrame = glfwGetTime();
		DeltaTime = currentFrame - LastFrame;
		LastFrame = currentFrame;

		glfwPollEvents();

		player.PollSounds();
        
        if (!MouseEnabled && !chat.Focused) {
            player.Move(float(DeltaTime));
            Entity::Update(DeltaTime);
        }
		
		Render_Scene();
        Entity::Draw();
        player.Draw();
        
        glClear(GL_DEPTH_BUFFER_BIT);
        UI::Draw();
        
		glfwSwapBuffers(Window);
	}
    
    chunkGeneration.join();
    
    glfwTerminate();
    return 0;
}

void Parse_Config() {
    std::stringstream file_content;
    
    std::ifstream file(CONFIG_FILE);
    file_content << file.rdbuf();
    file.close();
    
    std::string line;
    
    while (std::getline(file_content, line)) {
        std::istringstream is_line(line);
        std::string key, value;
        
        std::getline(is_line, key, '=');
        std::getline(is_line, value);
        
        if (key == "") {
            continue;
        }
        
        *Options[key] = std::stoi(value);
    }
}

void Write_Config() {
    std::ofstream file(CONFIG_FILE);
    
    for (auto const &option : Options) {
        file << option.first << "=" << *option.second << "\n";
    }
    
    file.close();
}

void Init_GL() {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_RESIZABLE, false);
    
    if (Fullscreen) {
        glfwWindowHint(GLFW_DECORATED, false);
        
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
        
        SCREEN_WIDTH = videoMode->width;
        SCREEN_HEIGHT = videoMode->height;
        
        Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", monitor, nullptr);
    }
    else {
        glfwWindowHint(GLFW_DECORATED, true);
        Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", nullptr, nullptr);
    }

	glfwSetWindowPos(Window, 0, 0);

	glfwMakeContextCurrent(Window);
	glfwSwapInterval(VSync);

	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetKeyCallback(Window, key_proxy);
	glfwSetCursorPosCallback(Window, mouse_proxy);
	glfwSetScrollCallback(Window, scroll_proxy);
	glfwSetMouseButtonCallback(Window, click_proxy);
    glfwSetCharCallback(Window, text_proxy);

	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, 1.0f);
}

void Init_Textures() {
	unsigned int atlas = Load_Texture("atlas.png");
    unsigned int itemAtlas = Load_Texture("itemAtlas.png");
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, itemAtlas);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas);
}

void Init_Shaders() {
    shader = new Shader("shader");
    outlineShader = new Shader("outline");
    modelShader = new Shader("model");
    
    glm::mat4 projection = glm::perspective(glm::radians((float)player.Cam.Zoom), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.001f, 1000.0f);
    
	glGenBuffers(1, &UBO);
	glUniformBlockBinding(shader->Program, glGetUniformBlockIndex(shader->Program, "Matrices"), 0);
    glUniformBlockBinding(outlineShader->Program, glGetUniformLocation(outlineShader->Program, "Matrices"), 0);
    glUniformBlockBinding(modelShader->Program, glGetUniformLocation(modelShader->Program, "Matrices"), 0);

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Init_Buffers() {
    Data data;
    
    float points[8][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 1} };
    float n[2] {float(-1 / sqrt(3)), float(1 / sqrt(3))};
    
    for (float y = 0; y < 3; y++) {
        for (int i = 0; i < 4; i++) {
            float x1 = points[i][0];
            float z1 = points[i][1];
            
            float y1 = (y == 2) ? 0 : y;
            float y2 = (y == 2) ? 1 : y;
            
            float x2 = (y == 2) ? x1 : ((i != 4) ? points[i + 1][0] : 0);
            float z2 = (y == 2) ? z1 : ((i != 4) ? points[i + 1][1] : 0);
            
            Extend(data, x1, y1, z1);
            Extend(data, n[int(x1)], n[int(y1)], n[int(z1)]);
            
            Extend(data, x2, y2, z2);
            Extend(data, n[int(x2)], n[int(y2)], n[int(z2)]);
        }
    }
    
    OutlineBuffer.Init(outlineShader);
    OutlineBuffer.Create(3, 3, data);
    OutlineBuffer.VertexType = GL_LINES;
}

void Init_Rendering() {
    glm::mat4 model;
    
    shader->Upload("model", model);
    shader->Upload("ambient", AMBIENT_LIGHT);
    shader->Upload("diffuse", DIFFUSE_LIGHT);
    shader->Upload("diffTex", 0);
    
    modelShader->Upload("tex", 0);
}

void Render_Scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4 model;
    glm::mat4 view = player.Cam.GetViewMatrix();
    
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
	if (ToggleWireframe) {
		Wireframe = !Wireframe;
		ToggleWireframe = false;
        
        glPolygonMode(GL_FRONT_AND_BACK, Wireframe ? GL_LINE : GL_FILL);
        shader->Upload("diffTex", Wireframe ? 50 : 0);
	}
    
    for (auto const &chunk : ChunkMap) {
        if (chunk.second->Meshed) {
            if (!chunk.second->DataUploaded) {
                chunk.second->buffer.Upload(chunk.second->VBOData);
                chunk.second->DataUploaded = true;
            }
            
            chunk.second->buffer.Draw();
        }
	}
    
    if (player.LookingAtBlock) {
        outlineShader->Upload("model", glm::translate(model, Get_World_Pos(player.LookingChunk, player.LookingTile)));
        OutlineBuffer.Draw();
    }
}

unsigned int Load_Texture(std::string file) {
	std::string path = "images/" + file;

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height;
    unsigned char* image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    
    IMAGE_SIZE_X = width / 16;
    IMAGE_SIZE_Y = height / 16;

    return texture;
}

void BackgroundThread() {
	while (true) {
        if (glfwWindowShouldClose(Window)) return;
        
        bool queueEmpty = true;
        
        while (ChunkMapBusy) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        ChunkMapBusy = true;
        
        for (auto const &chunk : ChunkMap) {
            if (!chunk.second->Meshed) {
                bool inRange = pow(chunk.second->Position.x - player.CurrentChunk.x, 2) +
                               pow(chunk.second->Position.z - player.CurrentChunk.z, 2) <=
                               pow(RenderDistance, 2);
                
                if (inRange) {
                    if (!chunk.second->Generated) {
                        chunk.second->Generate();
                    }
                    
                    chunk.second->Light();
                    chunk.second->Mesh();
                    chunk.second->Meshed = true;
                    chunk.second->DataUploaded = false;
                    
                    queueEmpty = false;
                    break;
                }
            }
        }
        
        ChunkMapBusy = false;
        
        if (queueEmpty) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
	}
}

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (chat.Focused) {
        player.Clear_Keys();
    }
    else {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    UI::Toggle_Menu();
                    break;
                    
                case GLFW_KEY_U:
                    UI::Toggle_Debug();
                    break;
                
                case GLFW_KEY_TAB:
                    UI::Toggle_Inventory();
                    break;
            }
        }
        
        player.KeyHandler(key, action);
    }
    
    if (action == GLFW_PRESS) {
        chat.Key_Handler(key);
    }
}
void text_proxy(GLFWwindow* window, unsigned int codepoint) {
    if (chat.Focused && !chat.FocusToggled) {
        chat.Input(codepoint);
    }
}
void mouse_proxy(GLFWwindow* window, double posX, double posY) {
    UI::Mouse_Handler(posX, posY);
    
    if (!chat.Focused) {
        player.MouseHandler(posX, posY);
    }
}
void scroll_proxy(GLFWwindow* window, double offsetX, double offsetY) {
    player.ScrollHandler(offsetY);
}
void click_proxy(GLFWwindow* window, int button, int action, int mods) {
    UI::Click(player.LastMousePos.x, player.LastMousePos.y, action, button);
    player.ClickHandler(button, action);
}