#include "Sound.h"

#include <iostream>

const int MAX_SOUND_BUFFER_SIZE = 10240;
static char SOUND_BUFFER[MAX_SOUND_BUFFER_SIZE]; // 10 KiB

Listener::Listener() {
    Device = alcOpenDevice(NULL);
    Context = alcCreateContext(Device, NULL);

    alcMakeContextCurrent(Context);
}

void Listener::Add_Sound(Sound &sound, glm::vec3 pos) {
    SoundPlayer soundPlayer;

    soundPlayer.Set_Position(pos);
    soundPlayer.Set_Volume(GlobalVolume);

    soundPlayer.Add(sound);
    soundPlayer.Play();

    soundPlayers.push_back(soundPlayer);
}

void Listener::Poll_Sounds() {
    if (soundPlayers.size() == 0) {
        return;
    }

    std::vector<SoundPlayer>::iterator sound = soundPlayers.begin();

    while (sound != soundPlayers.end()) {
        sound = sound->Playing() ? sound + 1 : soundPlayers.erase(sound);
    }
}

void Listener::Set_Position(glm::vec3 pos) {
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
}

void Listener::Set_Orientation(glm::vec3 frontVector, glm::vec3 upVector) {
    float vec[6] = {
        frontVector.x, frontVector.y, frontVector.z,
        upVector.x, upVector.y, upVector.z
    };

    alListenerfv(AL_ORIENTATION, vec);
}

void Listener::Delete() {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(Context);
    alcCloseDevice(Device);
}


Sound::Sound(std::string sound) {
    Name = sound;

    std::string path = "Sounds/" + sound + ".ogg";
    std::vector<char> data = Load_OGG(path);

    alGenBuffers(1, &Buffer);
    alBufferData(Buffer, Format, &data[0], Length, Frequency);
}

std::vector<char> Sound::Load_OGG(std::string path) {
	FILE* file;
	int err = fopen_s(&file, path.c_str(), "rb");

    if (!file || err) {
        std::cout << "ERROR::SOUND::FILE_NOT_FOUND\n" << path << std::endl;
    }

    OggVorbis_File oggFile;
    ov_open(file, &oggFile, NULL, 0);

    vorbis_info* pInfo = ov_info(&oggFile, -1);

    if (pInfo == NULL) {
        return std::vector<char> {'0'};
    }

    Format = (pInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    int bitStream;
    long bytes;

    std::vector<char> buffer;

    do {
        bytes = ov_read(&oggFile, SOUND_BUFFER, MAX_SOUND_BUFFER_SIZE, 0, 2, 1, &bitStream);
        buffer.insert(buffer.end(), SOUND_BUFFER, SOUND_BUFFER + bytes);
    } while (bytes > 0);

    Frequency = int(pInfo->rate);
    Length = ALsizei(buffer.size());
    Duration = float(Length) / Frequency / 2;

    ov_clear(&oggFile);
    fclose(file);

	std::fill(SOUND_BUFFER, SOUND_BUFFER + MAX_SOUND_BUFFER_SIZE, 0);

    return buffer;
}

void Sound::Delete() {
    alDeleteBuffers(1, &Buffer);
}


SoundPlayer::SoundPlayer() {
    alGenSources(1, &Source);

    alSourcef(Source, AL_ROLLOFF_FACTOR, 0);
    alSourcei(Source, AL_SOURCE_RELATIVE, 0);
}

void SoundPlayer::Set_Volume(float volume) {
    Volume = volume;
    alSourcef(Source, AL_GAIN, volume);
}

void SoundPlayer::Set_Pitch(float pitch) {
    Pitch = pitch;
    alSourcef(Source, AL_PITCH, pitch);
}

void SoundPlayer::Set_Rolloff(float rolloff) {
    Rolloff = rolloff;
    alSourcef(Source, AL_ROLLOFF_FACTOR, rolloff);
}

void SoundPlayer::Set_Loop(bool loop) {
    Loop = loop;
    alSourcei(Source, AL_LOOPING, loop);
}

void SoundPlayer::Set_Position(glm::vec3 pos) {
    alSource3f(Source, AL_POSITION, pos.x, pos.y, pos.z);
}

void SoundPlayer::Set_Seek(float seek) {
    alSourcei(Source, AL_BYTE_OFFSET, static_cast<int>(Queue.front().Length * seek));
}

void SoundPlayer::Add(Sound sound) {
    alSourceQueueBuffers(Source, 1, &sound.Buffer);
    Queue.push_back(sound);
}

void SoundPlayer::Remove() {
    if (Queue.size() > 0) {
        alSourceUnqueueBuffers(Source, 1, &Queue.front().Buffer);
        alSourcei(Source, AL_BUFFER, 0);
        Queue.erase(Queue.begin());
    }
}

void SoundPlayer::Play() {
    alSourcePlay(Source);
}

bool SoundPlayer::Playing() {
    alGetSourcei(Source, AL_SOURCE_STATE, &State);
    return State == AL_PLAYING;
}

void SoundPlayer::Stop() {
    alSourceStop(Source);
}

void SoundPlayer::Rewind() {
    alSourceRewind(Source);
}

void SoundPlayer::Pause() {
    alSourcePause(Source);
}

void SoundPlayer::Delete() {
    if (Playing()) {
        Stop();
    }

    alDeleteSources(1, &Source);
}
