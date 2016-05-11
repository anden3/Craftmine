#include "Player.h"

#include <sstream>
#include <dirent.h>

const float PLAYER_BASE_SPEED  = 3.0f;
const float PLAYER_SPRINT_MODIFIER = 1.5f;

const double PLAYER_SENSITIVITY = 0.25;
const float PLAYER_RANGE = 5.0f;

const float PLAYER_WIDTH = 0.1f;

const float CAMERA_HEIGHT = 1.7f;

const float VOLUME = 0.1f;

const float HITSCAN_STEP_SIZE = 0.1f;

const float JUMP_HEIGHT = 0.1f;

const int TORCH_LIGHT_LEVEL = 12;

const float ATTRACT_RANGE = 4.0f;
const float PICKUP_RANGE = 1.5f;
const float ATTRACT_SPEED = 1.0f;

std::set<Chunk*> lightMeshingList;

glm::vec3 lastChunk(-5);

bool keys[1024] = {0};
bool MouseDown = false;
bool Creative = false;

int NumKeys[10] = {GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0};

std::vector<SoundPlayer> soundPlayers;
std::map<std::string, std::vector<Sound>> Sounds;

std::vector<std::string> Split(const std::string &s, char delim) {
    std::vector<std::string> elements;
    std::stringstream ss(s);
    std::string item;
    
    while (std::getline(ss, item, delim)) {
        elements.push_back(item);
    }
    
    return elements;
}

void Upload_Data(const unsigned int vbo, const Data &data) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Draw_Cube(unsigned const int vao, const glm::mat4 model, int vertices) {
    modelShader->Bind();
    
    glUniformMatrix4fv(glGetUniformLocation(modelShader->Program, "model"), 1, false, glm::value_ptr(model));
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    modelShader->Unbind();
}

void Extend(Data &storage, const Data input) {
    for (auto const &object : input) {
        storage.push_back(object);
    }
}

void Init_3D_Textured(unsigned int &vao, unsigned int &vbo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Data Create_Textured_Cube(const int type, glm::vec3 offset) {
    Data data;
    
    glm::vec2 texPosition = textureCoords[type];
    static float textureStep = (1.0f / 16.0f);
    
    float texStartX = textureStep * (texPosition.x - 1.0f);
    float texStartY = textureStep * (texPosition.y - 1.0f);
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            Extend(data, Data {vertices[i][j][0] + offset.x, vertices[i][j][1] + offset.y, vertices[i][j][2] + offset.z});
            
            if (type == 2) {
                data.push_back(textureStep * (grassTextures[i].x - 1.0f) + tex_coords[i][j][0] * textureStep);
                data.push_back(textureStep * (grassTextures[i].y - 1.0f) + tex_coords[i][j][1] * textureStep);
            }
            else {
                data.push_back(texStartX + tex_coords[i][j][0] * textureStep);
                data.push_back(texStartY + tex_coords[i][j][1] * textureStep);
            }
        }
    }
    
    return data;
}


void Player::Init() {
    Init_Model();
    Init_Holding();
    Init_Damage();
    
    Init_Sounds();
}

void Player::Init_Model() {
    Data data = Create_Textured_Cube(5, glm::vec3(-0.5f, -0.5f + CAMERA_HEIGHT, -0.5f));
    
    Init_3D_Textured(ModelVAO, ModelVBO);
    Upload_Data(ModelVBO, data);
}

void Player::Init_Holding() {
    Init_3D_Textured(HoldingVAO, HoldingVBO);
}

void Player::Init_Damage() {
    Init_3D_Textured(DamageVAO, DamageVBO);
}

void Player::Init_Sounds() {
    DIR *dir = opendir("sounds");
    struct dirent *ent;
    
    while ((ent = readdir(dir)) != nullptr) {
        std::string name(ent->d_name);
        int divPos = int(name.find('_'));
        int dotPos = int(name.find('.'));
        
        if (divPos != std::string::npos) {
            std::string type = name.substr(0, divPos);
            std::string fileName = name.substr(0, dotPos);
            
            Sounds[type].push_back(Sound(fileName));
        }
    }
    
    closedir(dir);
}


void Player::Mesh_Holding() {
    CurrentBlock = inventory.Get_Info().first;
    
    if (CurrentBlock == 0) {
        return;
    }
    
    Data data = Create_Textured_Cube(CurrentBlock);
    Upload_Data(HoldingVBO, data);
}

void Player::Mesh_Damage(int index) {
    Data data = Create_Textured_Cube(246 + index, glm::vec3(0.0f));
    Upload_Data(DamageVBO, data);
}


void Player::Draw_Model() {
    glm::mat4 model;
    model = glm::translate(model, WorldPos);
    model = glm::rotate(model, float(glm::radians(270.0f - Cam.Yaw)), glm::vec3(0, 1, 0));
    
    Draw_Cube(ModelVAO, model);
}

void Player::Draw_Holding() {
    if (CurrentBlock == 0) {
        return;
    }
    
    glm::vec3 offsetFront = Cam.Front;
    glm::vec3 offsetRight = Cam.Right;
    
    offsetFront *= 0.5;
    offsetRight *= 0.5;
    
    glm::mat4 model;
    model = glm::translate(model, WorldPos + offsetFront + offsetRight + glm::vec3(0, 0.5, 0));
    model = glm::rotate(model, float(glm::radians(270.0f - Cam.Yaw)), glm::vec3(0, 1, 0));
    model = glm::rotate(model, float(glm::radians(Cam.Pitch)), glm::vec3(1, 0, 0));
    
    Draw_Cube(HoldingVAO, model);
}

void Player::Draw_Damage() {
    if (!LookingAtBlock || !MouseDown || Creative) {
        return;
    }
    
    glm::mat4 model;
    model = glm::translate(model, Get_World_Pos(LookingChunk, LookingTile));
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-50.0f, -50.0f);
    Draw_Cube(DamageVAO, model);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void Player::PollSounds() {
	if (soundPlayers.size() > 0) {
        std::vector<SoundPlayer>::iterator sound = soundPlayers.begin();
        
        while (sound != soundPlayers.end()) {
            if (!sound->Playing()) {
                sound = soundPlayers.erase(sound);
            }
            else {
                ++sound;
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
    
    Check_Pickup();
    
    if (MouseDown && LookingAtBlock) {
        MouseTimer += deltaTime;
        
        int blockType = ChunkMap[LookingChunk]->Get_Block(LookingTile);
        float requiredTime = blockHardness[blockType] * 1.5f * 3.33f;
        
        if (!OnGround) {
            requiredTime *= 5;
        }
        
        if (MouseTimer >= requiredTime) {
            Break_Block();
            MouseTimer = 0.0;
            update = true;
        }
        else {
            Mesh_Damage(floor(MouseTimer / requiredTime * 10));
        }
    }
    
    if (update || WorldPos != prevPos) {
        std::vector<glm::vec3> chunkPos = Get_Chunk_Pos(WorldPos);
        CurrentChunk = chunkPos[0];
        CurrentTile = chunkPos[1];
        
        if (Exists(CurrentChunk) && ChunkMap[CurrentChunk]->Get_Block(CurrentTile - glm::vec3(0, 1, 0)) != 0) {
            LightLevel = ChunkMap[CurrentChunk]->Get_Light(CurrentTile - glm::vec3(0, 1, 0));
            
            if (LightLevel == 0) {
                if (WorldPos.y >= topBlocks[glm::vec2(CurrentChunk.x, CurrentChunk.z)][glm::vec2(CurrentTile.x, CurrentTile.z)] - 1) {
                    LightLevel = SUN_LIGHT_LEVEL;
                }
            }
            
            modelShader->Bind();
            glUniform1i(glGetUniformLocation(modelShader->Program, "lightLevel"), LightLevel);
            modelShader->Unbind();
        }
        
        Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + CAMERA_HEIGHT, WorldPos.z);
        
        if (ThirdPerson) {
            glm::vec3 frontDir = Cam.FrontDirection;
            frontDir *= -2;
            
            Cam.Position += glm::vec3(0, 2, 0) + frontDir;
        }
        
        listener.Set_Position(Cam.Position);
        
        std::vector<glm::vec3> hit = Hitscan();
        
        LookingAtBlock = (hit.size() == 4);
        
        if (LookingAtBlock) {
            if (MouseTimer > 0.0 && LookingTile != hit[1]) {
                MouseTimer = 0.0;
            }
            
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

void Player::Check_Pickup() {
    glm::vec3 playerCenter = WorldPos + glm::vec3(0, 1, 0);
    
    std::vector<EntityInstance*>::iterator it = Entities.begin();
    
    while (it != Entities.end()) {
        if (!(*it)->Can_Move) {
            ++it;
            continue;
        }
        
        float dist = glm::distance((*it)->Position, playerCenter);
        
        if (dist < PICKUP_RANGE) {
            int blockType = (*it)->Type;
            
            if (blockType == 2) {
                blockType = 3;
            }
            
            Play_Sound("pickup", CurrentChunk, CurrentTile);
            
            inventory.Add_Stack(blockType, (*it)->Size);
            Mesh_Holding();
            
            delete *it;
            it = Entities.erase(it);
        }
        else if (dist < ATTRACT_RANGE && (*it)->Size > 0) {
            glm::vec3 vector = glm::normalize(playerCenter - (*it)->Position);
            vector *= (ATTRACT_SPEED * (ATTRACT_RANGE - dist));
            
            (*it)->Velocity.x += vector.x;
            (*it)->Velocity.z += vector.z;
            
            ++it;
        }
        else {
            ++it;
        }
    }
    
    if (CurrentBlock == 0 && inventory.Get_Info().first) {
        Mesh_Holding();
    }
}

void Player::Teleport(glm::vec3 pos) {
    Velocity = glm::vec3(0);
    WorldPos = pos;
    Move(0.0f, true);
}

void Player::Drop_Item() {
    if (CurrentBlock != 0) {
        inventory.Decrease_Size();
        
        glm::vec3 position = Cam.Front;
        
        glm::vec3 velocityVector = glm::vec3(Cam.Front.x, 0, Cam.Front.z);
        velocityVector *= 2;
        
        Entity::Spawn(WorldPos + glm::vec3(0, CAMERA_HEIGHT, 0) + position, CurrentBlock, velocityVector);
        Mesh_Holding();
    }
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

void Player::Place_Light(int lightLevel) {
    ChunkMap[LookingAirChunk]->Set_Light(LookingAirTile, lightLevel);
    ChunkMap[LookingAirChunk]->LightQueue.emplace(LookingAirChunk, LookingAirTile);
    
    ChunkMap[LookingAirChunk]->Light();
    ChunkMap[LookingAirChunk]->Mesh();
}

void Player::Remove_Light() {
    std::queue<LightNode> lightRemovalQueue;
    
    ChunkMap[LookingChunk]->LightRemovalQueue.emplace(LookingChunk, LookingTile, ChunkMap[LookingChunk]->Get_Light(LookingTile));
    ChunkMap[LookingChunk]->Set_Light(LookingTile, 0);
    
    ChunkMap[LookingChunk]->Light();
    ChunkMap[LookingChunk]->Mesh();
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
            
            Mesh_Holding();
        }
        
        if (key == GLFW_KEY_Q) {
            Drop_Item();
        }
        
        if (key == GLFW_KEY_V) {
            ThirdPerson = !ThirdPerson;
            Move(0.0f, true);
        }
        
        // Pressing a number key
        if (!inventory.Is_Open) {
            for (int i = 0; i < 10; i++) {
                if (key == NumKeys[i]) {
                    inventory.ActiveToolbarSlot = i;
                    inventory.Switch_Slot();
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

    if (Cam.Pitch > 89.9) {
        Cam.Pitch = 89.9;
    }
    
    if (Cam.Pitch < -89.9) {
        Cam.Pitch = -89.9;
    }
    
    if (Cam.Yaw > 360) {
        Cam.Yaw -= 360;
    }
    
    if (Cam.Yaw < 0) {
        Cam.Yaw += 360;
    }
    
    LastMousePos = glm::dvec2(posX, posY);
    Cam.UpdateCameraVectors();

	listener.Set_Orientation(Cam.Front, Cam.Up);

    std::vector<glm::vec3> hit = Hitscan();

    if (hit.size() == 4) {
        LookingAtBlock = true;
        
        if (MouseTimer > 0.0 && LookingTile != hit[1]) {
            MouseTimer = 0.0;
        }
        
        LookingChunk = hit[0];
        LookingAirChunk = hit[2];
        
        LookingTile = hit[1];
		LookingAirTile = hit[3];
    }
    else {
        LookingAtBlock = false;
    }
    
    if (ThirdPerson) {
        glm::vec3 frontDir = Cam.FrontDirection;
        frontDir *= -2;
        
        Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + CAMERA_HEIGHT + 2, WorldPos.z) + frontDir;
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
        Mesh_Holding();
    }
}

void Player::ClickHandler(int button, int action) {
    if (!MouseEnabled) {
        MouseDown = (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT);
        
        if (!MouseDown || Creative) {
            MouseTimer = 0.0;
        }
        
        if (MouseDown && LookingAtBlock && Creative) {
            Break_Block();
        }
        
        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
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
                    
                    inventory.Decrease_Size();
                    CurrentBlock = inventory.Get_Info().first;
                    
                    if (CurrentBlock == 11) {
                        Place_Light(TORCH_LIGHT_LEVEL);
                    }
                }
            }
        }
    }
    
    MouseHandler(LastMousePos.x, LastMousePos.y);
}

void Player::Break_Block() {
    Chunk* lookingChunk = ChunkMap[LookingChunk];
    
    int blockType = lookingChunk->Get_Block(LookingTile);
    
    if (blockType == 11) {
        Remove_Light();
    }
    
    for (auto const &types : BlockSounds) {
        if (std::find(types.second.begin(), types.second.end(), blockType) != types.second.end()) {
            Play_Sound(types.first, LookingChunk, LookingTile);
            break;
        }
    }
    
    lookingChunk->Remove_Block(LookingTile);
    Entity::Spawn(Get_World_Pos(LookingChunk, LookingTile), blockType);
}

void Player::Play_Sound(std::string type, glm::vec3 chunk, glm::vec3 tile) {
	SoundPlayer soundPlayer;

	soundPlayer.Set_Position(Get_World_Pos(chunk, tile));
	soundPlayer.Set_Volume(VOLUME);
    
    std::vector<Sound>::iterator sound = Sounds[type].begin();
    std::advance(sound, std::rand() % Sounds[type].size());
    
	soundPlayer.Add((*sound));
	soundPlayer.Play();

	soundPlayers.push_back(soundPlayer);
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
        player.Mesh_Holding();
        return "Inventory cleared!";
    }
    
    else if (command == "gamemode") {
        if (parameters[1] == "c") {
            Creative = true;
            return "Gamemode changed to Creative";
        }
        else if (parameters[1] == "s") {
            Creative = false;
            return "Gamemode changed to Survival";
        }
        else {
            return "Error! Invalid gamemode";
        }
    }
    
    else {
        return "Error! Command not recognized";
    }
}