#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

#include "Buffer.h"

extern Buffer HeadBuffer;
extern Buffer BodyBuffer;
extern Buffer LeftArmBuffer;
extern Buffer RightArmBuffer;
extern Buffer LeftLegBuffer;
extern Buffer RightLegBuffer;

typedef std::vector<float> Data;

struct Block;

std::vector<std::string> Split(const std::string &s, char delim);

class Player {
  public:
    glm::vec3 WorldPos = glm::vec3(0);
    glm::vec3 CurrentTile = glm::vec3(0);
    glm::vec3 CurrentChunk = glm::vec3(0);

    bool Creative = false;
    bool LookingAtBlock = false;

    const Block* LookingBlockType = nullptr;

    glm::vec3 LookingChunk;
    glm::vec3 LookingAirChunk;

    glm::vec3 LookingTile;
    glm::vec3 LookingAirTile;

    glm::ivec2 LastMousePos = glm::ivec2(0, 0);

    void Init();

    void Mesh_Holding();
    void Mesh_Damage(int index);

    void Move();

    void Update(bool update = false);
    void Draw();

    void Play_Sound(std::string type, glm::vec3 chunk, glm::vec3 tile);

    void Teleport(glm::vec3 pos);

    void Queue_Chunks(bool regenerate = false);
    void Load_Data(const std::string data);
    void Clear_Keys();

    void Key_Handler(int key, int action);
    void Mouse_Handler(int posX, int posY);
    void Scroll_Handler(double offsetY);
    void Click_Handler(int button, int action);
    void Request_Handler(std::string packet, bool sending);

  private:
    bool Flying = false;
    bool Jumping = false;
    bool OnGround = false;
    bool MovedMouse = false;
    bool ThirdPerson = false;

    int LightLevel = 15;

    int CurrentBlock;
    int CurrentBlockData = 0;

    const Block* CurrentBlockType = nullptr;

    float Rotation;
    float MouseTimer = 0.0f;

    glm::vec3 Velocity;

    void Init_Model();
    void Init_Sounds();

    void Draw_Model();
    void Draw_Holding();

    void Check_Hit();
    void Col_Detection();

    float Get_Block_Break_Time();
    void Check_Pickup();

    void Drop_Item();
    void Break_Block(glm::vec3 pos, bool external = false);

    void Place_Light(int lightLevel);
    void Remove_Light();

    void Cull_Chunks();
};
