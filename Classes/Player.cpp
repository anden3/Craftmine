#include "Player.h"

#include <cmath>
#include <sstream>
#include <dirent.h>

#include "UI.h"
#include "Chat.h"
#include "main.h"
#include "Chunk.h"
#include "Sound.h"
#include "Blocks.h"
#include "Camera.h"
#include "Entity.h"
#include "Shader.h"
#include "Worlds.h"
#include "Network.h"
#include "Interface.h"
#include "Inventory.h"

#include <json.hpp>

const float PLAYER_BASE_SPEED      = 3.0f;
const float PLAYER_SPRINT_MODIFIER = 1.5f;

const double PLAYER_SENSITIVITY = 0.25;
const float PLAYER_RANGE = 5.0f;

const float PLAYER_WIDTH = 0.1f;

const float CAMERA_HEIGHT = 1.7f;

const float HITSCAN_STEP_SIZE = 0.1f;

const float JUMP_HEIGHT = 0.1f;

const float ATTRACT_RANGE = 4.0f;
const float PICKUP_RANGE  = 1.5f;
const float ATTRACT_SPEED = 1.0f;

const int PLAYER_TEXTURE_UNIT = 5;

const float MOVEMENT_ANGLE_START = -45.0f;
const float MOVEMENT_ANGLE_END   =  45.0f;

static float PUNCHING_ANGLE_START = 30.0f;
static float PUNCHING_ANGLE_END   = 60.0f;

static std::set<Chunk*> lightMeshingList;

static glm::vec3 lastChunk(-5);
static bool MouseDown = false;

static int NumKeys[10] = {
    GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
    GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0
};

bool keys[1024] = {0};

static Buffer HoldingBuffer;

static Buffer HeadBuffer;
static Buffer BodyBuffer;
static Buffer LeftArmBuffer;
static Buffer RightArmBuffer;
static Buffer LeftLegBuffer;
static Buffer RightLegBuffer;

static int PunchingAngleDirection = 200;
static int MovementAngleDirection = 5000;

static float PunchingAngle = 0.0f;
static float MovementAngle = 0.0f;

static std::map<std::string, std::vector<Sound>> Sounds;

std::vector<std::string> Split(const std::string &s, const char delim) {
    std::vector<std::string> elements;
    std::stringstream ss(s);
    std::string item;

    while (std::getline(ss, item, delim)) {
        elements.push_back(item);
    }

    return elements;
}

bool Check_Col(glm::vec3 pos) {
    glm::vec3 chunk, tile;
    std::tie(chunk, tile) = Get_Chunk_Pos(pos);

    int blockID = ChunkMap[chunk]->Get_Type(tile);
    return Exists(chunk) && blockID != 0 && Blocks::Get_Block(blockID)->Collision;
}

void Player::Init() {
    LightLevel = SUN_LIGHT_LEVEL;

    glm::vec4 hpDims(50, 50, 200, 20);
    glm::vec3 hpRange(0, 100, 100);

    modelShader->Upload("tex", 0);

    HoldingBuffer.Init(modelShader);
    HoldingBuffer.Create(3, 3);

    Interface::Set_Document("playerUI");
    Interface::Add_Bar("health", "HP", hpDims, hpRange);
    Interface::Set_Document("");

    Init_Model();
    Init_Sounds();
}

void Player::Init_Model() {
    mobShader->Upload("tex", PLAYER_TEXTURE_UNIT);

    glActiveTexture(GL_TEXTURE0 + PLAYER_TEXTURE_UNIT);
    unsigned int texture = std::get<0>(Load_Texture("skin.png"));
    glBindTexture(GL_TEXTURE_2D, texture);

    Buffer* buffers[6] = {
        &HeadBuffer, &BodyBuffer, &LeftArmBuffer,
        &RightArmBuffer, &LeftLegBuffer, &RightLegBuffer
    };
    glm::vec3 offsets[6] = {
        {0.5f, 0.0f, 0.5f}, {0.5f, 0.0f, 0.5f}, {2.0f, 1.0f, 0.5f},
        {-1.0f, 1.0f, 0.5f}, {1.0f, 1.0f, 0.5f}, {0.0f, 1.0f, 0.5f}
    };
    std::string parts[6] = { "head", "body", "arm", "arm", "leg", "leg" };

    for (int buf = 0; buf < 6; ++buf) {
        Data data;
        buffers[buf]->Init(mobShader);

        for (unsigned long i = 0; i < 6; i++) {
            for (unsigned long j = 0; j < 6; j++) {
                Extend(data, vertices[i][j] - offsets[buf]);
                data.push_back(PlayerTexCoords.at(parts[buf])[i]
                    [static_cast<unsigned long>(tex_coords[i][j].x)].x / 64);
                data.push_back(PlayerTexCoords.at(parts[buf])[i]
                    [static_cast<unsigned long>(tex_coords[i][j].y)].y / 32);
            }
        }

        buffers[buf]->Create(3, 2, data);
    }
}

void Player::Init_Sounds() {
    DIR *dir = opendir("sounds");
    struct dirent *ent;

    while ((ent = readdir(dir)) != nullptr) {
        std::string name(ent->d_name);

        if (name.find('_') != std::string::npos) {
            Sounds[name.substr(0, name.find('_'))].push_back(
                Sound(name.substr(0, name.find('.')))
            );
        }
    }

    closedir(dir);
}


void Player::Mesh_Holding() {
    CurrentBlock = Inventory::Get_Info()->Type;
    CurrentBlockData = Inventory::Get_Info()->Data;
    CurrentBlockType = Blocks::Get_Block(CurrentBlock, CurrentBlockData);

    if (CurrentBlock > 0) {
        HoldingBuffer.Upload(Blocks::Mesh(CurrentBlockType, glm::vec3(-0.5f)));
    }
}

void Player::Mesh_Damage(int index) {
    ChunkMap[LookingChunk]->ExtraTextures[LookingTile] = Blocks::Get_Block(255, index + 1)->Texture;
	ChunkMap[LookingChunk]->HasExtraTextures = true;
    ChunkMap[LookingChunk]->Mesh();
}

void Player::Draw_Model() {
    if (!ThirdPerson && !MouseDown) {
        return;
    }

    static glm::vec3 translateOffsets[6] = {
        {0, 1.5, 0}, {0, 0.75, 0}, {0, 1.50, 0},
        {0, 1.5, 0}, {0, 0.75, 0}, {0, 0.75, 0}
    };

    static glm::vec3 scalingFactors[6] = {
        {0.5, 0.5, 0.5}, {0.5, 0.75, 0.25}, {0.25, 0.75, 0.25},
        {0.25, 0.75, 0.25}, {0.25, 0.75, 0.25}, {0.25, 0.75, 0.25}
    };

    static Buffer* buffers[6] = {
        &HeadBuffer, &BodyBuffer, &LeftArmBuffer,
        &RightArmBuffer, &LeftLegBuffer, &RightLegBuffer
    };

    if (!ThirdPerson) { // Only draw right arm if holding mouse down and in first person.
        glm::mat4 model;
        model = glm::translate(model, WorldPos + translateOffsets[3]);
        model = glm::rotate(model, glm::radians(PunchingAngle), Cam.Right);
        model = glm::rotate(model, Rotation, {0, 1, 0});
        model = glm::scale(model, scalingFactors[3]);
        mobShader->Upload("model", model);
        buffers[3]->Draw();
    }
    else {
        for (int i = 0; i < 6; i++) {
            glm::mat4 model;
            model = glm::translate(model, WorldPos + translateOffsets[i]);
            float angle = glm::radians((i == 3 && MouseDown) ? PunchingAngle : MovementAngle)
                * ((i == 2 || i == 5) ? -1 : 1);

            if (i >= 2) { // Rotate body parts
                model = glm::rotate(model, angle, Cam.Right);
            }

            model = glm::rotate(model, Rotation, {0, 1, 0});

            if (i == 0) { // Rotate head up/down
                model = glm::rotate(model, float(glm::radians(Cam.Pitch)), {1, 0, 0});
            }

            model = glm::scale(model, scalingFactors[i]);

            mobShader->Upload("model", model);
            buffers[i]->Draw();
        }
    }

    if (CurrentBlock == 0) {
        return;
    }

    float angle = glm::radians((MouseDown ? PunchingAngle : MovementAngle) - 90);
    glm::vec3 offset = WorldPos + glm::vec3(0, 1.6f + glm::sin(angle), 0)
        + Cam.RightDirection * 0.37f + Cam.FrontDirection * glm::cos(angle);

    glm::mat4 model;
    model = glm::translate(model, offset);
    model = glm::rotate(model, angle, glm::vec3(Cam.Right));
    model = glm::rotate(model, Rotation, {0, 1, 0});
    model = glm::scale(model, glm::vec3(0.2f));

    modelShader->Upload("model", model);
    HoldingBuffer.Draw();
}

void Player::Draw_Holding() {
    if (CurrentBlock == 0) {
        return;
    }

    glm::mat4 model;
    model = glm::translate(model,
        WorldPos + (Cam.Front + Cam.Right) * 0.5f + glm::vec3(0, 1.0, 0)
    );
    model = glm::rotate(model, float(glm::radians(270.0f - Cam.Yaw)), {0, 1, 0});
    model = glm::scale(model, glm::vec3(0.5, 0.5, 0.5));

    modelShader->Upload("model", model);
    HoldingBuffer.Draw();
}

void Player::Move() {
    Velocity.x = 0;
    Velocity.z = 0;

    float speed = PLAYER_BASE_SPEED * static_cast<float>(DeltaTime);

    if (keys[GLFW_KEY_LEFT_SHIFT]) {
        speed *= PLAYER_SPRINT_MODIFIER * (Flying + 1);
    }

    static int moveKeys[4] = {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_A};
    static float signs[2] = {1.0f, -1.0f};

    glm::vec3 flyDirs[2] = {Cam.Front, Cam.Right};
    glm::vec3 walkDirs[2] = {Cam.FrontDirection, Cam.RightDirection};

    if (Flying) {
        MovementAngle = 0.0f;
        Velocity = glm::vec3(0);

        for (int i = 0; i < 4; i++) {
            if (keys[moveKeys[i]]) {
                WorldPos += flyDirs[i / 2] * speed * signs[i % 2];
            }
        }

        std::tie(CurrentChunk, CurrentTile) = Get_Chunk_Pos(WorldPos);
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

        Col_Detection();

        if (Velocity.xz() != glm::vec2(0)) {
            if (MovementAngle >= MOVEMENT_ANGLE_END && MovementAngleDirection > 0) {
                MovementAngleDirection *= -1;
            }
            else if (MovementAngle <= MOVEMENT_ANGLE_START && MovementAngleDirection < 0) {
                MovementAngleDirection *= -1;
            }

            MovementAngle += MovementAngleDirection * speed
                * static_cast<float>(DeltaTime);
        }
        else {
            MovementAngle = 0.0f;
        }
    }
}

void Player::Col_Detection() {
    if (ChunkMap.empty() || !Exists(CurrentChunk)) {
        return;
    }

    Velocity.y -= GRAVITY;
    OnGround = (Velocity.y <= 0 && Check_Col(
        {WorldPos.x, WorldPos.y + Velocity.y - 0.01f, WorldPos.z}
    ));

    if (OnGround) {
        Velocity.y = 0;
    }

    else if (Velocity.y != 0) {
        glm::vec3 checkPos = WorldPos + glm::vec3(0, Velocity.y, 0);
        if (Velocity.y > 0) {
            checkPos.y += CAMERA_HEIGHT;
        }

        if (!Check_Col(checkPos)) {
            WorldPos.y += Velocity.y;
        }
        else if (Velocity.y > 0) {
            Velocity.y = 0;
        }
    }

    glm::vec3 offsets[2] = {
        {Velocity.x + PLAYER_WIDTH * std::copysign(1.0f, Velocity.x), 0, 0},
        {0, 0, Velocity.z + PLAYER_WIDTH * std::copysign(1.0f, Velocity.z)}
    };

    for (int i = 0; i < 3; i += 2) {
        if (Velocity[i] == 0.0f) {
            continue;
        }

        glm::vec3 checkingPos = WorldPos + offsets[i / 2];

        if (Check_Col(checkingPos)) {
            continue;
        }

        checkingPos.y += CAMERA_HEIGHT;

        if (!Check_Col(checkingPos)) {
            WorldPos[i] += Velocity[i];
        }
    }
}

void Player::Update(bool update) {
    static bool FirstUpdate = true;

    if (FirstUpdate) {
        update = true;
        FirstUpdate = false;
    }

    glm::vec3 prevPos = WorldPos;
    Move();

    if (MouseDown) {
        PunchingAngle += static_cast<float>(DeltaTime) * PunchingAngleDirection;

        if (PunchingAngle >= PUNCHING_ANGLE_END && PunchingAngleDirection > 0) {
            PunchingAngle = PUNCHING_ANGLE_END;
            PunchingAngleDirection *= -1;
        }
        else if (PunchingAngle <= PUNCHING_ANGLE_START && PunchingAngleDirection < 0) {
            PunchingAngle = PUNCHING_ANGLE_START;
            PunchingAngleDirection *= -1;
        }
    }
    else {
        PunchingAngle = PUNCHING_ANGLE_START;
    }

    Check_Pickup();

    if (LookingAtBlock && ChunkMap.count(LookingChunk)) {
        Chunk* lookingChunk = ChunkMap[LookingChunk];

        if (MouseDown) {
            if (Creative) {
                Break_Block(Get_World_Pos(LookingChunk, LookingTile));
                MouseTimer = 0.0;

                if (Multiplayer) {
                    Request_Handler("blockBreak", true);
                }
            }
            else {
                float requiredTime = Get_Block_Break_Time();
                int prevDamageIndex = static_cast<int>(std::floor(MouseTimer / requiredTime * 10));
                MouseTimer += static_cast<float>(DeltaTime);

                if (MouseTimer >= requiredTime) {
                    Break_Block(Get_World_Pos(LookingChunk, LookingTile));
    				lookingChunk->ExtraTextures.erase(LookingTile);

    				if (lookingChunk->ExtraTextures.size() == 0) {
    					lookingChunk->HasExtraTextures = false;
    				}

                    MouseTimer = 0.0;
                    update = true;

                    if (Multiplayer) {
                        Request_Handler("blockBreak", true);
                    }
                }
                else {
                    int damageIndex = static_cast<int>(std::floor(MouseTimer / requiredTime * 10));

                    if (damageIndex != prevDamageIndex) {
                        Mesh_Damage(damageIndex);
                    }
                }
            }
        }
        else {
            if (lookingChunk->ExtraTextures.count(LookingTile)) {
    			lookingChunk->ExtraTextures.erase(LookingTile);

    			if (lookingChunk->ExtraTextures.size() == 0) {
    				lookingChunk->HasExtraTextures = false;
    			}

    			lookingChunk->Mesh();
            }
        }
    }

    if (update || WorldPos != prevPos) {
        glm::vec3 prevTile = CurrentTile;
        std::tie(CurrentChunk, CurrentTile) = Get_Chunk_Pos(WorldPos);

        if (prevTile != CurrentTile && Exists(CurrentChunk)) {
            if (WorldPos.y >= ChunkMap[CurrentChunk]->Get_Top(CurrentTile)) {
                LightLevel = SUN_LIGHT_LEVEL;
            }
            else {
                LightLevel = ChunkMap[CurrentChunk]->Get_Light(CurrentTile);
            }

            modelShader->Upload("lightLevel", LightLevel);
            mobShader->Upload("lightLevel", LightLevel);
        }

        // Update camera position
        Cam.Position = WorldPos + glm::vec3(0, CAMERA_HEIGHT, 0);

        if (ThirdPerson) {
            Cam.Position -= Cam.Front * 2.0f;
        }

        // Update audio listener position
        listener.Set_Position(Cam.Position);
        Check_Hit();

        if (update || CurrentChunk != lastChunk) {
            lastChunk = CurrentChunk;
            Queue_Chunks();
        }
    }
}

void Player::Draw() {
    if (!ThirdPerson) {
        glClear(GL_DEPTH_BUFFER_BIT);

        if (!MouseDown && CurrentBlock > 0) {
            Draw_Holding();
        }
    }

    Draw_Model();
    Interface::Draw_Document("playerUI");
}

float Player::Get_Block_Break_Time() {
    float requiredTime = LookingBlockType->Hardness;

    if (requiredTime == 0.0f) {
        return 0.0f;
    }

    if (CurrentBlock != 0 && CurrentBlockType->IsTool) {
        if (LookingBlockType->Material == CurrentBlockType->EffectiveMaterial) {
            if (CurrentBlockType->MiningLevel >= LookingBlockType->RequiredMiningLevel) {
                requiredTime *= 1.5f;
            }
            else {
                requiredTime *= 5.0f;
            }

            requiredTime /= CurrentBlockType->MiningSpeed;
        }
        else {
            requiredTime *= 1.5f * 3.33f;
        }
    }
    else {
        requiredTime *= 1.5f * 3.33f;
    }

    if (!OnGround) {
        requiredTime *= 5.0f;
    }
    return requiredTime;
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
            Inventory::Add_Stack((*it)->Type, (*it)->BlockData, (*it)->Size);
            Mesh_Holding();

            delete *it;
        }
        else if (dist < ATTRACT_RANGE && (*it)->Size > 0) {
            glm::vec3 vector = glm::normalize(
                playerCenter - (*it)->Position) * ATTRACT_SPEED * (ATTRACT_RANGE - dist
            );
            (*it)->Velocity += glm::vec3(vector.x, 0, vector.z);
        }

        it = (dist < PICKUP_RANGE) ? Entities.erase(it) : it + 1;
    }

    if (CurrentBlock == 0 && Inventory::Get_Info()->Type) {
        Mesh_Holding();
    }
}

void Player::Teleport(glm::vec3 pos) {
    Velocity = glm::vec3(0);
    WorldPos = pos;

    Update(true);
    Queue_Chunks();
}

void Player::Drop_Item() {
    if (CurrentBlock == 0) {
        return;
    }

    Inventory::Decrease_Size();
    Entity::Spawn(
        WorldPos + glm::vec3(0, CAMERA_HEIGHT, 0) + Cam.Front,
        CurrentBlock, CurrentBlockData,
        glm::vec3(Cam.Front.x, 0, Cam.Front.z) * 2.0f
    );
    Mesh_Holding();
}

void Player::Place_Light(int lightLevel) {
    ChunkMap[LookingAirChunk]->Set_Light(LookingAirTile, lightLevel);
    ChunkMap[LookingAirChunk]->LightQueue.emplace(LookingAirChunk, LookingAirTile);

    ChunkMap[LookingAirChunk]->Light();
    ChunkMap[LookingAirChunk]->Mesh();
}

void Player::Remove_Light() {
    std::queue<LightNode> lightRemovalQueue;

    ChunkMap[LookingChunk]->LightRemovalQueue.emplace(
        LookingChunk, LookingTile, ChunkMap[LookingChunk]->Get_Light(LookingTile)
    );
    ChunkMap[LookingChunk]->Set_Light(LookingTile, 0);

    ChunkMap[LookingChunk]->Light();
    ChunkMap[LookingChunk]->Mesh();
}

void Player::Check_Hit() {
    glm::vec3 lookChunk, lookTile;
    glm::vec3 airChunk, airTile;
    glm::vec3 lastPos;

    bool started = false;

	glm::vec3 prevTile = glm::vec3(0);

    for (float t = 0; t < PLAYER_RANGE; t += HITSCAN_STEP_SIZE) {
        glm::vec3 checkingPos = Cam.Position + Cam.Front * t;

		glm::vec3 checkChunk, checkTile;
		std::tie(checkChunk, checkTile) = Get_Chunk_Pos(checkingPos);

		if (checkTile == prevTile) {
			continue;
		}

		prevTile = checkTile;

		if (!Is_Block(checkingPos)) {
			lastPos = checkingPos;
			started = true;
			continue;
		}

		lookChunk = checkChunk;
		lookTile = checkTile;

        const Block* blockType = Blocks::Get_Block(
            ChunkMap[lookChunk]->Get_Type(lookTile),
            ChunkMap[lookChunk]->Get_Data(lookTile)
        );

        if (!Blocks::Get_Block(ChunkMap[lookChunk]->Get_Type(lookTile))->Targetable) {
            lastPos = checkingPos;
            started = true;
            continue;
        }

        glm::vec3 checkingFrac = glm::fract(checkingPos);

        if (glm::lessThan(checkingFrac, blockType->ScaleOffset) != glm::bvec3(false)) {
            lastPos = checkingPos;
            started = true;
            continue;
        }

        if (glm::lessThanEqual(checkingFrac, blockType->Scale + blockType->ScaleOffset) != glm::bvec3(true)) {
            lastPos = checkingPos;
            started = true;
            continue;
        }

        if (!started) {
            break;
        }

        std::tie(airChunk, airTile) = Get_Chunk_Pos(lastPos);
        LookingAtBlock = true;

        glm::vec3 prevChunk = LookingChunk;
        glm::vec3 prevTile  = LookingTile;

        LookingChunk    = lookChunk;
        LookingTile     = lookTile;
        LookingAirChunk = airChunk;
        LookingAirTile  = airTile;

		if (LookingTile == prevTile) {
			return;
		}

        MouseTimer = 0.0f;

        if (Exists(prevChunk) && ChunkMap[prevChunk]->ExtraTextures.count(prevTile)) {
            ChunkMap[prevChunk]->ExtraTextures.erase(prevTile);

			if (ChunkMap[prevChunk]->ExtraTextures.size() == 0) {
				ChunkMap[prevChunk]->HasExtraTextures = false;
			}

            ChunkMap[prevChunk]->Mesh();
        }

        LookingBlockType = Blocks::Get_Block(
            ChunkMap[LookingChunk]->Get_Type(LookingTile),
            ChunkMap[LookingChunk]->Get_Data(LookingTile)
        );

        return;
    }

    LookingAtBlock = false;
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
            Inventory::Is_Open = !Inventory::Is_Open;

            if (Inventory::Is_Open) {
                Inventory::Mouse_Handler();
            }

            Mesh_Holding();
        }

        else if (key == GLFW_KEY_Q) {
            Drop_Item();
        }

        else if (key == GLFW_KEY_V) {
            ThirdPerson = !ThirdPerson;
            Update(true);
        }

        // Pressing a number key
        if (!Inventory::Is_Open) {
            for (int i = 0; i < 10; i++) {
                if (key == NumKeys[i]) {
                    Inventory::Switch_Slot(i);
                    Mesh_Holding();
                    break;
                }
            }
        }
    }

    if (key >= 0 && key < 1024) {
        keys[key] = (action == GLFW_PRESS) ?
            true :
            ((action == GLFW_RELEASE) ?
                false : keys[key]);
    }
}

void Player::Mouse_Handler(double posX, double posY) {
    if (!MovedMouse) {
        LastMousePos = glm::dvec2(posX, posY);
        MovedMouse = true;
    }

    if (MouseEnabled) {
        if (Inventory::Is_Open) {
            Inventory::Mouse_Handler(posX, posY);
        }

        LastMousePos = glm::dvec2(posX, posY);
        return;
    }

    Cam.Yaw   += static_cast<float>((posX - LastMousePos.x) * PLAYER_SENSITIVITY);
    Cam.Pitch += static_cast<float>((LastMousePos.y - posY) * PLAYER_SENSITIVITY);

    if (Cam.Pitch > 89.9f) {
        Cam.Pitch = 89.9f;
    }

    else if (Cam.Pitch < -89.9f) {
        Cam.Pitch = -89.9f;
    }

    if (Cam.Yaw > 360.0f) {
        Cam.Yaw -= 360.0f;
    }

    else if (Cam.Yaw < 0.0f) {
        Cam.Yaw += 360.0f;
    }

    LastMousePos = glm::dvec2(posX, posY);
    Cam.UpdateCameraVectors();

    if (ThirdPerson) {
        Cam.Position = glm::vec3(
            WorldPos.x, WorldPos.y + CAMERA_HEIGHT, WorldPos.z
        ) - Cam.Front * 2.0f;
    }

    Rotation = glm::radians(270.0f - Cam.Yaw);

    PUNCHING_ANGLE_START = Cam.Pitch + 90.0f;
    PUNCHING_ANGLE_END   = Cam.Pitch + 120.0f;

    listener.Set_Orientation(Cam.Front, Cam.Up);

    Check_Hit();
    Cull_Chunks();
}

void Player::Scroll_Handler(double offsetY) {
    if (std::abs(offsetY) <= 0.2) {
        return;
    }

    if (Chat::Focused && !Chat::FocusToggled) {
        Chat::Scroll(int(std::copysign(1, offsetY)));
    }
    else {
        int slot = Inventory::ActiveToolbarSlot + int(std::copysign(1, offsetY));

        if (slot > 9) {
            slot = 0;
        }

        else if (slot < 0) {
            slot = 9;
        }

        Inventory::Switch_Slot(slot);
        Mesh_Holding();
    }
}

void Player::Click_Handler(int button, int action) {
    if (MouseEnabled) {
        return;
    }

    MouseDown = (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT);

    if (!MouseDown || Creative) {
        MouseTimer = 0.0;
    }

    if (MouseDown && LookingAtBlock && Creative) {
        Break_Block(Get_World_Pos(LookingChunk, LookingTile));

        if (Multiplayer) {
            Request_Handler("blockBreak", true);
        }
    }

    if (action != GLFW_PRESS || button != GLFW_MOUSE_BUTTON_RIGHT ||!LookingAtBlock) {
        return;
    }

    if (LookingBlockType->Interactable) {
        LookingBlockType->RightClickFunction();
        return;
    }

    if (!CurrentBlock || !CurrentBlockType->Placeable) {
        return;
    }

    if (!Exists(LookingAirChunk)) {
        return;
    }

    ChunkMap[LookingAirChunk]->Add_Block(
        LookingAirTile, CurrentBlock, CurrentBlockData
    );

    Inventory::Decrease_Size();

    CurrentBlock = Inventory::Get_Info()->Type;
    CurrentBlockData = Inventory::Get_Info()->Data;
    CurrentBlockType = Blocks::Get_Block(CurrentBlock, CurrentBlockData);

    if (CurrentBlockType->Luminosity > 0) {
        Place_Light(CurrentBlockType->Luminosity);
    }
}

void Player::Break_Block(glm::vec3 pos) {
    glm::vec3 chunk, tile;
    std::tie(chunk, tile) = Get_Chunk_Pos(pos);
    const Block* block = Blocks::Get_Block(
        ChunkMap[chunk]->Get_Type(tile), ChunkMap[chunk]->Get_Data(tile)
    );
    int blockType = block->ID;

    if (blockType == 50) {
        Remove_Light();
    }

    if (block->Sound != "") {
        Play_Sound(block->Sound, chunk, tile);
    }

    if (blockType == 1) {
        blockType = 4;
    }

    else if (blockType == 2) {
        blockType = 3;
    }

    ChunkMap[chunk]->Remove_Block(tile);

    if (block->MultiBlock) {
        Entity::Spawn(pos, blockType);
    }
    else {
        Entity::Spawn(pos, blockType, block->Data);
    }

    Check_Hit();
}

void Player::Play_Sound(std::string type, glm::vec3 chunk, glm::vec3 tile) {
    // Get random sound.
    std::vector<Sound>::iterator sound = Sounds[type].begin();
    std::advance(sound, std::rand() % static_cast<int>(Sounds[type].size()));
    listener.Add_Sound((*sound), Get_World_Pos(chunk, tile));
}

void Player::Cull_Chunks() {
    for (auto const &chunk : ChunkMap) {
        if (glm::distance(CurrentChunk.xz(), chunk.first.xz()) <= 2) {
            chunk.second->Visible = true;
        }
        else {
            chunk.second->Visible = glm::degrees(
                glm::acos(glm::dot(
                    Cam.FrontDirection.xz(),
                    glm::normalize(chunk.first.xz() - CurrentChunk.xz())
                ))
            ) <= DEFAULT_FOV;
        }
    }
}

void Player::Queue_Chunks(bool regenerate) {
    float startY = 3;
    float endY = -10;

    if (player.CurrentChunk.y <= -6) {
        startY = player.CurrentChunk.y + 3;
        endY = player.CurrentChunk.y - 3;
    }

	while (ChunkMapBusy.test_and_set(std::memory_order_acquire)) {
		;
	}

    for (auto chunk = ChunkMap.begin(); chunk != ChunkMap.end();) {
        float dist = glm::distance(CurrentChunk.xz(), chunk->first.xz());
        bool outOfRange = dist >= RENDER_DISTANCE || chunk->first.y > startY || chunk->first.y < endY;

        if (chunk->second == nullptr || regenerate || outOfRange) {
            delete chunk->second;
            chunk = ChunkMap.erase(chunk);
        }
        else {
            ++chunk;
        }
    }

    for (float x = CurrentChunk.x - RENDER_DISTANCE; x <= CurrentChunk.x + RENDER_DISTANCE; x++) {
        for (float z = CurrentChunk.z - RENDER_DISTANCE; z <= CurrentChunk.z + RENDER_DISTANCE; z++) {
            for (float y = startY; y >= endY; y--) {
                glm::vec3 pos(x, y, z);

				if (ChunkMap.count(pos)) {
					continue;
				}

				if (glm::distance(CurrentChunk.xz(), pos.xz()) >= RENDER_DISTANCE) {
					continue;
				}

                auto savedData = Worlds::Load_Chunk(WORLD_NAME, pos);

                if (savedData.size() > 0) {
                    ChangedBlocks[pos] = savedData;
                }
                else {
                    ChangedBlocks.erase(pos);
                }

                ChunkMap[pos] = new Chunk(pos);
                ChunkMap[pos]->buffer.Init(shader);
                ChunkMap[pos]->buffer.Create(3, 3, 1, 1, 1);
            }
        }
    }

	ChunkMapBusy.clear(std::memory_order_release);
}

void Player::Request_Handler(std::string packet, bool sending) {
    nlohmann::json data;

    if (sending) {
        if (packet == "blockBreak") {
            data["event"] = "blockBreak";
            data["player"] = PLAYER_NAME;
            data["pos"] = Network::Format_Vector(Get_World_Pos(LookingChunk, LookingTile));
        }

        Network::Send(data.dump());
    }
    else {
        data = nlohmann::json::parse(packet);

        if (data["event"] == "blockBreak") {
            if (data["player"] == PLAYER_NAME) {
                return;
            }

            std::vector<std::string> coords = Split(data["pos"], ',');
            glm::vec3 pos, chunk, tile;

            for (unsigned long i = 0; i < 3; i++) {
                pos[static_cast<int>(i)] = std::stof(coords[i]);
            }

            std::tie(chunk, tile) = Get_Chunk_Pos(pos);

            if (Exists(chunk)) {
                Break_Block(pos);
            }
            else {
                ChangedBlocks[chunk][tile] = std::make_pair(0, 0);
                Worlds::Save_Chunk(WORLD_NAME, chunk);
            }
        }

        else if (data["event"] == "message") {
            if (data["player"] == PLAYER_NAME) {
                return;
            }

            Chat::Write(data["player"].get<std::string>() + ": " + data["message"].get<std::string>());
        }

        else if (data["event"] == "config") {
            WORLD_NAME = data["world"].get<std::string>();
            Worlds::Load_World(data["seed"]);

            UI::ShowWorlds = false;
            UI::ShowTitle = false;
            GamePaused = false;

            UI::Toggle_Mouse(false);
        }

        else if (data["event"] == "load") {
            Load_Data(packet);
        }
    }
}

void Player::Clear_Keys() {
    std::fill_n(keys, 1024, false);
}

void Player::Load_Data(const std::string data) {
    Inventory::Clear();
    ChunkMap.clear();

    nlohmann::json playerData = nlohmann::json::parse(data);

    Cam.Yaw   = playerData["Yaw"];
    Cam.Pitch = playerData["Pitch"];
    Cam.UpdateCameraVectors();

	WorldPos = glm::vec3(
		playerData["Position"][0], playerData["Position"][1], playerData["Position"][2]
	);
	Velocity = glm::vec3(0);

	std::tie(CurrentChunk, CurrentTile) = Get_Chunk_Pos(WorldPos);
	LookingChunk = CurrentChunk;
	LookingTile = CurrentTile;

    if (playerData.count("Storage")) {
        if (playerData["Storage"].count("Inventory")) {
            Inventory::Load(playerData["Storage"]["Inventory"]);
        }

        if (playerData["Storage"].count("Crafting")) {
            Inventory::Load(playerData["Storage"]["Crafting"]);
        }
    }
}