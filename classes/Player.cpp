#include "Player.h"

#include <sstream>

const float PLAYER_BASE_SPEED  = 3.0f;
const float PLAYER_SPRINT_MODIFIER = 1.5f;

const double PLAYER_SENSITIVITY = 0.25;
const float PLAYER_RANGE = 5.0f;

const float PLAYER_WIDTH = 0.1f;

const float CAMERA_HEIGHT = 1.7f;

const float VOLUME = 0.1f;

const float HITSCAN_STEP_SIZE = 0.1f;

const double MAX_FOV = 120.0;
const double MIN_FOV = 1.0;

const float GRAVITY = 0.004f;
const float JUMP_HEIGHT = 0.1f;

const int TORCH_LIGHT_LEVEL = 12;

const bool CONSTRAIN_PITCH = true;

std::queue<LightNode> lightQueue;
std::queue<LightNode> lightRemovalQueue;
std::set<Chunk*> lightMeshingList;

glm::vec3 lastChunk(-5);

bool keys[1024] = {0};

std::set<glm::vec3, Vec3Comparator> EmptyChunks;
std::vector<SoundPlayer> soundPlayers;

struct InventorySlot {
    unsigned char Slot;
    unsigned char Type;
    unsigned int Size;
};

std::vector<InventorySlot> Inventory;

std::vector<std::string> Split(const std::string &s, char delim) {
    std::vector<std::string> elements;
    std::stringstream ss(s);
    std::string item;
    
    while (std::getline(ss, item, delim)) {
        elements.push_back(item);
    }
    
    return elements;
}

void Player::PollSounds() {
	if (soundPlayers.size() > 0) {
		std::vector<SoundPlayer>::iterator player = soundPlayers.begin();

		while (player != soundPlayers.end()) {
			if (!player->Playing()) {
				player = soundPlayers.erase(player);
			}
			else {
				player++;
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

    if (Flying) {
        Velocity = glm::vec3(0);

        if (keys[GLFW_KEY_W]) {
            WorldPos += Cam.Front * speed;
        }

        if (keys[GLFW_KEY_S]) {
            WorldPos -= Cam.Front * speed;
        }

        if (keys[GLFW_KEY_A]) {
            WorldPos -= Cam.Right * speed;
        }

        if (keys[GLFW_KEY_D]) {
            WorldPos += Cam.Right * speed;
        }
    }
    else {
        if (keys[GLFW_KEY_W]) {
            Velocity += Cam.FrontDirection * speed;
        }

        if (keys[GLFW_KEY_S]) {
            Velocity -= Cam.FrontDirection * speed;
        }

        if (keys[GLFW_KEY_A]) {
            Velocity -= Cam.RightDirection * speed;
        }

        if (keys[GLFW_KEY_D]) {
            Velocity += Cam.RightDirection * speed;
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
	
	if (Velocity.y < 0) {
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

	if (Velocity.x != 0) {
		glm::vec3 checkingPos = glm::vec3(WorldPos.x + Velocity.x + (PLAYER_WIDTH * std::copysign(1, Velocity.x)), WorldPos.y, WorldPos.z);

		if (!Is_Block(checkingPos)) {
			checkingPos.y += CAMERA_HEIGHT;

			if (!Is_Block(checkingPos)) {
				WorldPos.x += Velocity.x;
			}
		}
	}

	if (Velocity.z != 0) {
		glm::vec3 checkingPos = glm::vec3(WorldPos.x, WorldPos.y, WorldPos.z + Velocity.z + (PLAYER_WIDTH * std::copysign(1, Velocity.z)));

		if (!Is_Block(checkingPos)) {
			checkingPos.y += CAMERA_HEIGHT;

			if (!Is_Block(checkingPos)) {
				WorldPos.z += Velocity.z;
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
    
    for (auto const ch : lightMeshingList) {
        if (ChunkMap[ch] != nullptr) {
            ChunkMap[ch]->Mesh();
        }
    }
    
    EditingChunkMap = false;
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
        LastMousePos = glm::dvec2(posX, posY);
        MovedMouse = false;
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
    if (Cam.Zoom >= MIN_FOV && Cam.Zoom <= MAX_FOV) {
        Cam.Zoom -= offsetY;
    }

    if (Cam.Zoom < MIN_FOV) {
        Cam.Zoom = MIN_FOV;
    }

    if (Cam.Zoom > MAX_FOV) {
        Cam.Zoom = MAX_FOV;
    }
}

void Player::ClickHandler(int button, int action) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (LookingAtBlock) {
                Chunk* lookingChunk = ChunkMap[LookingChunk];
                
                int blockType = lookingChunk->Get_Block(LookingTile);
                
                if (blockType == 11) {
                    Remove_Torch();
                }
                else {
                    if (!Check_Top()) {
                        lookingChunk->LightQueue.emplace(LookingChunk, LookingTile, ChunkMap[LookingChunk]->Get_Light(LookingTile));
                    }
                }
                
                lookingChunk->Remove_Block(LookingTile);
                
                MouseHandler(LastMousePos.x, LastMousePos.y);
            }
        }
		else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (LookingAtBlock) {
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
                
                EditingChunkMap = true;
                
                if (ChunkMap.count(LookingAirChunk)) {
                    ChunkMap[LookingAirChunk]->Add_Block(LookingAirTile, diff, CurrentBlock);
                    
                    if (CurrentBlock == 11) {
                        Place_Torch();
                    }
                }
                
                ChunkMap[LookingAirChunk]->Add_Block(LookingAirTile, diff, CurrentBlock);
                
				MouseHandler(LastMousePos.x, LastMousePos.y);
			}
		}
    }
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
    while (ChunkMapBusy) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    ChunkMapBusy = true;
    
    std::map<glm::vec3, Chunk*>::iterator it = ChunkMap.begin();
    
    while (it != ChunkMap.end()) {
        double dist = pow(CurrentChunk.x - it->first.x, 2) + pow(CurrentChunk.z - it->first.z, 2);
        
        if (dist > pow(RENDER_DISTANCE, 2)) {
            delete it->second;
            it = ChunkMap.erase(it);
        }
        else {
            it++;
        }
    }

    for (int x = (int) CurrentChunk.x - RenderDistance; x <= CurrentChunk.x + RenderDistance; x++) {
        for (int z = (int) CurrentChunk.z - RenderDistance; z <= CurrentChunk.z + RenderDistance; z++) {
			for (int y = 3; y >= -10; y--) {
                glm::vec3 pos(x, y, z);

				if (pos == CurrentChunk) {
					continue;
				}
                
                if (ChunkSet.count(pos)) {
                    continue;
                }

				if (ChunkMap.count(pos)) {
					continue;
				}

				if (EmptyChunks.find(pos) != EmptyChunks.end()) {
					continue;
				}

                bool inRange = pow(CurrentChunk.x - x, 2) + pow(CurrentChunk.z - z, 2) <= pow(RenderDistance, 2);

                if (!inRange) {
					continue;
                }
                
				ChunkMap[pos] = new Chunk(pos);
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
        player.CurrentBlock = std::stoi(parameters[1]);
        return "Given block " + parameters[1] + " to player";
    }
    
    else {
        return "Error! Command not recognized.";
    }
}