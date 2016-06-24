#include "Network.h"

#include <enet/enet.h>
#include <cstdio>

#include "Chat.h"
#include "main.h"
#include "Player.h"

#include <json.hpp>

static ENetHost* client;
static ENetPeer* peer;

void NetworkClient::Init(std::string name) {
    ClientName = name;

    enet_initialize();
    atexit(enet_deinitialize);

    client = enet_host_create(NULL, 1, 2, 0, 0);
}

void NetworkClient::Update(unsigned int timeout) {
    ENetEvent event;

    while (enet_host_service(client, &event, timeout) > 0) {
        if (event.type == ENET_EVENT_TYPE_NONE) {
            return;
        }

        if (event.type == ENET_EVENT_TYPE_CONNECT) {
            printf("Connected to %x:%u.\n\n", event.peer->address.host, event.peer->address.port);

            nlohmann::json j;
            j["events"]["connect"]["name"] = PLAYER_NAME;
            Send(j.dump());
        }
        else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            player.Request_Handler(
                std::string(reinterpret_cast<char*>(event.packet->data)), false
            );
            enet_packet_destroy(event.packet);
        }
        else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
            puts("Disconnected from server!\n");
        }
    }
}

void NetworkClient::Connect(std::string host, int port) {
    ENetAddress address;

    enet_address_set_host(&address, host.c_str());
    address.port = static_cast<unsigned short>(port);

    peer = enet_host_connect(client, &address, 2, 0);

    ENetEvent event;

    if (enet_host_service(client, &event, 500) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        printf("Connected to %x:%u.\n\n", event.peer->address.host, event.peer->address.port);

        nlohmann::json j;
        j["events"]["connect"]["name"] = PLAYER_NAME;
        Send(j.dump());
    }
    else {
        throw "Server connection failed!";
    }
}

void NetworkClient::Disconnect() {
    enet_peer_disconnect(peer, 0);
}

void NetworkClient::Send(std::string message, int channel) {
    ENetPacket* packet = enet_packet_create(message.c_str(), message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, static_cast<unsigned char>(channel), packet);
}
