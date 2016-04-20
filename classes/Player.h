#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "Sound.h"
#include "Chat.h"

enum Directions {
    LEFT,
    RIGHT,
    DOWN,
    UP,
    BACK,
    FRONT
};

extern std::queue<Chunk*> ChunkQueue;
extern std::set<glm::vec3, Vec3Comparator> ChunkSet;

extern bool EditingChunkQueue;
extern bool EditingChunkMap;

extern bool ShowMenu;

extern int RENDER_DISTANCE;

extern Chat chat;

class Player {
public:
    glm::vec3 WorldPos = glm::vec3(0.0f);
    glm::vec3 CurrentChunk = glm::vec3(0);
    glm::vec3 CurrentTile = glm::vec3(0);

    bool LookingAtBlock = false;

    glm::vec3 LookingChunk;
    glm::vec3 LookingTile;

	glm::vec3 LookingAirChunk;
	glm::vec3 LookingAirTile;
    
    glm::dvec2 LastMousePos = glm::dvec2(0.0, 0.0);

    Camera Cam = Camera();

	void PollSounds();
    void Move(float deltaTime);
    
    void Process_Sunlight();
    
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

	float SpeedModifier = 1.0f;
    
    int CurrentBlock = 11;

	glm::vec3 Velocity;

	Listener listener;

	void ColDetection();
	std::vector<glm::vec3> Hitscan();

	void PlaySound(glm::vec3 chunk, glm::vec3 tile);
    
    void Place_Torch();
    void Remove_Torch();
};

void Process_Light_Queue();
void Process_Light_Removal_Queue();

bool IsBlock(glm::vec3 pos);