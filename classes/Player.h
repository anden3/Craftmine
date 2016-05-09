#pragma once

#include "Camera.h"
#include "Sound.h"
#include "Chat.h"
#include "Inventory.h"
#include "Entity.h"

enum Directions {
    LEFT,
    RIGHT,
    DOWN,
    UP,
    BACK,
    FRONT
};

extern bool MouseEnabled;
extern bool ChunkMapBusy;

extern int RenderDistance;

extern Chat chat;

std::vector<std::string> Split(const std::string &s, char delim);

class Player {
public:
    glm::vec3 WorldPos = glm::vec3(0.0f);
    glm::vec3 CurrentChunk = glm::vec3(0);
    glm::vec3 CurrentTile = glm::vec3(0);
    
    Inventory inventory;

    bool LookingAtBlock = false;
    
    int CurrentBlock = 0;
    
    glm::vec3 LookingChunk;
    glm::vec3 LookingAirChunk;
    
    glm::vec3 LookingTile;
	glm::vec3 LookingAirTile;
    
    glm::dvec2 LastMousePos = glm::dvec2(0.0, 0.0);

    Camera Cam = Camera();
    
    void Init_Model();
    void Draw_Model();
    
    void Init_Holding();
    void Draw_Holding();

	void PollSounds();
    void Move(float deltaTime, bool update = false);
    void Teleport(glm::vec3 pos);
        
    void RenderChunks();
    
    void Clear_Keys();

    void KeyHandler(int key, int action);
    void MouseHandler(double posX, double posY);
    void ScrollHandler(double offsetY);
    void ClickHandler(int button, int action);

private:
	bool Flying = false;
	bool Jumping = false;
	bool OnGround = false;
	bool MovedMouse = false;
    bool FirstTime = true;
    bool ThirdPerson = false;
    
    unsigned int HoldingVAO, HoldingVBO;
    unsigned int ModelVAO, ModelVBO;
    
    int HoldingVertices = 0;
    int ModelVertices = 0;
    
    int LightLevel = SUN_LIGHT_LEVEL;
    
	float SpeedModifier = 1.0f;

	glm::vec3 Velocity;

	Listener listener;
    
    void Mesh_Holding();
    
	void ColDetection();
	std::vector<glm::vec3> Hitscan();
    
    void Check_Pickup();
    
    void Drop_Item();

	void PlaySound(glm::vec3 chunk, glm::vec3 tile);
    
    void Place_Light(int lightLevel);
    void Remove_Light();
    
    bool Check_Top();
};

extern Player player;

void Process_Light_Queue();
void Process_Light_Removal_Queue();