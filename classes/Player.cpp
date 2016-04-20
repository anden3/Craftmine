#include "Player.h"

#include "Time.h"

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
std::set<glm::vec3, Vec3Comparator> lightMeshingList;

std::queue<LightNode> sunlightQueue;

glm::vec3 lastChunk(-5);

bool keys[1024] = {0};

std::set<glm::vec3, Vec3Comparator> EmptyChunks;
std::vector<SoundPlayer> soundPlayers;

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

void Player::Move(float deltaTime) {
    glm::vec3 prevPos = WorldPos;
    
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
    
    if (WorldPos != prevPos) {
        std::vector<glm::vec3> chunkPos = Get_Chunk_Pos(WorldPos);
        CurrentChunk = chunkPos[0];
        CurrentTile = chunkPos[1];
        
        Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + CAMERA_HEIGHT, WorldPos.z);
        listener.Set_Position(Cam.Position);
        
        std::vector<glm::vec3> hit = Hitscan();
        
        LookingAtBlock = (hit.size() == 4);
        
        if (LookingAtBlock) {
            LookingChunk = hit[0];
            LookingTile = hit[1];
            
            LookingAirChunk = hit[2];
            LookingAirTile = hit[3];
        }
        
        if (CurrentChunk != lastChunk) {
            lastChunk = CurrentChunk;
            RenderChunks();
        }
    }
}

void Player::ColDetection() {
	if (!ChunkMap.size() || !ChunkMap.count(CurrentChunk)) {
		return;
	}

	Velocity.y -= GRAVITY;
    
    OnGround = (Velocity.y < 0 && IsBlock(glm::vec3(WorldPos.x, WorldPos.y + Velocity.y, WorldPos.z)));
    
    if (OnGround) {
        Velocity.y = 0;
    }
	
	if (Velocity.y < 0) {
		if (!IsBlock(glm::vec3(WorldPos.x, WorldPos.y + Velocity.y, WorldPos.z))) {
			WorldPos.y += Velocity.y;
		}
	}
	else if (Velocity.y > 0) {
		if (!IsBlock(glm::vec3(WorldPos.x, WorldPos.y + CAMERA_HEIGHT + Velocity.y, WorldPos.z))) {
			WorldPos.y += Velocity.y;
		}
		else {
			Velocity.y = 0;
		}
	}

	if (Velocity.x != 0) {
		glm::vec3 checkingPos = glm::vec3(WorldPos.x + Velocity.x + (PLAYER_WIDTH * std::copysign(1, Velocity.x)), WorldPos.y, WorldPos.z);

		if (!IsBlock(checkingPos)) {
			checkingPos.y += CAMERA_HEIGHT;

			if (!IsBlock(checkingPos)) {
				WorldPos.x += Velocity.x;
			}
		}
	}

	if (Velocity.z != 0) {
		glm::vec3 checkingPos = glm::vec3(WorldPos.x, WorldPos.y, WorldPos.z + Velocity.z + (PLAYER_WIDTH * std::copysign(1, Velocity.z)));

		if (!IsBlock(checkingPos)) {
			checkingPos.y += CAMERA_HEIGHT;

			if (!IsBlock(checkingPos)) {
				WorldPos.z += Velocity.z;
			}
		}
	}
}

void Process_Light_Queue() {
    if (lightQueue.empty()) {
        return;
    }
    
    EditingChunkMap = true;
    
    while (!lightQueue.empty()) {
        LightNode node = lightQueue.front();
        glm::vec3 chunk = node.Chunk;
        glm::vec3 tile = node.Tile;
        lightQueue.pop();
        
        if (!ChunkMap.count(chunk)) continue;
        
        int lightLevel = ChunkMap[chunk]->Get_Light(tile);
        bool underground = !ChunkMap[chunk]->Get_Air(tile);
        
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(chunk, tile);
        
        for (auto neighbor : neighbors) {
            if (!ChunkMap.count(neighbor.first)) continue;
            if (!ChunkMap[neighbor.first]->Get_Block(neighbor.second)) continue;
            if (!ChunkMap[neighbor.first]->Get_Air(neighbor.second) && underground) continue;
            if (!ChunkMap[neighbor.first]->Is_Top(neighbor.second)) continue;
            if (ChunkMap[neighbor.first]->Get_Light(neighbor.second) + 2 >= lightLevel) continue;
            
            ChunkMap[neighbor.first]->Set_Light(neighbor.second, lightLevel - 1);
            
            LightNode node;
            node.Chunk = neighbor.first;
            node.Tile = neighbor.second;
            lightQueue.push(node);
            
            lightMeshingList.insert(neighbor.first);
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
    EditingChunkMap = true;
    
    while (!lightRemovalQueue.empty()) {
        LightNode node = lightRemovalQueue.front();
        
        glm::vec3 chunk = node.Chunk;
        glm::vec3 tile = node.Tile;
        short lightLevel = node.LightLevel;
        
        lightRemovalQueue.pop();
        
        bool underground = !ChunkMap[chunk]->Get_Air(tile);
        
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(chunk, tile);
        
        for (auto neighbor : neighbors) {
            if (!ChunkMap.count(neighbor.first)) {
                continue;
            }
            
            if (!ChunkMap[neighbor.first]->Get_Block(neighbor.second)) {
                continue;
            }
            
            if (!ChunkMap[neighbor.first]->Get_Air(neighbor.second) && underground) {
                continue;
            }
            
            int neighborLight = ChunkMap[neighbor.first]->Get_Light(neighbor.second);
            
            if (neighborLight != 0 && neighborLight < lightLevel) {
                ChunkMap[neighbor.first]->Set_Light(neighbor.second, 0);
                
                LightNode node;
                node.Chunk = neighbor.first;
                node.Tile = neighbor.second;
                node.LightLevel = neighborLight;
                
                lightMeshingList.insert(neighbor.first);
                lightRemovalQueue.push(node);
            }
            else if (neighborLight >= lightLevel) {
                LightNode node;
                node.Chunk = neighbor.first;
                node.Tile = neighbor.second;
                node.LightLevel = neighborLight;
                
                lightMeshingList.insert(neighbor.first);
                lightQueue.push(node);
            }
        }
    }
    EditingChunkMap = false;
    
    Process_Light_Queue();
}

void Player::Place_Torch() {
    EditingChunkMap = true;
    ChunkMap[LookingAirChunk]->Set_Light(LookingAirTile, TORCH_LIGHT_LEVEL);
    EditingChunkMap = false;
    
    LightNode node;
    node.Tile = LookingAirTile;
    node.Chunk = LookingAirChunk;
    node.LightLevel = TORCH_LIGHT_LEVEL;
    lightQueue.push(node);
    
    Process_Light_Queue();
}

void Player::Remove_Torch() {
    LightNode node;
    node.Chunk = LookingChunk;
    node.Tile = LookingTile;
    
    EditingChunkMap = true;
    node.LightLevel = ChunkMap[LookingChunk]->Get_Light(LookingTile);
    ChunkMap[LookingChunk]->Set_Light(LookingTile, 0);
    EditingChunkMap = false;
    
    lightRemovalQueue.push(node);
    
    Process_Light_Removal_Queue();
}

void Player::Process_Sunlight() {
    if (sunlightQueue.empty()) {
        return;
    }
    
    std::set<glm::vec3, Vec3Comparator> meshingList;
    
    while (!sunlightQueue.empty()) {
        LightNode node = sunlightQueue.front();
        glm::vec3 chunk = node.Chunk;
        glm::vec3 tile = node.Tile;
        sunlightQueue.pop();
        
        if (!ChunkMap.count(chunk)) continue;
        
        int lightLevel = ChunkMap[chunk]->Get_Light(tile);
        bool underground = !ChunkMap[chunk]->Get_Air(tile);
        
        std::vector<std::pair<glm::vec3, glm::vec3>> neighbors = Get_Neighbors(chunk, tile);
        
        for (auto neighbor : neighbors) {
            if (!ChunkMap.count(neighbor.first)) continue;
            if (!ChunkMap[neighbor.first]->Get_Block(neighbor.second)) continue;
            if (!ChunkMap[neighbor.first]->Get_Air(neighbor.second) && underground) continue;
            if (!ChunkMap[neighbor.first]->Is_Top(neighbor.second)) continue;
            if (ChunkMap[neighbor.first]->Get_Light(neighbor.second) + 2 >= lightLevel) continue;
            
            ChunkMap[neighbor.first]->Set_Light(neighbor.second, lightLevel - 1);
            
            LightNode node;
            node.Chunk = neighbor.first;
            node.Tile = neighbor.second;
            sunlightQueue.push(node);
            
            meshingList.insert(neighbor.first);
        }
    }
    
    for (auto const ch : meshingList) {
        if (ChunkMap[ch] != nullptr) {
            ChunkMap[ch]->Mesh();
        }
    }
}

std::vector<glm::vec3> Player::Hitscan() {
    std::vector<glm::vec3> checkingChunkTile;
    std::vector<glm::vec3> failedScan = {glm::vec3(0)};

    glm::vec3 checkingPos;
	glm::vec3 lastPos;

	bool started = false;

    for (float t = 0; t < PLAYER_RANGE; t += HITSCAN_STEP_SIZE) {
        checkingPos = Cam.Position + Cam.Front * t;

		EditingChunkMap = true;

		if (IsBlock(checkingPos)) {
			if (started) {
				std::vector<glm::vec3> result1 = Get_Chunk_Pos(checkingPos);
				std::vector<glm::vec3> result2 = Get_Chunk_Pos(lastPos);

				result1.insert(result1.end(), result2.begin(), result2.end());

				return result1;
			}

			return Get_Chunk_Pos(checkingPos);
		}

		EditingChunkMap = false;

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

void Player::Clear_Keys() {
    std::fill_n(keys, 1024, false);
}

void Player::MouseHandler(double posX, double posY) {
    if (MovedMouse) {
        LastMousePos = glm::dvec2(posX, posY);
        MovedMouse = false;
    }
    
    if (ShowMenu) {
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
        LookingTile = hit[1];

		LookingAirChunk = hit[2];
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
                if (ChunkMap[LookingChunk]->Get_Block(LookingTile) == 11) {
                    Remove_Torch();
                }
                
                LightNode node;
                node.Chunk = LookingChunk;
                node.Tile = LookingTile;
                node.LightLevel = ChunkMap[LookingChunk]->Get_Light(LookingTile);
                sunlightQueue.push(node);
                
                ChunkMap[LookingChunk]->Remove_Block(LookingTile);
                MouseHandler(LastMousePos.x, LastMousePos.y);
                
                Process_Sunlight();
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
				
                EditingChunkMap = false;
                
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
	EditingChunkMap = true;
    
    std::map<glm::vec3, Chunk*>::iterator it = ChunkMap.begin();
    
    while (it != ChunkMap.end()) {
        double dist = pow(CurrentChunk.x - it->first.x, 2) + pow(CurrentChunk.z - it->first.z, 2);
        
        if (dist > pow(RENDER_DISTANCE, 2)) {
            delete it->second;
            it->second = nullptr;
            it = ChunkMap.erase(it);
        }
        else {
            it++;
        }
    }

	EditingChunkQueue = true;

    for (int x = (int) CurrentChunk.x - RENDER_DISTANCE; x <= CurrentChunk.x + RENDER_DISTANCE; x++) {
        for (int z = (int) CurrentChunk.z - RENDER_DISTANCE; z <= CurrentChunk.z + RENDER_DISTANCE; z++) {
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

                bool inRange = pow(CurrentChunk.x - x, 2) + pow(CurrentChunk.z - z, 2) <= pow(RENDER_DISTANCE, 2);

                if (!inRange) {
					continue;
                }

				ChunkQueue.push(new Chunk(pos));
                ChunkSet.insert(pos);
            }
        }
    }

	EditingChunkQueue = false;
	EditingChunkMap = true;
}

bool IsBlock(glm::vec3 pos) {
	std::vector<glm::vec3> chunkPos = Get_Chunk_Pos(pos);

	bool isBlock = false;
	EditingChunkMap = true;

	if (ChunkMap.count(chunkPos[0])) {
		isBlock = ChunkMap[chunkPos[0]]->Get_Block(chunkPos[1]) > 0;
	}

	EditingChunkMap = false;
	return isBlock;
}