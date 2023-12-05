#pragma once
#include <math.h>
#include <Windows.h>

/* Generate Sine Wave Sound. Amplitude range is 0 to 1. */
extern void DS_Sine(double amplitude, double frequency);

/* Generate Rect Wave Sound. Amplitude range is 0 to 1. */
extern void DS_Rect(double amplitude, double frequency);

/* Generate Triangle Wave Sound. Amplitude range is 0 to 1. */
extern void DS_Triangle(double amplitude, double frequency);

/* Generate Saw Tooth Wave Sound. Amplitude range is 0 to 1. */
extern void DS_Saw(double amplitude, double frequency);

/* Generate Random Wave Sound. Amplitude range is 0 to 1. */
//extern void DS_Random(double amplitude);

/* Sine Wave Envelope Generator. Amplitude range is 0 to 1. Time unit is second. */
extern void DS_SineADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime);

/* Rect Wave Envelope Generator. Amplitude range is 0 to 1. Time unit is second. */
extern void DS_RectADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime);

/* Triangle Wave Envelope Generator. Amplitude range is 0 to 1. Time unit is second. */
extern void DS_TriangleADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime);

/* Saw Tooth Wave Envelope Generator. Amplitude range is 0 to 1. Time unit is second. */
extern void DS_SawADSR(double frequency, double attackAmplitude, double sustainAmplitude, double attackTime, double decayTime, double sustainTime, double releaseTime);

/* Close Sound Device */
extern void DS_Close();

/* Exucute This When Initializing Game */
extern void DS_Initialize();

/* Play Sound Generated by DS_Sine(), DS_Rect() and DS_Triangle() */
extern void DS_Play();