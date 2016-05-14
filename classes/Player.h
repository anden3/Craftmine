#pragma once

#include "Camera.h"
#include "Sound.h"
#include "Chat.h"
#include "Inventory.h"
#include "Entity.h"

#include "Buffer.h"

enum Directions {LEFT, RIGHT, DOWN, UP, BACK, FRONT};

extern bool MouseEnabled;
extern bool ChunkMapBusy;

extern int RenderDistance;

extern Chat chat;
extern Shader* shader;

extern std::map<std::string, std::vector<unsigned int>> BlockSounds;

std::vector<std::string> Split(const std::string &s, char delim);

void Extend(Data &storage, const Data input);

void Init_3D_Textured(unsigned int &vao, unsigned int &vbo);
Data Create_Textured_Cube(const int type, glm::vec3 offset = glm::vec3(-0.5));

void Upload_Data(const unsigned int vbo, const Data &data);
void Draw_Cube(unsigned const int vao, const glm::mat4 model, int vertices = 36);

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
    
    void Init();
    
    void Mesh_Holding();
    void Mesh_Damage(int index);
    
    void Draw_Model();
    void Draw_Holding();
    void Draw_Damage();

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
    
    Buffer HoldingBuffer;
    Buffer ModelBuffer;
    Buffer DamageBuffer;
    
    int LightLevel = SUN_LIGHT_LEVEL;
    
	float SpeedModifier = 1.0f;
    
    double MouseTimer = 0.0;

	glm::vec3 Velocity;

	Listener listener;
    
    void Init_Model();
    void Init_Holding();
    void Init_Damage();
    
    void Init_Sounds();
    
	void ColDetection();
	std::vector<glm::vec3> Hitscan();
    
    void Check_Pickup();
    
    void Drop_Item();
    void Break_Block();

    void Play_Sound(std::string type, glm::vec3 chunk, glm::vec3 tile);
    
    void Place_Light(int lightLevel);
    void Remove_Light();
    
    bool Check_Top();
};

extern Player player;

void Process_Light_Queue();
void Process_Light_Removal_Queue();