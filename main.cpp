#include "main.h"

#include <thread>
#include <chrono>

#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include "Light.h"

int main() {
	Init_GL();
	Init_Textures();
	
    UI::Init();
    
	Init_Shaders();
    Init_Buffers();
	Init_Rendering();
    
	std::thread chunkGeneration(BackgroundThread);
    
	while (!glfwWindowShouldClose(Window)) {
		double currentFrame = glfwGetTime();
		DeltaTime = currentFrame - LastFrame;
		LastFrame = currentFrame;

		glfwPollEvents();

		player.PollSounds();
        
        if (!ShowMenu && !chat.Focused) {
            player.Move(float(DeltaTime));
        }
		
		Update_Data_Queue();
		Render_Scene();
        
        UI::Draw();
        
		glfwSwapBuffers(Window);
	}

	chunkGeneration.detach();
    UI::Clean();
    
	delete shader;

    glfwTerminate();
    return 0;
}

void Init_GL() {
	glfwInit();

	glfwWindowHint(GLFW_DECORATED, false);
	glfwWindowHint(GLFW_RESIZABLE, false);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
    
    SCREEN_WIDTH = videoMode->width;
    SCREEN_HEIGHT = videoMode->height;

	Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", monitor, nullptr);

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
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas);
}

void Init_Shaders() {
    shader = new Shader("shader");
    outlineShader = new Shader("outline");
    
    glm::mat4 projection = glm::perspective(glm::radians((float)player.Cam.Zoom), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.001f, 1000.0f);
    
	glGenBuffers(1, &UBO);
	glUniformBlockBinding(shader->Program, glGetUniformBlockIndex(shader->Program, "Matrices"), 0);
    glUniformBlockBinding(outlineShader->Program, glGetUniformLocation(outlineShader->Program, "Matrices"), 0);

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Init_Buffers() {
    glGenBuffers(1, &OutlineVBO);
    glGenVertexArrays(1, &OutlineVAO);
    
    std::vector<float> data;
    
    float points[8][2] = { {0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 0}, {1, 0}, {1, 1}, {0, 1} };
    float n[2] {float(-1 / sqrt(3)), float(1 / sqrt(3))};
    
    for (float y = 0; y < 3; y++) {
        for (int i = 0; i < 4; i++) {
            float x1 = points[i][0], x2 = x1;
            float y1 = y, y2 = y;
            float z1 = points[i][1], z2 = z1;
            
            if (y == 2) y1 = 0, y2 = 1;
            else {
                if (i != 4) x2 = points[i + 1][0], z2 = points[i + 1][1];
                else x2 = 0, z2 = 0;
            }
            
            Extend(data, std::vector<float> {x1, y1, z1});
            Extend(data, std::vector<float> {n[int(x1)], n[int(y1)], n[int(z1)]});
            
            Extend(data, std::vector<float> {x2, y2, z2});
            Extend(data, std::vector<float> {n[int(x2)], n[int(y2)], n[int(z2)]});
        }
    }
    
    
    glBindVertexArray(OutlineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, OutlineVBO);
    
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Init_Rendering() {
	shader->Bind();
	glUniform3f(glGetUniformLocation(shader->Program, "ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(shader->Program, "diffuse"), 0.7f, 0.7f, 0.7f);
	glUniform1i(glGetUniformLocation(shader->Program, "diffTex"), 0);
	shader->Unbind();
}

void Update_Data_Queue() {
	EditingDataQueue = true;

	if (DataQueue.size() > 0) {
		EditingChunkMap = true;

		std::map<glm::vec3, std::vector<float>, Vec3Comparator>::iterator it = DataQueue.begin();

		while (it != DataQueue.end()) {
			if (ChunkMap.count(it->first)) {
				ChunkMap[it->first]->vbo.Data(it->second);
				it = DataQueue.erase(it);
			}
			else {
				it++;
			}
		}

		EditingChunkMap = false;
	}

	EditingDataQueue = false;
}

void Render_Scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4 model;
    glm::mat4 view = player.Cam.GetViewMatrix();
    
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    shader->Bind();
    glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (ToggleWireframe) {
		Wireframe = !Wireframe;
		ToggleWireframe = false;

		if (Wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform1i(glGetUniformLocation(shader->Program, "diffTex"), 50);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUniform1i(glGetUniformLocation(shader->Program, "diffTex"), 0);
		}
	}

	EditingChunkMap = true;
	for (auto const chunk : ChunkMap) {
        chunk.second->vbo.Draw();
	}
	EditingChunkMap = false;

    shader->Unbind();
    
    if (player.LookingAtBlock) {
        outlineShader->Bind();
        
        model = glm::translate(model, Get_World_Pos(player.LookingChunk, player.LookingTile));
        glUniformMatrix4fv(glGetUniformLocation(outlineShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(OutlineVAO);
        glDrawArrays(GL_LINES, 0, 24);
        glBindVertexArray(0);
        
        outlineShader->Unbind();
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

    return texture;
}

void Extend(std::vector<float>& storage, std::vector<float> input) {
    for (auto const value : input) {
        storage.push_back(value);
    }
}

void BackgroundThread() {
	while (true) {
		if (ChunkQueue.size() > 0) {
			for (auto it = ChunkQueue.cbegin(); it != ChunkQueue.cend();) {
                while (EditingChunkQueue) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                
                bool inRange = pow(it->first.x - player.CurrentChunk.x, 2) + pow(it->first.y - player.CurrentChunk.y, 2) + pow(it->first.z - player.CurrentChunk.z, 2) <= pow(RENDER_DISTANCE, 2);
                
                if (inRange) {
                    it->second->Generate();
                    
                    while (EditingDataQueue) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                    
                    it->second->Mesh();
                    
                    while (EditingChunkMap) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                    
                    ChunkMap[it->first] = it->second;
                }
                
                ChunkQueue.erase(it++);
			}
		}
		else {
            player.Process_Sunlight();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

void key_proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (chat.Focused) {
        player.Clear_Keys();
        
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    chat.Focused = false;
                    chat.FocusToggled = true;
                    
                    chat.Input(0x001B);
                    break;
                    
                case GLFW_KEY_BACKSPACE:
                    chat.Input(0x0008);
                    break;
                    
                case GLFW_KEY_ENTER:
                    chat.Input(0x000D);
                    
                    chat.Focused = false;
                    chat.FocusToggled = true;
                    break;
            }
        }
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
                    
                case GLFW_KEY_T:
                    chat.Focused = true;
                    chat.FocusToggled = true;
                    break;
            }
        }
        
        player.KeyHandler(key, action);
    }
}
void text_proxy(GLFWwindow* window, unsigned int codepoint) {
    if (chat.Focused && !chat.FocusToggled) {
        chat.Input(codepoint);
    }
}
void mouse_proxy(GLFWwindow* window, double posX, double posY) {
    player.MouseHandler(posX, posY);
}
void scroll_proxy(GLFWwindow* window, double offsetX, double offsetY) {
    player.ScrollHandler(offsetY);
}
void click_proxy(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        UI::Click(player.LastMousePos.x, SCREEN_HEIGHT - player.LastMousePos.y, action);
    }
    
    player.ClickHandler(button, action);
}