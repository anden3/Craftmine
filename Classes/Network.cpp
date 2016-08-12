#include "Network.h"

#include <enet/enet.h>
#include <cstdio>

#include "Chat.h"
#include "main.h"
#include "Chunk.h"
#include "Blocks.h"
#include "Buffer.h"
#include "Camera.h"
#include "Entity.h"
#include "Player.h"
#include "Shader.h"

#include <json.hpp>

static ENetHost* client;
static ENetPeer* peer;

static std::string ClientName = "";

static const int DEFAULT_PORT = 1234;

static const float MOVEMENT_ANGLE_START = -45.0f;
static const float MOVEMENT_ANGLE_END   =  45.0f;

static const float PUNCHING_ANGLE_START = 90.0f;
static const float PUNCHING_ANGLE_END   = 120.0f;

static glm::vec3 LastPos = {0, 0, 0};

struct PlayerChar {
    glm::vec3 Tile     = {0, 0, 0};
    glm::vec3 Chunk    = {0, 0, 0};
    glm::vec3 Position = {0, 0, 0};
    glm::vec3 Velocity = {0, 0, 0};

    glm::vec3 Front;
    glm::vec3 Right;

    glm::vec3 FrontDirection;
    glm::vec3 RightDirection;

    float Pitch = 0.0f;
    float Yaw   = 0.0f;

    float MovementAngle = 0.0f;
    float PunchingAngle = 0.0f;

    float PunchingAngleDirection = 200;
    float MovementAngleDirection = 5000;

    int LightLevel = SUN_LIGHT_LEVEL;

    bool Flying    = false;
    bool Jumping   = false;
    bool OnGround  = true;

    std::map<int, bool> Keys = {};
};

static std::map<std::string, PlayerChar> Players;

void Move_Player(PlayerChar &p);
void Update_Player(PlayerChar &p);
void Check_Player_Pickup(PlayerChar &p);
void Collision_Detection(PlayerChar &p);
void Calculate_View_Vectors(PlayerChar &p);

static bool Check_Col(glm::vec3 pos) {
    glm::vec3 chunk, tile;
    std::tie(chunk, tile) = Get_Chunk_Pos(pos);

    if (!Exists(chunk)) {
        return false;
    }

    int blockID = ChunkMap[chunk]->Get_Type(tile);
    return blockID != 0 && Blocks::Get_Block(blockID)->Collision;
}

void Network::Init() {
    enet_initialize();
    atexit(enet_deinitialize);

    client = enet_host_create(NULL, 1, 2, 0, 0);
}

void Network::Update(unsigned int timeout) {
    ENetEvent event;

    while (enet_host_service(client, &event, timeout) > 0) {
        if (event.type == ENET_EVENT_TYPE_NONE) {
            return;
        }

        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            nlohmann::json j;
            j["event"] = "connect";
            j["name"] = ClientName;
            Send(j.dump());
        }
        else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            std::string data(reinterpret_cast<char*>(event.packet->data));
            nlohmann::json j = nlohmann::json::parse(data);
            std::string eventType = j["event"];

            PlayerChar* p;

            if (j.count("player")) {
                p = &Players[j["player"]];
            }

            if (eventType == "connect") {
                Chat::Write("&a" + j["player"].get<std::string>() + " has connected.");
            }

            else if (eventType == "disconnect") {
                Chat::Write("&c" + j["player"].get<std::string>() + " has disconnected.");
                Players.erase(j["player"].get<std::string>());
            }

            else if (eventType == "key") {
                int key = j["key"];
                int keyState = j["state"];

                p->Keys[key] = (keyState == GLFW_PRESS);

                if (keyState == GLFW_PRESS) {
                    if (key == GLFW_KEY_SPACE && p->OnGround) {
                        p->Jumping = true;
                    }

                    else if (key == GLFW_KEY_F) {
                        p->Flying = !p->Flying;
                    }
                }
            }

            else if (eventType == "look") {
                p->Yaw = j["yaw"];
                p->Pitch = j["pitch"];

                Calculate_View_Vectors(*p);
            }

            else if (eventType == "position") {
                p->Position = {
                    j["pos"][0], j["pos"][1], j["pos"][2]
                };
            }

            else {
                player.Request_Handler(data, false);
            }

            enet_packet_destroy(event.packet);
        }
    }
}

void Network::Update_Players() {
    for (auto &player : Players) {
        Update_Player(player.second);
    }
}

void Network::Render_Players() {
    static Buffer* buffers[6] = {
        &HeadBuffer,     &BodyBuffer,    &LeftArmBuffer,
        &RightArmBuffer, &LeftLegBuffer, &RightLegBuffer
    };

    static glm::vec3 translateOffsets[6] = {
        {0, 1.5, 0}, {0, 0.75, 0}, {0, 1.50, 0},
        {0, 1.5, 0}, {0, 0.75, 0}, {0, 0.75, 0}
    };

    static glm::vec3 scalingFactors[6] = {
        {0.5, 0.5, 0.5}, {0.5, 0.75, 0.25}, {0.25, 0.75, 0.25},
        {0.25, 0.75, 0.25}, {0.25, 0.75, 0.25}, {0.25, 0.75, 0.25}
    };

    for (auto const &player : Players) {
        const PlayerChar &p = player.second;

        if (!Exists(p.Chunk) || !ChunkMap[p.Chunk]->Visible) {
            continue;
        }

        mobShader->Upload("lightLevel", p.LightLevel);

        for (int i = 0; i < 6; i++) {
            glm::mat4 model;
            float angle = p.MovementAngle;

            if (i == 3 && p.Keys.at(GLFW_MOUSE_BUTTON_LEFT)) {
                angle = p.PunchingAngle + p.Pitch;
            }

            angle = glm::radians(angle);

            if (i == 2 || i == 5) {
                angle = -angle;
            }

            model = glm::translate(model, p.Position + translateOffsets[i]);

            if (i >= 2) { // Rotate body parts
                model = glm::rotate(model, angle, p.Right);
            }

            model = glm::rotate(model, glm::radians(270.0f - p.Yaw), {0, 1, 0});

            if (i == 0) { // Rotate head up/down
                model = glm::rotate(model, glm::radians(p.Pitch), {1, 0, 0});
            }

            model = glm::scale(model, scalingFactors[i]);

            mobShader->Upload("model", model);
            buffers[i]->Draw();
        }
    }
}

void Update_Player(PlayerChar &p) {
    static bool FirstUpdate = true;

    glm::vec3 prevPos = p.Position;

    Move_Player(p);
    Check_Player_Pickup(p);

    if (p.Keys.at(GLFW_MOUSE_BUTTON_LEFT)) {
        p.PunchingAngle += static_cast<float>(DeltaTime) * p.PunchingAngleDirection;

        if (p.PunchingAngle >= PUNCHING_ANGLE_END && p.PunchingAngleDirection > 0) {
            p.PunchingAngle = PUNCHING_ANGLE_END;
            p.PunchingAngleDirection *= -1;
        }
        else if (p.PunchingAngle <= PUNCHING_ANGLE_START && p.PunchingAngleDirection < 0) {
            p.PunchingAngle = PUNCHING_ANGLE_START;
            p.PunchingAngleDirection *= -1;
        }
    }
    else {
        p.PunchingAngle = PUNCHING_ANGLE_START;
    }

    if (FirstUpdate || p.Position != prevPos) {
        FirstUpdate = false;

        glm::vec3 prevTile = p.Tile;
        std::tie(p.Chunk, p.Tile) = Get_Chunk_Pos(p.Position);

        if (prevTile != p.Tile && Exists(p.Chunk)) {
            if (p.Position.y >= ChunkMap[p.Chunk]->Get_Top(p.Tile)) {
                p.LightLevel = SUN_LIGHT_LEVEL;
            }
            else {
                p.LightLevel = ChunkMap[p.Chunk]->Get_Light(p.Tile);
            }
        }
    }
}

void Move_Player(PlayerChar &p) {
    p.Velocity.x = 0;
    p.Velocity.z = 0;

    float speed = PLAYER_BASE_SPEED * static_cast<float>(DeltaTime);

    if (p.Keys[GLFW_KEY_LEFT_SHIFT]) {
        speed *= PLAYER_SPRINT_MODIFIER * (p.Flying + 1);
    }

    static int moveKeys[4] = {GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_A};
    static float signs[2]  = {1.0f, -1.0f};

    glm::vec3 flyDirs[2]  = {p.Front, p.Right};
    glm::vec3 walkDirs[2] = {p.FrontDirection, p.RightDirection};

    if (p.Flying) {
        p.MovementAngle = 0.0f;
        p.Velocity = glm::vec3(0);

        for (int i = 0; i < 4; i++) {
            if (p.Keys[moveKeys[i]]) {
                p.Position += flyDirs[i / 2] * speed * signs[i % 2];
            }
        }

        std::tie(p.Chunk, p.Tile) = Get_Chunk_Pos(p.Position);
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (p.Keys[moveKeys[i]]) {
                p.Velocity += walkDirs[i / 2] * speed * signs[i % 2];
            }
        }

        if (p.Jumping) {
            p.Jumping = false;
            p.Velocity.y += JUMP_HEIGHT;
        }

        Collision_Detection(p);

        if (p.Velocity.xz() != glm::vec2(0)) {
            if (p.MovementAngle >= MOVEMENT_ANGLE_END && p.MovementAngleDirection > 0) {
                p.MovementAngleDirection *= -1;
            }
            else if (p.MovementAngle <= MOVEMENT_ANGLE_START && p.MovementAngleDirection < 0) {
                p.MovementAngleDirection *= -1;
            }

            p.MovementAngle += p.MovementAngleDirection * speed * static_cast<float>(DeltaTime);
        }
        else {
            p.MovementAngle = 0.0f;
        }
    }
}

void Collision_Detection(PlayerChar &p) {
    if (!Exists(p.Chunk)) {
        return;
    }

    p.Velocity.y -= GRAVITY;
    p.OnGround = (p.Velocity.y <= 0 && Check_Col(
        {p.Position.x, p.Position.y + p.Velocity.y - 0.01f, p.Position.z}
    ));

    if (p.OnGround) {
        p.Velocity.y = 0;
    }

    else if (p.Velocity.y != 0) {
        glm::vec3 checkPos = p.Position + glm::vec3(0, p.Velocity.y, 0);

        if (p.Velocity.y > 0) {
            checkPos.y += CAMERA_HEIGHT;
        }

        if (!Check_Col(checkPos)) {
            p.Position.y += p.Velocity.y;
        }
        else if (p.Velocity.y > 0) {
            p.Velocity.y = 0;
        }
    }

    glm::vec3 offsets[2] = {
        {p.Velocity.x + PLAYER_WIDTH * std::copysign(1.0f, p.Velocity.x), 0, 0},
        {0, 0, p.Velocity.z + PLAYER_WIDTH * std::copysign(1.0f, p.Velocity.z)}
    };

    for (int i = 0; i < 3; i += 2) {
        if (p.Velocity[i] == 0) {
            continue;
        }

        glm::vec3 checkingPos = p.Position + offsets[i / 2];

        if (Check_Col(checkingPos)) {
            continue;
        }

        checkingPos.y += CAMERA_HEIGHT;

        if (!Check_Col(checkingPos)) {
            p.Position[i] += p.Velocity[i];
        }
    }
}

void Check_Player_Pickup(PlayerChar &p) {
    if (Entities.empty()) {
        return;
    }

    glm::vec3 playerCenter = p.Position + glm::vec3(0, 1, 0);
    std::vector<EntityInstance*>::iterator it = Entities.begin();

    while (it != Entities.end()) {
        if (!(*it)->Can_Move) {
            ++it;
            continue;
        }

        float dist = glm::distance((*it)->Position, playerCenter);

        if (dist < PICKUP_RANGE) {
            player.Play_Sound("pickup", p.Chunk, p.Tile);
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
}

void Calculate_View_Vectors(PlayerChar &p) {
    glm::vec3 front(
        glm::cos(glm::radians(p.Yaw)) * glm::cos(glm::radians(p.Pitch)),
        glm::sin(glm::radians(p.Pitch)),
        glm::sin(glm::radians(p.Yaw)) * glm::cos(glm::radians(p.Pitch))
    );

    p.FrontDirection = glm::normalize(glm::vec3(front.x, 0, front.z));
    p.RightDirection = glm::vec3(-p.FrontDirection.z, 0, p.FrontDirection.x);

    p.Front = glm::normalize(front);
    p.Right = glm::normalize(glm::cross(p.Front, glm::vec3(0, 1, 0)));
}

void Network::Send_Player_Position() {
    nlohmann::json data;
    data["event"] = "position";
    data["pos"] = {
        player.WorldPos.x, player.WorldPos.y, player.WorldPos.z
    };

    Send(data.dump());
}

void Network::Send_Key_Event(int key, int action) {
    nlohmann::json data;
    data["event"] = "key";
    data["key"] = key;
    data["state"] = action;

    Send(data.dump());
}

void Network::Send_Look_Event() {
    nlohmann::json data;
    data["event"] = "look";
    data["yaw"] = Cam.Yaw;
    data["pitch"] = Cam.Pitch;

    Send(data.dump());
}

std::string Network::Connect(std::string name, std::string host) {
    if (name == "") {
        return "&cError! &fPlease input a user name.";
    }

    ClientName = name;

    std::string ip;
    unsigned short port;

    if (host == "") {
        return "&cError! &fPlease input an IP address.";
    }

    if (std::count(host.begin(), host.end(), '.') != 3) {
        return "&cError! &fInvalid IP address.";
    }

    if (host.find(':') == std::string::npos || host.find(':') == host.length() - 1) {
        port = DEFAULT_PORT;

        if (ip.find(':') != std::string::npos) {
            ip = host.substr(0, host.length() - 1);
        }
        else {
            ip = host;
        }
    }
    else {
        try {
            port = static_cast<unsigned short>(std::stoi(host.substr(host.find(':') + 1)));
            ip = host.substr(0, host.find(':'));
        }
        catch (...) {
            return "&cError! &fInvalid port.";
        }
    }

    for (std::string const &part : Split(ip, '.')) {
        if (part == "") {
            return "&cError! &fMissing IP value.";
        }

        if (part.length() > 1 && part.front() == '0') {
            return "&cError! &fPlease remove leading zeroes from IP values.";
        }

        try {
            int partNum = std::stoi(part);

            if (partNum > 255) {
                return "&cError! &fIP value out of range. Value &6" + part + " &fis out of range (&60 &f- &6255&f).";
            }
        }
        catch (const std::invalid_argument) {
            return "&cError! &fNon-numeric characters in IP.";
        }
    }

    ENetEvent event;
    ENetAddress address;

    address.port = port;
    enet_address_set_host(&address, ip.c_str());
    peer = enet_host_connect(client, &address, 2, 0);

    if (enet_host_service(client, &event, 500) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        nlohmann::json j;
        j["event"] = "connect";
        j["name"] = ClientName;
        Send(j.dump());

        return "";
    }

    return "&cError! &fCould not connect to server!";
}

void Network::Disconnect() {
    enet_peer_disconnect(peer, 0);
}

void Network::Send(std::string message, unsigned char channel) {
    ENetPacket* packet = enet_packet_create(message.c_str(), message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, channel, packet);
}