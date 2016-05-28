#include "Player.h"

#include <cmath>
#include <sstream>
#include <dirent.h>

#include "Chat.h"
#include "Chunk.h"
#include "Sound.h"
#include "Camera.h"
#include "Entity.h"
#include "Shader.h"
#include "Interface.h"
#include "Inventory.h"

const float PLAYER_BASE_SPEED  = 3.0f;
const float PLAYER_SPRINT_MODIFIER = 1.5f;

const double PLAYER_SENSITIVITY = 0.25;
const float PLAYER_RANGE = 5.0f;

const float PLAYER_WIDTH = 0.1f;

const float CAMERA_HEIGHT = 1.7f;

const float VOLUME = 0.1f;

const float HITSCAN_STEP_SIZE = 0.1f;

const float JUMP_HEIGHT = 0.1f;

const float ATTRACT_RANGE = 4.0f;
const float PICKUP_RANGE = 1.5f;
const float ATTRACT_SPEED = 1.0f;

const int PLAYER_TEXTURE_UNIT = 5;

const float MOVEMENT_ANGLE_START = -45.0f;
const float MOVEMENT_ANGLE_END = 45.0f;

float PUNCHING_ANGLE_START = 30.0f;
float PUNCHING_ANGLE_END = 60.0f;

std::set<Chunk*> lightMeshingList;

glm::vec3 lastChunk(-5);

bool keys[1024] = {0};
bool MouseDown = false;

int ModelShaderModelLoc;
int ModelShaderTexLoc;

int NumKeys[10] = {GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0};

Buffer HoldingBuffer;
Buffer DamageBuffer;

Buffer HeadBuffer;
Buffer BodyBuffer;
Buffer LeftArmBuffer;
Buffer RightArmBuffer;
Buffer LeftLegBuffer;
Buffer RightLegBuffer;

int punchingAngleDirection = 200;
float punchingAngle = 0.0f;

int movementAngleDirection = 5000;
float movementAngle = 0.0f;

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

Data Create_Textured_Cube(const int type, glm::vec3 offset) {
    Data data;
    
    glm::vec2 texPosition = textureCoords[type];
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (CustomVertices.count(type)) {
                Extend(data, CustomVertices[type][i][vertices[i][j][0]]);
            }
            else {
                Extend(data, vertices[i][j] + offset);
            }
            
            if (CustomTexCoords.count(type)) {
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j].x].x / IMAGE_SIZE.x);
                data.push_back(CustomTexCoords[type][i][tex_coords[i][j].y].y / IMAGE_SIZE.y);
            }
            else if (MultiTextures.count(type)) {
                Extend(data, (MultiTextures[type][i] - 1.0f + tex_coords[i][j]) / IMAGE_SIZE);
            }
            else {
                Extend(data, (texPosition - 1.0f + tex_coords[i][j]) / IMAGE_SIZE);
            }
        }
    }
    
    return data;
}

void Player::Init() {
    glm::vec4 hpDims(50, 50, 200, 20);
    glm::vec3 hpRange(0, 100, 100);
    
    ModelShaderModelLoc = modelShader->Get_Location("model");
    ModelShaderTexLoc = modelShader->Get_Location("tex");
    
    HoldingBuffer.Init(modelShader);
    DamageBuffer.Init(modelShader);
    
    HoldingBuffer.Create(3, 2);
    DamageBuffer.Create(3, 2);
    
    interface.Set_Document("playerUI");
    interface.Add_Bar("health", "HP", hpDims, hpRange);
    interface.Set_Document("");
    
    Init_Model();
    Init_Sounds();
}

void Player::Init_Model() {
    glActiveTexture(GL_TEXTURE0 + PLAYER_TEXTURE_UNIT);
    unsigned int texture = std::get<0>(Load_Texture("skin.png"));
    glBindTexture(GL_TEXTURE_2D, texture);
    
    Buffer* buffers[6] = { &HeadBuffer, &BodyBuffer, &LeftArmBuffer, &RightArmBuffer, &LeftLegBuffer, &RightLegBuffer };
    glm::vec3 offsets[6] = {
        glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(2.0f, 1.0f, 0.5f),
        glm::vec3(-1.0f, 1.0f, 0.5f), glm::vec3(1.0f, 1.0f, 0.5f), glm::vec3(0.0f, 1.0f, 0.5f)
    };
    std::string parts[6] = { "head", "body", "arm", "arm", "leg", "leg" };
    
    for (int b = 0; b < 6; b++) {
        buffers[b]->Init(modelShader);
        
        Data data;
        
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                Extend(data, vertices[i][j] - offsets[b]);
                data.push_back(PlayerTexCoords[parts[b]][i][tex_coords[i][j].x].x / 64);
                data.push_back(PlayerTexCoords[parts[b]][i][tex_coords[i][j].y].y / 32);
            }
        }
        
        buffers[b]->Create(3, 2, data);
    }
}

void Player::Init_Sounds() {
    DIR *dir = opendir("sounds");
    struct dirent *ent;
    
    while ((ent = readdir(dir)) != nullptr) {
        std::string name(ent->d_name);
        
        if (name.find('_') != std::string::npos) {
            Sounds[name.substr(0, name.find('_'))].push_back(Sound(name.substr(0, name.find('.'))));
        }
    }
    
    closedir(dir);
}


void Player::Mesh_Holding() {
    CurrentBlock = inventory.Get_Info().Type;
    
    if (CurrentBlock > 0) {
        HoldingBuffer.Upload(Create_Textured_Cube(CurrentBlock));
    }
}

void Player::Mesh_Damage(int index) {
    DamageBuffer.Upload(Create_Textured_Cube(246 + index, glm::vec3(0)));
}

void Player::Draw_Model() {
    if (!ThirdPerson && !MouseDown) {
        return;
    }
    
    modelShader->Upload(ModelShaderTexLoc, PLAYER_TEXTURE_UNIT);
    
    static glm::vec3 translateOffsets[6] = {
        glm::vec3(0, 1.5, 0), glm::vec3(0, 0.75, 0), glm::vec3(0, 1.50, 0),
        glm::vec3(0, 1.5, 0), glm::vec3(0, 0.75, 0), glm::vec3(0, 0.75, 0)
    };
    
    static glm::vec3 scalingFactors[6] = {
        glm::vec3(0.5), glm::vec3(0.5, 0.75, 0.25), glm::vec3(0.25, 0.75, 0.25),
        glm::vec3(0.25, 0.75, 0.25), glm::vec3(0.25, 0.75, 0.25), glm::vec3(0.25, 0.75, 0.25)
    };
    
    static Buffer* buffers[6] = { &HeadBuffer, &BodyBuffer, &LeftArmBuffer, &RightArmBuffer, &LeftLegBuffer, &RightLegBuffer};
    
    if (!ThirdPerson) { // Only draw right arm if holding mouse down and in first person.
        glm::mat4 model;
        model = glm::translate(model, WorldPos + translateOffsets[3]);
        model = glm::rotate(model, glm::radians(punchingAngle), Cam.Right);
        model = glm::rotate(model, Rotation, glm::vec3(0, 1, 0));
        model = glm::scale(model, scalingFactors[3]);
        modelShader->Upload(ModelShaderModelLoc, model);
        buffers[3]->Draw();
    }
    else {
        for (int i = 0; i < 6; i++) {
            glm::mat4 model;
            model = glm::translate(model, WorldPos + translateOffsets[i]);
            float angle = glm::radians((i == 3 && MouseDown) ? punchingAngle : movementAngle) * ((i == 2 || i == 5) ? -1 : 1);
            
            if (i >= 2) { // Rotate body parts
                model = glm::rotate(model, angle, Cam.Right);
            }
            
            model = glm::rotate(model, Rotation, glm::vec3(0, 1, 0));
            
            if (i == 0) { // Rotate head up/down
                model = glm::rotate(model, float(glm::radians(Cam.Pitch)), glm::vec3(1, 0, 0));
            }
            
            model = glm::scale(model, scalingFactors[i]);
            
            modelShader->Upload(ModelShaderModelLoc, model);
            buffers[i]->Draw();
        }
    }
    
    if (CurrentBlock != 0) {
        float angle = glm::radians((MouseDown ? punchingAngle : movementAngle) - 90);
        glm::vec3 offset = WorldPos + glm::vec3(0, 1.6f + glm::sin(angle), 0) + Cam.RightDirection * 0.37f + Cam.FrontDirection * glm::cos(angle);
        
        glm::mat4 model;
        model = glm::translate(model, offset);
        model = glm::rotate(model, angle, glm::vec3(Cam.Right));
        model = glm::rotate(model, Rotation, glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.2f));
        
        modelShader->Upload(ModelShaderModelLoc, model);
        modelShader->Upload(ModelShaderTexLoc, 0);
        HoldingBuffer.Draw();
    }
}

void Player::Draw_Holding() {
    if (CurrentBlock == 0) {
        return;
    }
    
    glm::mat4 model;
    model = glm::translate(model, WorldPos + (Cam.Front + Cam.Right) * 0.5f + glm::vec3(0, 0.5, 0));
    model = glm::rotate(model, float(glm::radians(270.0f - Cam.Yaw)), glm::vec3(0, 1, 0));
    model = glm::rotate(model, float(glm::radians(Cam.Pitch)), glm::vec3(1, 0, 0));
    
    modelShader->Upload(ModelShaderModelLoc, model);
    modelShader->Upload(ModelShaderTexLoc, 0);
    HoldingBuffer.Draw();
}

void Player::Draw_Damage() {
    if (!LookingAtBlock || !MouseDown || Creative) {
        return;
    }
    
    glm::mat4 model;
    modelShader->Upload(ModelShaderModelLoc, glm::translate(model, Get_World_Pos(LookingChunk, LookingTile)));
    modelShader->Upload(ModelShaderTexLoc, 0);
    
    // "Fix" for Z-Fighting
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-4.0f, -4.0f);
    DamageBuffer.Draw();
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void Player::Poll_Sounds() {
	if (soundPlayers.size() > 0) {
        std::vector<SoundPlayer>::iterator sound = soundPlayers.begin();
        
        while (sound != soundPlayers.end()) {
            sound = sound->Playing() ? sound + 1 : soundPlayers.erase(sound);
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
        movementAngle = 0.0f;
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
        
        if (Velocity.xz() != glm::vec2(0)) {
            if (movementAngle >= MOVEMENT_ANGLE_END && movementAngleDirection > 0) {
                movementAngleDirection *= -1;
            }
            else if (movementAngle <= MOVEMENT_ANGLE_START && movementAngleDirection < 0) {
                movementAngleDirection *= -1;
            }
            
            movementAngle += deltaTime * movementAngleDirection * speed;
        }
        else {
            movementAngle = 0.0f;
        }
	}
    
    if (MouseDown) {
        punchingAngle += deltaTime * punchingAngleDirection;
        
        if (punchingAngle >= PUNCHING_ANGLE_END && punchingAngleDirection > 0) {
            punchingAngle = PUNCHING_ANGLE_END;
            punchingAngleDirection *= -1;
        }
        else if (punchingAngle <= PUNCHING_ANGLE_START && punchingAngleDirection < 0) {
            punchingAngle = PUNCHING_ANGLE_START;
            punchingAngleDirection *= -1;
        }
    }
    else {
        punchingAngle = PUNCHING_ANGLE_START;
    }
    
    Check_Pickup();
    
    if (MouseDown && LookingAtBlock) {
        if (Creative) {
            MouseTimer = 0.0;
            Break_Block();
        }
        else {
            MouseTimer += deltaTime;
            
            float requiredTime = 0.0f;
            int blockType = ChunkMap[LookingChunk]->Get_Block(LookingTile);
            
            if (BlockHardness.count(blockType)) {
                requiredTime = BlockHardness[blockType] * 1.5f * 3.33f * (OnGround ? 1 : 5);
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
    }
    
    if (update || WorldPos != prevPos) {
        std::tie(CurrentChunk, CurrentTile) = Get_Chunk_Pos(WorldPos);
        
        if (Exists(CurrentChunk) && ChunkMap[CurrentChunk]->Get_Block(CurrentTile - glm::vec3(0, 1, 0)) != 0) {
            LightLevel = ChunkMap[CurrentChunk]->Get_Light(CurrentTile - glm::vec3(0, 1, 0));
            
            if (LightLevel == 0) {
                if (WorldPos.y >= topBlocks[CurrentChunk.xz()][CurrentTile.xz()] - 1) {
                    LightLevel = SUN_LIGHT_LEVEL;
                }
            }
            
            modelShader->Upload("lightLevel", LightLevel);
        }
        
        Cam.Position = WorldPos + glm::vec3(0, CAMERA_HEIGHT, 0);
        
        if (ThirdPerson) {
            Cam.Position -= Cam.Front * 2.0f;
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
            Render_Chunks();
        }
    }
}

void Player::Draw() {
    Draw_Damage();
    
    if (!ThirdPerson) {
        glClear(GL_DEPTH_BUFFER_BIT);
        
        if (!MouseDown && CurrentBlock > 0) {
            Draw_Holding();
        }
    }
    
    Draw_Model();
    
    interface.Draw_Document("playerUI");
}

void Player::Check_Pickup() {
    if (Entities.empty()) {
        return;
    }
    
    glm::vec3 playerCenter = WorldPos + glm::vec3(0, 1, 0);
    
    std::vector<EntityInstance*>::iterator it = Entities.begin();
    
    while (it != Entities.end()) {
        if (!(*it)->Can_Move) {
            ++it;
            continue;
        }
        
        float dist = glm::distance((*it)->Position, playerCenter);
        
        if (dist < PICKUP_RANGE) {
            Play_Sound("pickup", CurrentChunk, CurrentTile);
            inventory.Add_Stack((*it)->Type, (*it)->Size);
            Mesh_Holding();
            
            delete *it;
        }
        else if (dist < ATTRACT_RANGE && (*it)->Size > 0) {
            glm::vec3 vector = glm::normalize(playerCenter - (*it)->Position) * ATTRACT_SPEED * (ATTRACT_RANGE - dist);
            (*it)->Velocity += glm::vec3(vector.x, 0, vector.z);
        }
        
        it = (dist < PICKUP_RANGE) ? Entities.erase(it) : it + 1;
    }
    
    if (CurrentBlock == 0 && inventory.Get_Info().Type) {
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
        Entity::Spawn(WorldPos + glm::vec3(0, CAMERA_HEIGHT, 0) + Cam.Front, CurrentBlock, glm::vec3(Cam.Front.x, 0, Cam.Front.z) * 2.0f);
        Mesh_Holding();
    }
}

void Player::ColDetection() {
	if (ChunkMap.empty() || !Exists(CurrentChunk)) {
		return;
	}

	Velocity.y -= GRAVITY;
    
    OnGround = (Velocity.y <= 0 && Is_Block(glm::vec3(WorldPos.x, WorldPos.y + Velocity.y - 0.01f, WorldPos.z)));
    
    if (OnGround) {
        Velocity.y = 0;
    }
    
    else if (Velocity.y != 0) {
        if (!Is_Block(glm::vec3(WorldPos.x, WorldPos.y + Velocity.y + ((Velocity.y > 0) ? CAMERA_HEIGHT : 0), WorldPos.z))) {
            WorldPos.y += Velocity.y;
        }
        else if (Velocity.y > 0) {
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
            std::vector<glm::vec3> result;
            glm::vec3 chunk1, tile1;
            
            std::tie(chunk1, tile1) = Get_Chunk_Pos(checkingPos);
            Extend(result, chunk1, tile1);
            
			if (started) {
                glm::vec3 chunk2, tile2;
                std::tie(chunk2, tile2) = Get_Chunk_Pos(lastPos);
                Extend(result, chunk2, tile2);
			}
            
			return result;
		}

		lastPos = checkingPos;
		started = true;
    }

    return failedScan;
}

void Player::Key_Handler(int key, int action) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_F) {
            Flying = !Flying;
        }

        else if (key == GLFW_KEY_SPACE && OnGround) {
            Jumping = true;
        }
        
        else if (key == GLFW_KEY_TAB) {
            inventory.Is_Open = !inventory.Is_Open;
            inventory.Mesh();
            
            if (inventory.Is_Open) {
                inventory.Mouse_Handler();
            }
            
            Mesh_Holding();
        }
        
        else if (key == GLFW_KEY_Q) {
            Drop_Item();
        }
        
        else if (key == GLFW_KEY_V) {
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
        keys[key] = (action == GLFW_PRESS) ? true : ((action == GLFW_RELEASE) ? false : keys[key]);
    }
}

void Player::Mouse_Handler(double posX, double posY) {
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
    
    else if (Cam.Pitch < -89.9) {
        Cam.Pitch = -89.9;
    }
    
    if (Cam.Yaw > 360) {
        Cam.Yaw -= 360;
    }
    
    else if (Cam.Yaw < 0) {
        Cam.Yaw += 360;
    }
    
    LastMousePos = glm::dvec2(posX, posY);
    Cam.UpdateCameraVectors();
    
    Rotation = float(glm::radians(270.0f - Cam.Yaw));
    
    PUNCHING_ANGLE_START = Cam.Pitch + 90;
    PUNCHING_ANGLE_END = Cam.Pitch + 120;

	listener.Set_Orientation(Cam.Front, Cam.Up);

    std::vector<glm::vec3> hit = Hitscan();
    
    LookingAtBlock = hit.size() == 4;

    if (hit.size() == 4) {
        if (MouseTimer > 0.0 && LookingTile != hit[1]) {
            MouseTimer = 0.0;
        }
        
        LookingChunk = hit[0];
        LookingAirChunk = hit[2];
        
        LookingTile = hit[1];
		LookingAirTile = hit[3];
    }
    
    if (ThirdPerson) {
        Cam.Position = glm::vec3(WorldPos.x, WorldPos.y + CAMERA_HEIGHT, WorldPos.z) - Cam.Front * 2.0f;
    }
    
    // Frustrum Culling Chunks
    for (auto const &chunk : ChunkMap) {
        if (glm::distance(CurrentChunk.xz(), chunk.first.xz()) <= 2) {
            chunk.second->Visible = true;
        }
        else {
            float dot = 0;
            
            if (chunk.first.xz() != CurrentChunk.xz()) {
                dot = glm::dot(Cam.FrontDirection.xz(), glm::normalize(chunk.first.xz() - CurrentChunk.xz()));
            }
            
            chunk.second->Visible = glm::degrees(glm::acos(dot)) <= DEFAULT_FOV;
        }
    }
}

void Player::Scroll_Handler(double offsetY) {
    if (fabs(offsetY) > 0.2) {
        inventory.ActiveToolbarSlot += int(std::copysign(1, offsetY));
        
        if (inventory.ActiveToolbarSlot > 9) {
            inventory.ActiveToolbarSlot = 0;
        }
        
        else if (inventory.ActiveToolbarSlot < 0) {
            inventory.ActiveToolbarSlot = 9;
        }
        
        inventory.Switch_Slot();
        Mesh_Holding();
    }
}

void Player::Click_Handler(int button, int action) {
    if (!MouseEnabled) {
        MouseDown = (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT);
        
        if (!MouseDown || Creative) {
            MouseTimer = 0.0;
        }
        
        if (MouseDown && LookingAtBlock && Creative) {
            Break_Block();
        }
        
        if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (LookingAtBlock && CurrentBlock && CurrentBlock < 246) {
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
                    CurrentBlock = inventory.Get_Info().Type;
                    
                    if (BlockLuminosity.count(CurrentBlock)) {
                        Place_Light(BlockLuminosity[CurrentBlock]);
                    }
                }
            }
        }
    }
    
    Mouse_Handler(LastMousePos.x, LastMousePos.y);
}

void Player::Break_Block() {
    Chunk* lookingChunk = ChunkMap[LookingChunk];
    
    int blockType = lookingChunk->Get_Block(LookingTile);
    
    if (blockType == 50) {
        Remove_Light();
    }
    
    for (auto const &types : BlockSounds) {
        if (std::find(types.second.begin(), types.second.end(), blockType) != types.second.end()) {
            Play_Sound(types.first, LookingChunk, LookingTile);
            break;
        }
    }
    
    if (blockType == 1) {
        blockType = 4;
    }
    
    else if (blockType == 2) {
        blockType = 3;
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

void Player::Render_Chunks() {
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
        bool inRange = dist > pow(RenderDistance, 2) || chunk->first.y > startY || chunk->first.y < endY;
        
        if (inRange) {
            delete chunk->second;
        }
        
        chunk = inRange ? ChunkMap.erase(chunk) : ++chunk;
    }

    for (int x = (int) CurrentChunk.x - RenderDistance; x <= CurrentChunk.x + RenderDistance; x++) {
        for (int z = (int) CurrentChunk.z - RenderDistance; z <= CurrentChunk.z + RenderDistance; z++) {
			for (int y = startY; y >= endY; y--) {
                glm::vec3 pos(x, y, z);

				if (pos != CurrentChunk && !ChunkMap.count(pos)) {
                    if (pow(CurrentChunk.x - x, 2) + pow(CurrentChunk.z - z, 2) <= pow(RenderDistance, 2)) {
                        ChunkMap[pos] = new Chunk(pos);
                        ChunkMap[pos]->buffer.Init(shader);
                        ChunkMap[pos]->buffer.Create(3, 2, 1, 1);
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