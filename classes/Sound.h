#pragma once

#ifdef _WIN32
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#elif __APPLE__
#include <OpenAL/OpenAL.h>

#endif

#include <glm/glm.hpp>

#include <vector>
#include <string>

#include <vorbis/vorbisfile.h>

class Listener {
public:
	Listener();

	void Set_Position(glm::vec3 pos);
	void Set_Orientation(glm::vec3 orientation);

	void Delete();

private:
	ALCdevice* Device;
	ALCcontext* Context;

	glm::vec3 Position = glm::vec3(0);
	glm::vec3 Orientation = glm::vec3(0);
};

class Sound {
public:
	std::string Name;

	ALsizei Length;
	unsigned int Buffer = 0;

	Sound(std::string sound);

	void Delete();

private:
	float Duration;

	int Frequency;
	int Format;

	std::vector<char> Load_OGG(std::string path);
};

class SoundPlayer {
public:
	SoundPlayer();

	void Set_Volume(float volume);
	void Set_Pitch(float pitch);
	void Set_Rolloff(float rolloff);

	void Set_Loop(bool loop);

	void Set_Position(glm::vec3 position);

	void Set_Seek(float seek);

	void Add(Sound sound);
	void Remove();

	void Play();
	bool Playing();

	void Stop();
	void Rewind();
	void Pause();

	void Delete();

private:
	unsigned int Source = 0;
	int State = 0;

	float Volume = 1.0f;
	float Pitch = 1.0f;
	float Rolloff = 1.0f;

	bool Loop = false;
	
	glm::vec3 Position = glm::vec3(0);

	std::vector<Sound> Queue;
};