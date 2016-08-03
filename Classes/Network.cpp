#include "Network.h"

#include <enet/enet.h>
#include <cstdio>

#include "Chat.h"
#include "main.h"
#include "Buffer.h"
#include "Camera.h"
#include "Player.h"
#include "Shader.h"

#include <json.hpp>

static ENetHost* client;
static ENetPeer* peer;

static std::string ClientName = "";

static const int DEFAULT_PORT = 1234;
static const int MAX_PLAYERS  = 16;

static glm::vec3 LastPos = {0, 0, 0};
static float LastPitch = 0.0f;
static float LastYaw = 0.0f;

struct PlayerChar {
    glm::vec3 Position;
    float Pitch;
    float Yaw;
};

static std::map<std::string, PlayerChar> Players;

void Network::Init() {
    enet_initialize();
    atexit(enet_deinitialize);

    client = enet_host_create(NULL, 1, 2, 0, 0);
}

void Network::Update(unsigned int timeout) {
    nlohmann::json message;
    ENetEvent event;

    message["event"] = "update";

    if (player.WorldPos != LastPos) {
        LastPos = player.WorldPos;
        message["pos"] = {
            player.WorldPos.x, player.WorldPos.y, player.WorldPos.z
        };
    }

    if (std::abs(Cam.Pitch - LastPitch) >= 1.0f) {
        LastPitch = Cam.Pitch;
        message["pitch"] = Cam.Pitch;
    }

    if (std::abs(Cam.Yaw - LastYaw) >= 1.0f) {
        LastYaw = Cam.Yaw;
        message["yaw"] = Cam.Yaw;
    }

    if (message.size() > 1) {
        Send(message.dump());
    }

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

            if (j["event"] == "update") {
                if (!Players.count(j["player"])) {
                    Players[j["player"]] = PlayerChar();
                }

                PlayerChar &p = Players[j["player"]];

                if (j.count("pos")) {
                    p.Position = glm::vec3(
                        j["pos"][0], j["pos"][1], j["pos"][2]
                    );
                }

                if (j.count("pitch")) {
                    p.Pitch = j["pitch"];
                }

                if (j.count("yaw")) {
                    p.Yaw = j["yaw"];
                }
            }

            else {
                player.Request_Handler(data, false);
            }

            enet_packet_destroy(event.packet);
        }
    }
}

void Network::Render_Players() {
    Buffer* buffers[6] = {
        &HeadBuffer, &BodyBuffer, &LeftArmBuffer,
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

        for (int i = 0; i < 6; i++) {
            glm::mat4 model;
            model = glm::translate(model, p.Position + translateOffsets[i]);
            model = glm::rotate(model, p.Yaw, {0, 1, 0});

            if (i == 0) { // Rotate head up/down
                model = glm::rotate(model, float(glm::radians(p.Pitch)), {1, 0, 0});
            }

            model = glm::scale(model, scalingFactors[i]);

            mobShader->Upload("model", model);
            buffers[i]->Draw();
        }
    }
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