#pragma once

#include <map>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>

typedef std::vector<float> Data;

enum Directions {LEFT, RIGHT, DOWN, UP, BACK, FRONT};

extern bool MouseEnabled;
extern bool ChunkMapBusy;
extern int RenderDistance;

extern const int SUN_LIGHT_LEVEL;

class Chat;
class Camera;
class Shader;
class Listener;
class Inventory;

extern Chat chat;
extern Camera Cam;
extern Shader* shader;
extern Listener listener;
extern Inventory inventory;

extern std::map<std::string, std::vector<unsigned int>> BlockSounds;
extern std::map<std::string, std::vector<std::vector<glm::vec2>>> PlayerTexCoords;

std::vector<std::string> Split(const std::string &s, char delim);
Data Create_Textured_Cube(const int type, glm::vec3 offset = glm::vec3(-0.5));

class Player {
public:
    glm::vec3 WorldPos = glm::vec3(0);
    glm::vec3 CurrentTile = glm::vec3(0);
    glm::vec3 CurrentChunk = glm::vec3(0);

    bool Creative = false;
    bool LookingAtBlock = false;
    
    glm::vec3 LookingChunk;
    glm::vec3 LookingAirChunk;
    
    glm::vec3 LookingTile;
	glm::vec3 LookingAirTile;
    
    glm::dvec2 LastMousePos = glm::dvec2(0.0, 0.0);
    
    void Init();
    
    void Mesh_Holding();
    void Mesh_Damage(int index);

	void Poll_Sounds();
    
    void Move(float deltaTime, bool update = false);
    void Draw();
    
    void Teleport(glm::vec3 pos);
        
    void Render_Chunks();
    void Clear_Keys();

    void Key_Handler(int key, int action);
    void Mouse_Handler(double posX, double posY);
    void Scroll_Handler(double offsetY);
    void Click_Handler(int button, int action);

private:
	bool Flying = false;
	bool Jumping = false;
	bool OnGround = false;
    bool FirstTime = true;
	bool MovedMouse = false;
    bool ThirdPerson = false;
    
    int LightLevel = SUN_LIGHT_LEVEL;
    int CurrentBlock = 0;
    
	float SpeedModifier = 1.0f;
    float Rotation;
    
    double MouseTimer = 0.0;

    glm::vec3 Velocity;
    
    void Init_Model();
    void Init_Sounds();
    
    void Draw_Model();
    void Draw_Holding();
    void Draw_Damage();
    
	void ColDetection();
	std::vector<glm::vec3> Hitscan();
    
    void Check_Pickup();
    
    void Drop_Item();
    void Break_Block();

    void Play_Sound(std::string type, glm::vec3 chunk, glm::vec3 tile);
    
    void Place_Light(int lightLevel);
    void Remove_Light();
};

extern Player player;