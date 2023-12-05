#define _USE_MATH_DEFINES
#include <math.h>
#include <al.h>
#include <alc.h>
#include "DynamicSound.h"

ALCdevice *device;
ALCcontext *context;
ALshort waveData[48000];
ALuint buffer;
ALuint source;

int dataLength = 48000;
double inputPhase = 0;
double phase_sine = 0;

int SamplingFrequency;

void SetInitialWaveData()
{
	for (int i = 0; i < dataLength; i++)
	{
		waveData[i] = 0;
	}
}

void DS_Sine(double amplitude, double frequency)
{
	if (amplitude > 1)
		amplitude = 1;
	else if (amplitude < 0)
		amplitude = 0;

	double phaseDelta = 2 * M_PI / (SamplingFrequency / frequency);
	inputPhase = phase_sine;

	for (int i = 0; i < dataLength; i++)
	{
		waveData[i] += 32767 * amplitude * sin(2 * M_PI * frequency * i / SamplingFrequency + inputPhase);
		phase_sine += phaseDelta;
		if (phase_sine >= 2 * M_PI)
			phase_sine -= (2 * M_PI);
	}
}

void DS_Rect(double amplitude, double frequency)
{
	if (amplitude > 1)
		amplitude = 1;
	else if (amplitude < 0)
		amplitude = 0;

	for (int i = 0; i < dataLength; i++)
	{
		if (sin(2 * M_PI * frequency * i / SamplingFrequency) > 0)
			waveData[i] += 32767 * amplitude;
		else
			waveData[i] -= 32767 * amplitude;
	}
}

void DS_Triangle(double amplitude, double frequency)
{
	if (amplitude > 1)
		amplitude = 1;
	else if (amplitude < 0)
		amplitude = 0;

	double currentSample = 0;
	double samplePerPeriod = SamplingFrequency / frequency;
	double gradient = 32767 * 4 * frequency * amplitude / SamplingFrequency;

	for (int i = 0; i < dataLength; i++)
	{
		if (currentSample > samplePerPeriod)
			currentSample -= samplePerPeriod;

		if (currentSample < samplePerPeriod / 4)
			waveData[i] += gradient * currentSample;
		else if (currentSample < samplePerPeriod / 2)
			waveData[i] += 32767 * amplitude - gradient * (currentSample - (samplePerPeriod / 4));
		else if (currentSample < samplePerPeriod * 3 / 4)
			waveData[i] += -gradient * (currentSample - (samplePerPeriod / 2));
		else
			waveData[i] += -32767 * amplitude + gradient * (currentSample - (samplePerPeriod * 3 / 4));

		currentSample++;
	}
}

void DS_Saw(double amplitude, double frequency)
{
	if (amplitude > 1)
		amplitude = 1;
	else if (amplitude < 0)
		amplitude = 0;

	double currentSample = 0;
	double samplePerPeriod = SamplingFrequency / frequency;
	double gradient = 32767 * 2 * frequency * amplitude / SamplingFrequency;

	for (int i = 0; i < dataLength; i++)
	{
		if (currentSample > samplePerPeriod)
			currentSample -= samplePerPeriod;

		if (currentSample < samplePerPeriod / 2)
			waveData[i] += gradient * currentSample;
		else
			waveData[i] += -32767 * amplitude + gradient * (currentSample - (samplePerPeriod / 2));

		currentSample++;
	}
}

void DS_SineADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime)
{
	if (attackAmplitude > 1)
		attackAmplitude = 1;
	else if (attackAmplitude < 0)
		attackAmplitude = 0;

	if (sustainAmplitude > 1)
		sustainAmplitude = 1;
	else if (sustainAmplitude < 0)
		sustainAmplitude = 0;

	int attackTimeInSample = (int)(attackTime * SamplingFrequency);
	int decayTimeInSample = (int)(decayTime * SamplingFrequency);
	int sustainTimeInSample = (int)(sustainTime * SamplingFrequency);
	int releaseTimeInSample = (int)(releaseTime * SamplingFrequency);

	double attackGradient = attackAmplitude / attackTimeInSample;
	double decayGradient = (sustainAmplitude - attackAmplitude) / decayTimeInSample;
	double releaseGradient = -sustainAmplitude / releaseTimeInSample;

	int dataLength = attackTimeInSample + decayTimeInSample + sustainTimeInSample + releaseTimeInSample;
	ALshort data[dataLength];

	for (int i = 0; i < dataLength; i++)
	{
		if (i < attackTimeInSample)
			data[i] = 32767 * attackGradient * i * sin(2 * M_PI * frequency * i / SamplingFrequency);
		else if (i < attackTimeInSample + decayTimeInSample)
			data[i] = 32767 * (attackAmplitude + decayGradient * (i - attackTimeInSample)) * sin(2 * M_PI * frequency * i / SamplingFrequency);
		else if (i < attackTimeInSample + decayTimeInSample + sustainTimeInSample)
			data[i] = 32767 * sustainAmplitude * sin(2 * M_PI * frequency * i / SamplingFrequency);
		else
			data[i] = 32767 * (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample)) * sin(2 * M_PI * frequency * i / SamplingFrequency);
	}

	ALuint _source;
	ALuint _buffer;
	alGenSources(1, &_source);
	alGenBuffers(1, &_buffer);
	alBufferData(_buffer, AL_FORMAT_MONO16, data, sizeof(data), SamplingFrequency);
	alSourcei(_source, AL_BUFFER, _buffer);
	alSourcePlay(_source);
}

void DS_RectADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime)
{
	if (attackAmplitude > 1)
		attackAmplitude = 1;
	else if (attackAmplitude < 0)
		attackAmplitude = 0;

	if (sustainAmplitude > 1)
		sustainAmplitude = 1;
	else if (sustainAmplitude < 0)
		sustainAmplitude = 0;

	int attackTimeInSample = (int)(attackTime * SamplingFrequency);
	int decayTimeInSample = (int)(decayTime * SamplingFrequency);
	int sustainTimeInSample = (int)(sustainTime * SamplingFrequency);
	int releaseTimeInSample = (int)(releaseTime * SamplingFrequency);

	double attackGradient = attackAmplitude / attackTimeInSample;
	double decayGradient = (sustainAmplitude - attackAmplitude) / decayTimeInSample;
	double releaseGradient = -sustainAmplitude / releaseTimeInSample;

	int dataLength = attackTimeInSample + decayTimeInSample + sustainTimeInSample + releaseTimeInSample;
	ALshort data[dataLength];

	for (int i = 0; i < dataLength; i++)
	{
		if (i < attackTimeInSample)
		{
			if (sin(2 * M_PI * frequency * i / SamplingFrequency) > 0)
				data[i] = 32767 * attackGradient * i;
			else
				data[i] = -32767 * attackGradient * i;
		}
		else if (i < attackTimeInSample + decayTimeInSample)
		{
			if (sin(2 * M_PI * frequency * i / SamplingFrequency) > 0)
				data[i] = 32767 * (attackAmplitude + decayGradient * (i - attackTimeInSample));
			else
				data[i] = -32767 * (attackAmplitude + decayGradient * (i - attackTimeInSample));
		}
		else if (i < attackTimeInSample + decayTimeInSample + sustainTimeInSample)
		{
			if (sin(2 * M_PI * frequency * i / SamplingFrequency) > 0)
				data[i] = 32767 * sustainAmplitude;
			else
				data[i] = -32767 * sustainAmplitude;
		}
		else
		{
			if (sin(2 * M_PI * frequency * i / SamplingFrequency) > 0)
				data[i] = 32767 * (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample));
			else
				data[i] = -32767 * (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample));
		}
	}

	ALuint _source;
	ALuint _buffer;
	alGenSources(1, &_source);
	alGenBuffers(1, &_buffer);
	alBufferData(_buffer, AL_FORMAT_MONO16, data, sizeof(data), SamplingFrequency);
	alSourcei(_source, AL_BUFFER, _buffer);
	alSourcePlay(_source);
}

void DS_TriangleADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime)
{
	if (attackAmplitude > 1)
		attackAmplitude = 1;
	else if (attackAmplitude < 0)
		attackAmplitude = 0;

	if (sustainAmplitude > 1)
		sustainAmplitude = 1;
	else if (sustainAmplitude < 0)
		sustainAmplitude = 0;

	int attackTimeInSample = (int)(attackTime * SamplingFrequency);
	int decayTimeInSample = (int)(decayTime * SamplingFrequency);
	int sustainTimeInSample = (int)(sustainTime * SamplingFrequency);
	int releaseTimeInSample = (int)(releaseTime * SamplingFrequency);

	double attackGradient = attackAmplitude / attackTimeInSample;
	double decayGradient = (sustainAmplitude - attackAmplitude) / decayTimeInSample;
	double releaseGradient = -sustainAmplitude / releaseTimeInSample;

	int dataLength = attackTimeInSample + decayTimeInSample + sustainTimeInSample + releaseTimeInSample;
	ALshort data[dataLength];

	double currentSample = 0;
	double samplePerPeriod = SamplingFrequency / frequency;
	double gradient = 32767 * 4 * frequency / SamplingFrequency;
	for (int i = 0; i < dataLength; i++)
	{
		if (currentSample > samplePerPeriod)
			currentSample -= samplePerPeriod;

		if (i < attackTimeInSample)
		{
			if (currentSample < samplePerPeriod / 4)
				data[i] = attackGradient * i * (gradient * currentSample);
			else if (currentSample < samplePerPeriod / 2)
				data[i] = attackGradient * i * (32767 - gradient * (currentSample - (samplePerPeriod / 4)));
			else if (currentSample < samplePerPeriod * 3 / 4)
				data[i] = attackGradient * i * (-gradient * (currentSample - (samplePerPeriod / 2)));
			else
				data[i] = attackGradient * i * (-32767 + gradient * (currentSample - (samplePerPeriod * 3 / 4)));
		}
		else if (i < attackTimeInSample + decayTimeInSample)
		{
			if (currentSample < samplePerPeriod / 4)
				data[i] = (attackAmplitude + decayGradient * (i - attackTimeInSample)) * (gradient * currentSample);
			else if (currentSample < samplePerPeriod / 2)
				data[i] = (attackAmplitude + decayGradient * (i - attackTimeInSample)) * (32767 - gradient * (currentSample - (samplePerPeriod / 4)));
			else if (currentSample < samplePerPeriod * 3 / 4)
				data[i] = (attackAmplitude + decayGradient * (i - attackTimeInSample)) * (-gradient * (currentSample - (samplePerPeriod / 2)));
			else
				data[i] = (attackAmplitude + decayGradient * (i - attackTimeInSample)) * (-32767 + gradient * (currentSample - (samplePerPeriod * 3 / 4)));
		}
		else if (i < attackTimeInSample + decayTimeInSample + sustainTimeInSample)
		{
			if (currentSample < samplePerPeriod / 4)
				data[i] = sustainAmplitude * (gradient * currentSample);
			else if (currentSample < samplePerPeriod / 2)
				data[i] = sustainAmplitude * (32767 - gradient * (currentSample - (samplePerPeriod / 4)));
			else if (currentSample < samplePerPeriod * 3 / 4)
				data[i] = sustainAmplitude * (-gradient * (currentSample - (samplePerPeriod / 2)));
			else
				data[i] = sustainAmplitude * (-32767 + gradient * (currentSample - (samplePerPeriod * 3 / 4)));
		}
		else
		{
			if (currentSample < samplePerPeriod / 4)
				data[i] = (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample)) * (gradient * currentSample);
			else if (currentSample < samplePerPeriod / 2)
				data[i] = (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample)) * (32767 - gradient * (currentSample - (samplePerPeriod / 4)));
			else if (currentSample < samplePerPeriod * 3 / 4)
				data[i] = (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample)) * (-gradient * (currentSample - (samplePerPeriod / 2)));
			else
				data[i] = (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample)) * (-32767 + gradient * (currentSample - (samplePerPeriod * 3 / 4)));
		}
		currentSample++;
	}

	ALuint _source;
	ALuint _buffer;
	alGenSources(1, &_source);
	alGenBuffers(1, &_buffer);
	alBufferData(_buffer, AL_FORMAT_MONO16, data, sizeof(data), SamplingFrequency);
	alSourcei(_source, AL_BUFFER, _buffer);
	alSourcePlay(_source);
}

void DS_SawADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime)
{
	if (attackAmplitude > 1)
		attackAmplitude = 1;
	else if (attackAmplitude < 0)
		attackAmplitude = 0;

	if (sustainAmplitude > 1)
		sustainAmplitude = 1;
	else if (sustainAmplitude < 0)
		sustainAmplitude = 0;

	int attackTimeInSample = (int)(attackTime * SamplingFrequency);
	int decayTimeInSample = (int)(decayTime * SamplingFrequency);
	int sustainTimeInSample = (int)(sustainTime * SamplingFrequency);
	int releaseTimeInSample = (int)(releaseTime * SamplingFrequency);

	double attackGradient = attackAmplitude / attackTimeInSample;
	double decayGradient = (sustainAmplitude - attackAmplitude) / decayTimeInSample;
	double releaseGradient = -sustainAmplitude / releaseTimeInSample;

	int dataLength = attackTimeInSample + decayTimeInSample + sustainTimeInSample + releaseTimeInSample;
	ALshort data[dataLength];

	double currentSample = 0;
	double samplePerPeriod = SamplingFrequency / frequency;
	double gradient = 32767 * 2 * frequency / SamplingFrequency;
	for (int i = 0; i < dataLength; i++)
	{
		if (currentSample > samplePerPeriod)
			currentSample -= samplePerPeriod;

		if (i < attackTimeInSample)
		{
			if (currentSample < samplePerPeriod / 2)
				data[i] = attackGradient * i * (gradient * currentSample);
			else
				data[i] = attackGradient * i * (-32767 + gradient * (currentSample - (samplePerPeriod / 2)));
		}
		else if (i < attackTimeInSample + decayTimeInSample)
		{
			if (currentSample < samplePerPeriod / 2)
				data[i] = (attackAmplitude + decayGradient * (i - attackTimeInSample)) * (gradient * currentSample);
			else
				data[i] = (attackAmplitude + decayGradient * (i - attackTimeInSample)) * (-32767 + gradient * (currentSample - (samplePerPeriod / 2)));
		}
		else if (i < attackTimeInSample + decayTimeInSample + sustainTimeInSample)
		{
			if (currentSample < samplePerPeriod / 2)
				data[i] = sustainAmplitude * (gradient * currentSample);
			else
				data[i] = sustainAmplitude * (-32767 + gradient * (currentSample - (samplePerPeriod / 2)));
		}
		else
		{
			if (currentSample < samplePerPeriod / 2)
				data[i] = (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample)) * (gradient * currentSample);
			else
				data[i] = (sustainAmplitude + releaseGradient * (i - attackTimeInSample - decayTimeInSample - sustainTimeInSample)) * (-32767 + gradient * (currentSample - (samplePerPeriod / 2)));
		}
		currentSample++;
	}

	ALuint _source;
	ALuint _buffer;
	alGenSources(1, &_source);
	alGenBuffers(1, &_buffer);
	alBufferData(_buffer, AL_FORMAT_MONO16, data, sizeof(data), SamplingFrequency);
	alSourcei(_source, AL_BUFFER, _buffer);
	alSourcePlay(_source);
}

void DS_Close()
{
	alSourceStop(source);
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

void DS_Initialize()
{
	SetInitialWaveData();
	// Open Device
	device = alcOpenDevice(NULL);
	// Generate Context
	context = alcCreateContext(device, NULL);
	// Assign Context to Use
	alcMakeContextCurrent(context);
	// Get Sampling Frequency
	alcGetIntegerv(device, ALC_FREQUENCY, 1, &SamplingFrequency);
}

void DS_Play()
{
	alSourcePause(source);
	alDeleteSources(1, &source);
	alGenSources(1, &source);
	alGenBuffers(1, &buffer);
	alBufferData(buffer, AL_FORMAT_MONO16, waveData, sizeof(waveData), SamplingFrequency);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcePlay(source);

	SetInitialWaveData();
}

/*
void DS_Random(double amplitude)
{
	if (amplitude > 1)
		amplitude = 1;
	else if (amplitude < 0)
		amplitude = 0;

	uniform_real_distribution<> dist(-amplitude, amplitude);
	random_device seed;
	mt19937 rd(seed());

	for(int i = 0; i < dataLength; i++)
	{
		waveData[i] += dist(rd);
	}
}*/

/*
void DS_Queue()
{
	// Unqueue Played Buffer
	alSourceUnqueueBuffers(source, 1, &buffer);
	alGenBuffers(1, &buffer);
	// Apply Sound Data to Buffer
	alBufferData(buffer, AL_FORMAT_MONO16, waveData, sizeof(waveData), SamplingFrequency);
	// Queue
	alSourceQueueBuffers(source, 1, &buffer);

	int sampleOffset;
	alGetSourcei(source, AL_SAMPLE_OFFSET, &sampleOffset);
	cout << sampleOffset << endl;

	alSourceStop(source);
	alSourcei(source, AL_SAMPLE_OFFSET, dataLength);
	alGetSourcei(source, AL_SAMPLE_OFFSET, &sampleOffset);
	cout << sampleOffset << endl;
	alSourcePlay(source);

	SetInitialWaveData();
}*/