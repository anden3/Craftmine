#pragma once

#include "camera.h"
#include "Chunk.h"
#include "Sound.h"

enum Directions {
    LEFT,
    RIGHT,
    DOWN,
    UP,
    BACK,
    FRONT
};

extern std::map<glm::vec3, Chunk*, Vec3Comparator> ChunkQueue;

extern bool EditingChunkQueue;
extern bool EditingChunkMap;

extern const int RENDER_DISTANCE;

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

    Camera Cam = Camera();

	void PollSounds();
    void Move(float deltaTime);

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

	glm::dvec2 LastMousePos = glm::dvec2(0.0, 0.0);
	glm::vec3 Velocity;

	Listener listener;

	void ColDetection();
	std::vector<glm::vec3> Hitscan();

	void PlaySound(glm::vec3 chunk, glm::vec3 tile);

	void RenderChunks();
};

bool IsBlock(glm::vec3 pos);