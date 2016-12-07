#pragma once
#include "Aliases.h"
#include "FourierTransform.h"

class AudioAnalyser
{
private:
	FFTResult m_fftResult;
	real* m_samples;
	unsigned int m_sampleCount;
	real* m_shortSamples;	//for when bin count is specified
	unsigned int m_shortSampleCount;	//for when bin count is specified
public:
	AudioAnalyser();
	~AudioAnalyser();
	void setSamples(real* _samples, unsigned int _sampleCount);
	void cleanup();
	void analyseAudio(unsigned int _instantSize);
	void analyseAudio(unsigned int _binCount, unsigned int _instantSize);
	const real* getSamples();
	unsigned int getSampleCount();
	const real* getShortSamples();
	unsigned int getShortSampleCount();
	const FFTResult& getFFTResult();
};