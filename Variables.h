#pragma once

// -------------------------------
// Enumerations
enum Directions {
    LEFT,
    RIGHT,
    DOWN,
    UP,
    BACK,
    FRONT
};

// -------------------------------
// Constants
static const int SCREEN_WIDTH  = 1440;
static const int SCREEN_HEIGHT = 900;

static const int AVG_FPS_RANGE         = 5;
static const int FPS_UPDATE_FRAME_FREQ = 5;

static const int CHUNK_SIZE = 16;
static const int CHUNK_ZOOM = 10;

static const float DEFAULT_FOV = 90.0f;
static const float MAX_FOV     = 120.0f;
static const float MIN_FOV     = 1.0f;

static const float PLAYER_BASE_SPEED  = 3.0f;
static const float PLAYER_SENSITIVITY = 0.25f;

static const int TEXT_TEXTURE_UNIT = 10;

bool CONSTRAIN_PITCH = true;

// -------------------------------
// Variables
noise::module::Perlin noiseModule;

// -------------------------------
// Data
GLfloat vertices[36][8] = {
        // Left
        { 0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f},
        { 0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f},

        { 0.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f},
        { 0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 0.0f,  0.0f,  1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f},

        // Right
        { 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f},

        { 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f},
        { 1.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f},

        // Down
        { 0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f},
        { 1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f},

        { 1.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f},
        { 0.0f,  0.0f,  1.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f},
        { 0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f},

        // Up
        { 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f},
        { 1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f},

        { 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f},
        { 0.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f},
        { 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f},

        // Back
        { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f},
        { 1.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f},
        { 1.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f},

        { 0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f},
        { 0.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f,  0.0f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f},

        // Front
        { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f},
        { 1.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f},
        { 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f},

        { 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f},
        { 0.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f},
        { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f}
    };