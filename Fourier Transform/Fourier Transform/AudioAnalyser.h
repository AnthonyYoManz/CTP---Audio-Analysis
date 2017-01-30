#pragma once
#include "Aliases.h"
#include "FourierTransform.h"
#include "AudioSpectrum.h"
#include "OutlierDetector.h"

struct AudioInstantData
{
	const real* m_samples;
	unsigned int m_sampleSize;
	const real* m_frequencyMagnitudes;
	unsigned int m_binCount;
	const AudioSpectrumInstant* m_audioSpectrum;
	bool m_potentialBeat;
};

class AudioAnalyser
{
private:
	OutlierDetector m_beatDetector;
	FFTResult m_fftResult;
	FFTResult* m_normalisedFFTResult;
	AudioSpectrum m_audioSpectrum;
	real* m_srcSamples;	//original samples
	unsigned int m_srcSampleCount;
	real* m_samples;	//normalised/modified
	unsigned int m_sampleCount;
public:
	AudioAnalyser();
	~AudioAnalyser();
	void setSamples(real* _samples, unsigned int _sampleCount);
	void cleanup();
	void analyseAudio(unsigned int _instantSize);
	void analyseAudio(unsigned int _binCount, unsigned int _instantSize);
	const real* getSourceSamples();
	unsigned int getSourceSampleCount();
	const real* getSamples();
	unsigned int getSampleCount();
	const FFTResult& getFFTResult();
	const FFTResult& getNormalisedFFTResult();
	const AudioSpectrum& getAudioSpectrum();
	AudioInstantData getInstantDataAtIndex(unsigned int _index);
};