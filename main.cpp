#include "main.h"

#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "classes/Light.h"
#include "classes/Time.h"
#include "classes/System.h"

#include <thread>
#include <chrono>

int main() {
	Init_GL();
	Init_Textures();
	Init_Text();

	shader = new Shader("shader");

	Init_UBO();
	Init_Rendering();

	std::thread chunkGeneration(BackgroundThread);

	while (!glfwWindowShouldClose(Window)) {
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		player.PollSounds();
		player.Move((float)deltaTime);
		
		Update_Data_Queue();
		Render_Scene();
		Draw_UI();

		glfwSwapBuffers(Window);
	}

	chunkGeneration.detach();
	
	delete shader;
	delete text;

    glfwTerminate();
    return 0;
}

void Init_GL() {
	glfwInit();

	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Craftmine", nullptr, nullptr);

	glfwSetWindowPos(Window, 0, 0);

	glfwMakeContextCurrent(Window);
	glfwSwapInterval(ENABLE_VSYNC);

	glewExperimental = GL_TRUE;
	glewInit();

	glfwSetKeyCallback(Window, key_proxy);
	glfwSetCursorPosCallback(Window, mouse_proxy);
	glfwSetScrollCallback(Window, scroll_proxy);
	glfwSetMouseButtonCallback(Window, click_proxy);

	glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glClearColor(CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, 1.0f);
}

void Init_Textures() {
	unsigned int texture = Load_Texture("images/grass.png");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void Init_Text() {
	text = new Text("Minecraftia", 24);

	for (int i = 0; i < AVG_UPDATE_RANGE; i++) {
		if (i < AVG_UPDATE_RANGE - 1) {
			last_fps[i] = last_fps[i + 1];
			last_cpu[i] = last_cpu[i + 1];
		}
		else {
			last_fps[i] = (1.0f / deltaTime + 0.5);
			last_cpu[i] = System::GetCPUUsage();
		}
	}

	text_counter = 0;

	double fps_sum = 0.0;
	double cpu_sum = 0.0;

	for (int i = 0; i < AVG_UPDATE_RANGE; i++) {
		fps_sum += last_fps[i];
		cpu_sum += last_cpu[i];
	}

	text->X = 30.0f;
	text->Scale = 0.5f;

	text->Add("fps", "FPS: " + std::to_string((int)(fps_sum / AVG_UPDATE_RANGE)), 50);
	text->Add("cpu", "CPU: " + std::to_string((int)(cpu_sum / AVG_UPDATE_RANGE)) + "%", 80);

	text->Add("vram", "VRAM: " + System::GetVRAMUsage(), 120);
	text->Add("ram", "RAM: " + System::GetPhysicalMemoryUsage(), 150);
	text->Add("virtualMemory", "Virtual Memory: " + System::GetVirtualMemoryUsage(), 180);

	text->Add("playerChunk", "Chunk: " + Format_Vector(player.CurrentChunk, true), 220);
	text->Add("playerTile", "Tile: " + Format_Vector(player.CurrentTile, true), 250);

	text->Add("playerPos", Format_Vector(player.WorldPos, false, "    "), 290);

	text->Add("chunkQueue", "Chunks Queued: " + std::to_string(ChunkQueue.size()), 330);
}

void Init_UBO() {
	glGenBuffers(1, &UBO);
	glUniformBlockBinding(shader->Program, glGetUniformBlockIndex(shader->Program, "Matrices"), 0);

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glm::mat4 projection = glm::perspective(glm::radians((float)player.Cam.Zoom), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.001f, 1000.0f);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Init_Rendering() {
	shader->Bind();
	Light::Add_Dir_Light(*shader, glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(0.2f), glm::vec3(0.7f));

	glUniform1i(glGetUniformLocation(shader->Program, "material.diffuse"), 0);
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

	glm::mat4 view = player.Cam.GetViewMatrix();
	glm::mat4 model;

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	shader->Bind();
	glUniformMatrix4fv(glGetUniformLocation(shader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (toggleWireframe) {
		wireframe = !wireframe;
		toggleWireframe = false;

		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform1i(glGetUniformLocation(shader->Program, "material.diffuse"), 50);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUniform1i(glGetUniformLocation(shader->Program, "material.diffuse"), 0);
		}
	}

	EditingChunkMap = true;

	bool disableOutline = false;

	for (auto const chunk : ChunkMap) {
		if (player.LookingAtBlock && player.LookingChunk == chunk.second->Position) {
			glm::vec3 blockPos = Get_World_Pos(player.LookingChunk, player.LookingTile);

			glUniform3f(glGetUniformLocation(shader->Program, "BlockPos"), blockPos.x, blockPos.y, blockPos.z);
			glUniform1i(glGetUniformLocation(shader->Program, "DrawOutline"), 1);

			disableOutline = true;
		}

		chunk.second->vbo.Draw();

		if (disableOutline) {
			glUniform1i(glGetUniformLocation(shader->Program, "DrawOutline"), 0);
			disableOutline = false;
		}
	}

	EditingChunkMap = false;

	shader->Unbind();
}

void Draw_UI() {
	for (int i = 0; i < AVG_UPDATE_RANGE; i++) {
		if (i < AVG_UPDATE_RANGE - 1) {
			last_fps[i] = last_fps[i + 1];
			last_cpu[i] = last_cpu[i + 1];
		}
		else {
			last_fps[i] = (1.0f / deltaTime + 0.5);
			last_cpu[i] = System::GetCPUUsage();
		}
	}

	if (text_counter == TEXT_UPDATE_FRAME_FREQ) {  // TODO Make UI updates independent of frame rate
		text_counter = 0;

		double fps_sum = 0.0;
		double cpu_sum = 0.0;

		for (int i = 0; i < AVG_UPDATE_RANGE; i++) {
			fps_sum += last_fps[i];
			cpu_sum += last_cpu[i];
		}

		text->Set_Text("fps", "FPS: " + std::to_string((int)(fps_sum / AVG_UPDATE_RANGE)));
		text->Set_Text("cpu", "CPU: " + std::to_string((int)(cpu_sum / AVG_UPDATE_RANGE)) + "%");
		text->Set_Text("vram", "VRAM: " + System::GetVRAMUsage());
		text->Set_Text("ram", "RAM: " + System::GetPhysicalMemoryUsage());
		text->Set_Text("virtualMemory", "Virtual Memory: " + System::GetVirtualMemoryUsage());
	}
	else {
		text_counter++;
	}

	text->Set_Text("playerChunk", "Chunk: " + Format_Vector(player.CurrentChunk, true));
	text->Set_Text("playerTile", "Tile: " + Format_Vector(player.CurrentTile, true));
	text->Set_Text("playerPos", Format_Vector(player.WorldPos, false, "    "));
	text->Set_Text("chunkQueue", "Chunks Queued: " + std::to_string(ChunkQueue.size()));

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	text->Draw_All();

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
}

unsigned int Load_Texture(std::string image_path) {
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

std::string Format_Vector(glm::vec3 vector, bool tuple, std::string separator) {
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

void BackgroundThread() {
	while (true) {
		while (EditingChunkQueue) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		if (ChunkQueue.size() > 0) {
			for (auto it = ChunkQueue.cbegin(); it != ChunkQueue.cend();) {
				it->second->Generate();

				while (EditingDataQueue) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}

				it->second->Mesh();

				while (EditingChunkMap) {
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}

				ChunkMap[it->first] = it->second;
				ChunkQueue.erase(it++);
			}
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
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