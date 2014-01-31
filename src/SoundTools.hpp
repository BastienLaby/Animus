#ifndef _HPP_ANIMUS_SOUNDTOOLS_HPP_
#define _HPP_ANIMUS_SOUNDTOOLS_HPP_

#include <fmod.h>

struct Sound {
    FMOD_SOUND*		sound;
    FMOD_CHANNEL*	channel;
    unsigned int 	id;
};

class SoundManager {

public:


	SoundManager() {
		FMOD_System_Create(&system);
    	FMOD_System_Init(system, 1, FMOD_INIT_NORMAL, NULL);
    	idOffset = 0;
	}

	~SoundManager() {
		for(size_t t = 0; t < soundList.size(); t++)
		{
			FMOD_Sound_Release(soundList[t].sound);
		}
    	FMOD_System_Close(system);
    	FMOD_System_Release(system);
	}

	unsigned int createSound(const char* path) {
		Sound sound;
		sound.id = idOffset;
		idOffset++;
		FMOD_RESULT result = FMOD_System_CreateSound(system, path, FMOD_SOFTWARE | FMOD_2D | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL, 0, &sound.sound);
		if (result != FMOD_OK)
	    {
	        fprintf(stderr, "Fail to load %s \n", path);
	        exit(EXIT_FAILURE);
	    }
	    FMOD_System_GetChannel(system, 0, &sound.channel);
	    soundList.push_back(sound);
	    return idOffset-1;
	}

	void playSound(unsigned int idSound) {
		FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, soundList[idSound].sound, 0, NULL);
	}

	void getSpectrum(unsigned int idSound, float* spectrum, unsigned int spectrumSize /* must be a power of 2 */) {
		if(	!(!(spectrumSize == 0) && !(spectrumSize & (spectrumSize - 1)))	|| spectrumSize == 0)
		{
			fprintf(stderr, "Error when getting Spectrum : the spectrum size must be a power of 2. \n");
		}
		else
		{
			float left[spectrumSize];
			float right[spectrumSize];
			FMOD_Channel_GetSpectrum(soundList[idSound].channel, left, spectrumSize, 0,  FMOD_DSP_FFT_WINDOW_RECT);
			FMOD_Channel_GetSpectrum(soundList[idSound].channel, right, spectrumSize, 1,  FMOD_DSP_FFT_WINDOW_RECT);
			for(size_t i = 0; i < spectrumSize; i++)
			{
				spectrum[i] = (left[i] + right[i])/2;
			}
		}
	}

private:
	FMOD_SYSTEM* system;
	std::vector<Sound> soundList;
	unsigned int idOffset;
};

#endif