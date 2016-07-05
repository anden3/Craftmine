#include "Network.h"

#include <enet/enet.h>
#include <cstdio>

#include "Chat.h"
#include "main.h"
#include "Player.h"

#include <json.hpp>

static ENetHost* client;
static ENetPeer* peer;

static std::string ClientName = "";

static const int DEFAULT_PORT = 1234;

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

        if (part.front() == '0') {
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
        printf("Connected to %x:%u.\n\n", event.peer->address.host, event.peer->address.port);

        nlohmann::json j;
        j["events"]["connect"]["name"] = ClientName;
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