#include "Player.h"

#include <sstream>

#include <glm/gtx/string_cast.hpp>

const float PLAYER_BASE_SPEED  = 3.0f;
const float PLAYER_SPRINT_MODIFIER = 1.5f;

const double PLAYER_SENSITIVITY = 0.25;
const float PLAYER_RANGE = 5.0f;

const float PLAYER_WIDTH = 0.1f;

const float CAMERA_HEIGHT = 1.7f;

const float VOLUME = 0.1f;

const float HITSCAN_STEP_SIZE = 0.1f;

const float GRAVITY = 0.004f;
const float JUMP_HEIGHT = 0.1f;

const int TORCH_LIGHT_LEVEL = 12;

const bool CONSTRAIN_PITCH = true;

std::queue<LightNode> lightQueue;
std::queue<LightNode> lightRemovalQueue;
std::set<Chunk*> lightMeshingList;

glm::vec3 lastChunk(-5);

bool keys[1024] = {0};

int NumKeys[10] = {GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0};

std::vector<SoundPlayer> soundPlayers;

std::vector<std::string> Split(const std::string &s, char delim) {
    std::vector<std::string> elements;
    std::stringstream ss(s);
    std::string item;
    
    while (std::getline(ss, item, delim)) {
        elements.push_back(item);
    }
    
    return elements;
}

void Player::Init_Model() {
    glGenVertexArrays(1, &ModelVAO);
    glGenBuffers(1, &ModelVBO);
    
    glBindVertexArray(ModelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ModelVBO);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), (void*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Player::Mesh_Model() {
    
}

void Player::Init_Holding() {
    glGenVertexArrays(1, &HoldingVAO);
    glGenBuffers(1, &HoldingVBO);
    
    glBindVertexArray(HoldingVAO);
    glBindBuffer(GL_ARRAY_BUFFER, HoldingVBO);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Player::Mesh_Holding() {
    if (CurrentBlock == 0) {
        return;
    }
    
    Data data;
    
    glm::vec2 texPosition = textureCoords[CurrentBlock];
    static float textureStep = (1.0f / 16.0f);
    
    float texStartX = textureStep * (texPosition.x - 1.0f);
    float texStartY = textureStep * (texPosition.y - 1.0f);
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            Extend(data, Data {vertices[i][j][0] - 0.5f, vertices[i][j][1] - 0.5f, vertices[i][j][2] - 0.5f});
            
            if (CurrentBlock == 2) {
                data.push_back(textureStep * (grassTextures[i].x - 1.0f) + tex_coords[i][j][0] * textureStep);
                data.push_back(textureStep * (grassTextures[i].y - 1.0f) + tex_coords[i][j][1] * textureStep);
            }
            else {
                data.push_back(texStartX + tex_coords[i][j][0] * textureStep);
                data.push_back(texStartY + tex_coords[i][j][1] * textureStep);
            }
        }
    }
    
    Upload_Data(HoldingVBO, data);
    
    HoldingVertices = int(data.size() / 5);
}

void Player::Draw_Holding() {
    if (CurrentBlock == 0) {
        return;
    }
    
    modelShader->Bind();
    
    glm::mat4 model;
    
    glm::vec3 offsetFront = Cam.Front;
    glm::vec3 offsetRight = Cam.Right;
    
    offsetFront *= 0.5;
    offsetRight *= 0.5;
    
    model = glm::translate(model, WorldPos + offsetFront + offsetRight + glm::vec3(0, 0.5, 0));
    model = glm::rotate(model, float(glm::radians(360.0f - Cam.Yaw - 90.0f)), glm::vec3(0, 1, 0));
    model = glm::rotate(model, float(glm::radians(Cam.Pitch)), glm::vec3(1, 0, 0));
    
    if (Cam.Yaw > 360) {
        Cam.Yaw -= 360;
    }
    
    if (Cam.Yaw < 0) {
        Cam.Yaw += 360;
    }
    
    glUniformMatrix4fv(glGetUniformLocation(modelShader->Program, "model"), 1, false, glm::value_ptr(model));
    
    glBindVertexArray(HoldingVAO);
    glDrawArrays(GL_TRIANGLES, 0, HoldingVertices);
    glBindVertexArray(0);
    
    modelShader->Unbind();
}

void Player::PollSounds() {
	if (soundPlayers.size() > 0) {
        for (auto player = soundPlayers.begin(); player != soundPlayers.end();) {
            if (!player->Playing()) {
                soundPlayers.erase(player++);
            }
            else {
                ++player;
            }
        }
	}
}

void Player::Move(float deltaTime, bool update) {
    glm::vec3 prevPos = WorldPos;
    
    if (FirstTime) {
        update = true;
        FirstTime = false;
    }
    
    Velocity.x = 0;
    Velocity.z = 0;

	float speed = PLAYER_BASE_SPEED * deltaTime;

    if (keys[GLFW_KEY_LEFT_SHIFT]) {
		speed *= PLAYER_SPRINT_MODIFIER * (Flying + 1);
    }
    
    static int moveKeys[4] = {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_A};
    static float signs[2] = {1.0f, -1.0f};
    
    glm::vec3 flyDirs[2] = {Cam.Front, Cam.Right};
    glm::vec3 walkDirs[2] = {Cam.FrontDirection, Cam.RightDirection};

    if (Flying) {
        Velocity = glm::vec3(0);

        for (int i = 0; i < 4; i++) {
            if (keys[moveKeys[i]]) {
                WorldPos += flyDirs[i / 2] * speed * signs[i % 2];
            }
        }
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (keys[moveKeys[i]]) {
                Velocity += walkDirs[i / 2] * speed * signs[i % 2];
            }
        }

        if (Jumping) {
            Jumping = false;
            Velocity.y += JUMP_HEIGHT;
        }

        ColDetection();
	}
    
    if (update || WorldPos != prevPos) {
        std::vector<glm::vec3> chunkPos = Get_Chunk_Pos(WorldPos);
        CurrentChunk = chunkPos[0];
        CurrentTile = chunkPos[1];
        
        Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + CAMERA_HEIGHT, WorldPos.z);
        listener.Set_Position(Cam.Position);
        
        std::vector<glm::vec3> hit = Hitscan();
        
        LookingAtBlock = (hit.size() == 4);
        
        if (LookingAtBlock) {
            LookingChunk = hit[0];
            LookingAirChunk = hit[2];
            
            LookingTile = hit[1];
            LookingAirTile = hit[3];
        }
        
        if (CurrentChunk != lastChunk) {
            lastChunk = CurrentChunk;
            RenderChunks();
        }
    }
}

void Player::Teleport(glm::vec3 pos) {
    Velocity = glm::vec3(0);
    WorldPos = pos;
    Move(0.0f, true);
}

void Player::ColDetection() {
	if (ChunkMap.empty() || !Exists(CurrentChunk)) {
		return;
	}

	Velocity.y -= GRAVITY;
    
    OnGround = (Velocity.y < 0 && Is_Block(glm::vec3(WorldPos.x, WorldPos.y + Velocity.y, WorldPos.z)));
    
    if (OnGround) {
        Velocity.y = 0;
    }
	
	else if (Velocity.y < 0) {
		if (!Is_Block(glm::vec3(WorldPos.x, WorldPos.y + Velocity.y, WorldPos.z))) {
			WorldPos.y += Velocity.y;
		}
	}
    
	else if (Velocity.y > 0) {
		if (!Is_Block(glm::vec3(WorldPos.x, WorldPos.y + CAMERA_HEIGHT + Velocity.y, WorldPos.z))) {
			WorldPos.y += Velocity.y;
		}
		else {
			Velocity.y = 0;
		}
	}
    
    glm::vec3 offsets[2] = {
        glm::vec3(Velocity.x + PLAYER_WIDTH * std::copysign(1, Velocity.x), 0, 0),
        glm::vec3(0, 0, Velocity.z + PLAYER_WIDTH * std::copysign(1, Velocity.z))
    };
    
    for (int i = 0; i < 3; i += 2) {
        if (Velocity[i]) {
            glm::vec3 checkingPos = WorldPos + offsets[i / 2];
            
            if (!Is_Block(checkingPos)) {
                checkingPos.y += CAMERA_HEIGHT;
                
                if (!Is_Block(checkingPos)) {
                    WorldPos[i] += Velocity[i];
                }
            }
        }
    }
}

void Process_Light_Queue() {
    if (lightQueue.empty()) {
        return;
    }
    
    while (!lightQueue.empty()) {
        LightNode node = lightQueue.front();
        glm::vec3 chunk = node.Chunk;
        glm::vec3 tile = node.Tile;
        lightQueue.pop();
        
        int lightLevel = ChunkMap[chunk]->Get_Light(tile);
        bool underground = !ChunkMap[chunk]->Get_Air(tile);
        
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(chunk, tile);
        
        for (auto const &neighbor : neighbors) {
            if (!Exists(neighbor.first)) {
                continue;
            }
            
            Chunk* neighborChunk = ChunkMap[neighbor.first];
            
            if (!neighborChunk->Get_Block(neighbor.second)) continue;
            if (!neighborChunk->Get_Air(neighbor.second) && underground) continue;
            if (neighborChunk->Get_Top(neighbor.second)) continue;
            if (neighborChunk->Get_Light(neighbor.second) + 2 >= lightLevel) continue;
            
            neighborChunk->Set_Light(neighbor.second, lightLevel - 1);
            lightQueue.emplace(neighbor.first, neighbor.second);
            lightMeshingList.insert(neighborChunk);
        }
    }
    
    for (auto const &ch : lightMeshingList) {
        ch->Mesh();
    }
}

void Process_Light_Removal_Queue() {
    while (!lightRemovalQueue.empty()) {
        LightNode node = lightRemovalQueue.front();
        glm::vec3 chunk = node.Chunk;
        glm::vec3 tile = node.Tile;
        short lightLevel = node.LightLevel;
        lightRemovalQueue.pop();
        
        bool underground = !ChunkMap[chunk]->Get_Air(tile);
        
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(chunk, tile);
        
        for (auto const &neighbor : neighbors) {
            if (!Exists(neighbor.first)) {
                continue;
            }
            
            Chunk* neighborChunk = ChunkMap[neighbor.first];
                
            if (!neighborChunk->Get_Block(neighbor.second)) continue;
            if (!neighborChunk->Get_Air(neighbor.second) && underground) continue;
            
            int neighborLight = neighborChunk->Get_Light(neighbor.second);
            
            if (neighborLight != 0 && neighborLight < lightLevel) {
                neighborChunk->Set_Light(neighbor.second, 0);
                
                lightMeshingList.insert(neighborChunk);
                lightRemovalQueue.emplace(neighbor.first, neighbor.second, neighborLight);
            }
            else if (neighborLight >= lightLevel) {
                lightMeshingList.insert(neighborChunk);
                lightQueue.emplace(neighbor.first, neighbor.second, neighborLight);
            }
        }
    }
    
    Process_Light_Queue();
}

void Player::Place_Torch() {
    ChunkMap[LookingAirChunk]->Set_Light(LookingAirTile, TORCH_LIGHT_LEVEL);
    lightQueue.emplace(LookingAirChunk, LookingAirTile, TORCH_LIGHT_LEVEL);
    
    Process_Light_Queue();
}

void Player::Remove_Torch() {
    lightRemovalQueue.emplace(LookingChunk, LookingTile, ChunkMap[LookingChunk]->Get_Light(LookingTile));
    ChunkMap[LookingChunk]->Set_Light(LookingTile, 0);
    
    Process_Light_Removal_Queue();
}

std::vector<glm::vec3> Player::Hitscan() {
    std::vector<glm::vec3> failedScan = {glm::vec3(0)};

	glm::vec3 lastPos;

	bool started = false;

    for (float t = 0; t < PLAYER_RANGE; t += HITSCAN_STEP_SIZE) {
        glm::vec3 checkingPos = Cam.Position + Cam.Front * t;

		if (Is_Block(checkingPos)) {
			if (started) {
				std::vector<glm::vec3> result1 = Get_Chunk_Pos(checkingPos);
				std::vector<glm::vec3> result2 = Get_Chunk_Pos(lastPos);

				result1.insert(result1.end(), result2.begin(), result2.end());
				return result1;
			}

			return Get_Chunk_Pos(checkingPos);
		}

		lastPos = checkingPos;
		started = true;
    }

    return failedScan;
}

void Player::KeyHandler(int key, int action) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_F) {
            Flying = !Flying;
        }

        if (key == GLFW_KEY_SPACE && OnGround) {
            Jumping = true;
        }
        
        if (key == GLFW_KEY_TAB) {
            inventory.Is_Open = !inventory.Is_Open;
            inventory.Mesh();
            
            if (inventory.Is_Open) {
                inventory.Mouse_Handler();
            }
        }
        
        if (!inventory.Is_Open) {
            for (int i = 0; i < 10; i++) {
                if (key == NumKeys[i]) {
                    inventory.ActiveToolbarSlot = i;
                    inventory.Switch_Slot();
                    CurrentBlock = inventory.Get_Info(i).first;
                    Mesh_Holding();
                    break;
                }
            }
        }
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

void Player::MouseHandler(double posX, double posY) {
    if (!MovedMouse) {
        LastMousePos = glm::dvec2(posX, posY);
        MovedMouse = true;
    }
    
    if (MouseEnabled) {
        if (inventory.Is_Open) {
            inventory.Mouse_Handler(posX, posY);
        }
        
        LastMousePos = glm::dvec2(posX, posY);
        return;
    }

    Cam.Yaw += (posX - LastMousePos.x) * PLAYER_SENSITIVITY;
    Cam.Pitch += (LastMousePos.y - posY) * PLAYER_SENSITIVITY;

    if (CONSTRAIN_PITCH) {
        if (Cam.Pitch > 89.9) {
            Cam.Pitch = 89.9;
        }

        if (Cam.Pitch < -89.9) {
            Cam.Pitch = -89.9;
        }
    }
    
    LastMousePos = glm::dvec2(posX, posY);
    Cam.UpdateCameraVectors();

	listener.Set_Orientation(Cam.Front);

    std::vector<glm::vec3> hit = Hitscan();

    if (hit.size() == 4) {
        LookingAtBlock = true;
        
        LookingChunk = hit[0];
        LookingAirChunk = hit[2];
        
        LookingTile = hit[1];
		LookingAirTile = hit[3];
    }
    else {
        LookingAtBlock = false;
    }
}

void Player::ScrollHandler(double offsetY) {
    if (fabs(offsetY) > 0.2) {
        inventory.ActiveToolbarSlot += int(std::copysign(1, offsetY));
        
        if (inventory.ActiveToolbarSlot > 9) {
            inventory.ActiveToolbarSlot = 0;
        }
        
        if (inventory.ActiveToolbarSlot < 0) {
            inventory.ActiveToolbarSlot = 9;
        }
        
        inventory.Switch_Slot();
        CurrentBlock = inventory.Get_Info(inventory.ActiveToolbarSlot).first;
        Mesh_Holding();
    }
}

void Player::ClickHandler(int button, int action) {
    if (!MouseEnabled && action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (LookingAtBlock) {
                Chunk* lookingChunk = ChunkMap[LookingChunk];
                
                int blockType = lookingChunk->Get_Block(LookingTile);
                
                if (blockType == 2) {
                    inventory.Add_Stack(3, 1);
                }
                else {
                    inventory.Add_Stack(blockType, 1);
                }
                
                if (CurrentBlock == 0 && inventory.Get_Info(inventory.ActiveToolbarSlot).first) {
                    CurrentBlock = inventory.Get_Info(inventory.ActiveToolbarSlot).first;
                    Mesh_Holding();
                }
                
                if (blockType == 11) {
                    Remove_Torch();
                }
                
                lookingChunk->Remove_Block(LookingTile);
            }
        }
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (LookingAtBlock && CurrentBlock) {
				glm::vec3 newBlockPos = Get_World_Pos(LookingAirChunk, LookingAirTile);

				if (LookingAirChunk.x == CurrentChunk.x && LookingAirChunk.z == CurrentChunk.z) {
					if (LookingAirTile.x == CurrentTile.x && LookingAirTile.z == CurrentTile.z) {
						int diff = int(newBlockPos.y) - int(floor(WorldPos.y));

						if (diff >= 0 && diff < 2) {
							return;
						}
					}
				}

                glm::vec3 diff = newBlockPos - Get_World_Pos(LookingChunk, LookingTile);
                
                if (ChunkMap.count(LookingAirChunk)) {
                    ChunkMap[LookingAirChunk]->Add_Block(LookingAirTile, diff, CurrentBlock);
                    
                    inventory.Decrease_Size(inventory.ActiveToolbarSlot);
                    
                    if (!inventory.Get_Info(inventory.ActiveToolbarSlot).first) {
                        CurrentBlock = 0;
                    }
                    
                    if (CurrentBlock == 11) {
                        Place_Torch();
                    }
                }
			}
		}
    }
    
    MouseHandler(LastMousePos.x, LastMousePos.y);
}

void Player::PlaySound(glm::vec3 chunk, glm::vec3 tile) {
	SoundPlayer player;

	player.Set_Position(Get_World_Pos(chunk, tile));
	player.Set_Volume(VOLUME);

	Sound sound = Sound("dirt_1");
	player.Add(sound);
	sound.Delete();

	player.Play();

	soundPlayers.push_back(player);
}

void Player::RenderChunks() {
    int startY = 3;
    int endY = -10;
    
    if (player.CurrentChunk.y <= -6) {
        startY = int(player.CurrentChunk.y) + 3;
        endY = int(player.CurrentChunk.y) - 3;
    }
    
    while (ChunkMapBusy) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    ChunkMapBusy = true;
    
    for (auto chunk = ChunkMap.begin(); chunk != ChunkMap.end();) {
        double dist = pow(CurrentChunk.x - chunk->first.x, 2) + pow(CurrentChunk.z - chunk->first.z, 2);
        
        if (dist > pow(RenderDistance, 2) || chunk->first.y > startY || chunk->first.y < endY) {
            delete chunk->second;
            ChunkMap.erase(chunk++);
        }
        else {
            ++chunk;
        }
    }

    for (int x = (int) CurrentChunk.x - RenderDistance; x <= CurrentChunk.x + RenderDistance; x++) {
        for (int z = (int) CurrentChunk.z - RenderDistance; z <= CurrentChunk.z + RenderDistance; z++) {
			for (int y = startY; y >= endY; y--) {
                glm::vec3 pos(x, y, z);

				if (pos != CurrentChunk && !ChunkMap.count(pos)) {
                    if (pow(CurrentChunk.x - x, 2) + pow(CurrentChunk.z - z, 2) <= pow(RenderDistance, 2)) {
                        ChunkMap[pos] = new Chunk(pos);
                    }
				}
            }
        }
    }
    
    ChunkMapBusy = false;
}

void Player::Clear_Keys() {
    std::fill_n(keys, 1024, false);
}

bool Player::Check_Top() {
    if (ChunkMap[LookingChunk]->Get_Top(LookingTile)) {
        ChunkMap[LookingChunk]->Set_Top(LookingTile, false);
        glm::vec3 checkingPos = Get_World_Pos(LookingChunk, LookingTile) - glm::vec3(0, 1, 0);
        
        while (true) {
            std::vector<glm::vec3> check = Get_Chunk_Pos(checkingPos);
            
            if (!Exists(check[0])) {
                return false;
            }
            
            Chunk* chunk = ChunkMap[check[0]];
            
            if (chunk->Get_Block(check[1])) {
                chunk->Set_Top(check[1], true);
                chunk->Set_Light(check[1], SUN_LIGHT_LEVEL);
                chunk->LightQueue.emplace(check[0], check[1]);
                return true;
            }
            
            checkingPos -= glm::vec3(0, 1, 0);
        }
    }
    
    return false;
}

std::string Process_Commands(std::string message) {
    std::vector<std::string> parameters = Split(message, ' ');
    
    if (parameters.size() == 0) {
        return "/";
    }
    
    std::string command = parameters[0];
    
    if (command == "tp") {
        int x = std::stoi(parameters[1]);
        int y = std::stoi(parameters[2]);
        int z = std::stoi(parameters[3]);
        
        player.Teleport(glm::vec3(x, y, z));
        
        return "Player teleported to (" + parameters[1] + ", " + parameters[2] + ", " + parameters[3] + ")";
    }
    
    else if (command == "give") {
        int block = std::stoi(parameters[1]);
        int size = std::stoi(parameters[2]);
        
        player.inventory.Add_Stack(block, size);
        return "Given block " + parameters[1] + " to player";
    }
    
    else if (command == "clear") {
        player.inventory.Clear();
        return "Inventory cleared!";
    }
    
    else {
        return "Error! Command not recognized.";
    }
}