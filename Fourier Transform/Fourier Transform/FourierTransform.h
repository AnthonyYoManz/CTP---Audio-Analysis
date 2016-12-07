#pragma once
#include <iostream>
#include <vector>
#include <math.h>
#include <utility>
#include "Aliases.h"

const real PI = 3.141592653589793238460;

struct FrequencyBin
{
	int m_index;
	real m_magnitude;
};

struct FFTResult
{
	std::vector<real*> m_samples;
	unsigned int m_sampleCount;
	unsigned int m_binCountPerInstant;
	unsigned int m_samplesPerInstant;
	std::vector<real*> m_magnitudes;
	std::vector<FrequencyBin> m_maxMagnitudeBins;
};

void freeFFTResult(FFTResult& _fftResult);

//using this windowing func for now, need to research most suitable one
//although brief reading of one white paper makes it sound like this is the GOAT
void applyHannWindow(real* _samples, real* _output, unsigned int _sampleCount);

void splitSamples(real* _samples, unsigned int _sampleCount);

//p sure i can do this without complex inputs but this is the most well documented 
//version of fft as far as i can tell.
//i'd have done a bunch of more experimental implementations if i hadn't bummed myself
//with a 2 week time constraint (will probably do a less optimised version of my own later).
//this version is basically a rewritten version of the one in numerical recipes but changed a bit so it works
//easily with audio (real) samples.
void computeFFT(real* _samples, real* _output, unsigned int _sampleCount);

void splitSamplesIntoChannels(real* _samples, unsigned int _totalSampleCount, real*& _outLeft, real*& _outRight);

//long name but kinda necessary for the specificity of what's actually happening
FFTResult compute2ChannelRealTo1ChannelComplexFFT(real* _samples, unsigned int _sampleCount, unsigned int _sampleCountPerInstant);