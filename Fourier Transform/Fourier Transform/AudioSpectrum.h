#pragma once
#include <vector>
#include "AudioSpectrumInstant.h"
#include "FourierTransform.h"

class AudioSpectrum
{
private:
	std::vector<AudioSpectrumInstant> m_spectrum;
	std::vector<AudioSpectrumInstant> m_normalisedSpectrum;
	bool m_initialised;
public:
	AudioSpectrum();
	void generateSpectrum(const FFTResult& _fft);
	const std::vector<AudioSpectrumInstant>& getSpectrum();
	const std::vector<AudioSpectrumInstant>& getNormalisedSpectrum();
	const AudioSpectrumInstant& getSpectrumInstant(unsigned int _index) const;
	const AudioSpectrumInstant& getNormalisedSpectrumInstant(unsigned int _index) const;
	bool isInitialised();
};