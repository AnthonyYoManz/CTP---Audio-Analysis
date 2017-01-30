#include "AudioSpectrum.h"
#include <iostream>

AudioSpectrum::AudioSpectrum()
{
	m_initialised = false;
}

void AudioSpectrum::generateSpectrum(const FFTResult& _fft)
{
	m_initialised = true;
	m_spectrum.reserve(_fft.m_magnitudes.size());
	m_normalisedSpectrum.reserve(_fft.m_magnitudes.size());

	unsigned int sampleRate = 44100;		//currently just assumes this, potential subject to change
	real binFreqRange = (sampleRate/2) / (real)_fft.m_binCountPerInstant;
	unsigned int subBassBin;
	unsigned int subBassRange;
	unsigned int bassBin;
	unsigned int bassRange;
	unsigned int lowMidrangeBin;
	unsigned int lowMidrangeRange;
	unsigned int midrangeBin;
	unsigned int midrangeRange;
	unsigned int upperMidrangeBin;
	unsigned int upperMidrangeRange;
	unsigned int presenceBin;
	unsigned int presenceRange;
	unsigned int brillianceBin;
	unsigned int brillianceRange;
	int curFreq = binFreqRange;
	int curBin = 1;	//skip 0
	int startBin = curBin;
	subBassBin = curBin;
	while (curFreq < 60)
	{
		curBin++;
		curFreq += binFreqRange;
	}
	subBassRange = (curBin - startBin);
	startBin = curBin;
	bassBin = curBin;
	while (curFreq < 250)
	{
		curBin++;
		curFreq += binFreqRange;
	}
	bassRange = (curBin - startBin);
	startBin = curBin;
	lowMidrangeBin = curBin;
	while (curFreq < 500)
	{
		curBin++;
		curFreq += binFreqRange;
	}
	lowMidrangeRange = (curBin - startBin);
	startBin = curBin;
	midrangeBin = curBin;
	while (curFreq < 2000)
	{
		curBin++;
		curFreq += binFreqRange;
	}
	midrangeRange = (curBin - startBin);
	startBin = curBin;
	upperMidrangeBin = curBin;
	while (curFreq < 4000)
	{
		curBin++;
		curFreq += binFreqRange;
	}
	upperMidrangeRange = (curBin - startBin);
	startBin = curBin;
	presenceBin = curBin;
	while (curFreq < 6000)
	{
		curBin++;
		curFreq += binFreqRange;
	}
	presenceRange = (curBin - startBin);
	startBin = curBin;
	brillianceBin = curBin;
	while (curFreq < 20000)
	{
		curBin++;
		curFreq += binFreqRange;
	}
	brillianceRange = (curBin - startBin);

	float maxBandValue = 0;
	for (real* bins : _fft.m_magnitudes)
	{
		AudioSpectrumInstant instant = { 0, 0, 0, 0, 0, 0, 0 };
		for (int i = subBassBin; i < subBassBin + subBassRange; i++)
		{
			real mag = bins[i];
			instant.m_subBass += mag;
		}
		for (int i = bassBin; i < bassBin + bassRange; i++)
		{
			instant.m_bass += bins[i];
		}
		for (int i = lowMidrangeBin; i < lowMidrangeBin + lowMidrangeRange; i++)
		{
			instant.m_lowMidrange += bins[i];
		}
		for (int i = midrangeBin; i < midrangeBin + midrangeRange; i++)
		{
			instant.m_midrange += bins[i];
		}
		for (int i = upperMidrangeBin; i < upperMidrangeBin + upperMidrangeRange; i++)
		{
			instant.m_upperMidrange += bins[i];
		}
		for (int i = presenceBin; i < presenceBin + presenceRange; i++)
		{
			instant.m_presence += bins[i];
		}
		for (int i = brillianceBin; i < brillianceBin + brillianceRange; i++)
		{
			instant.m_brilliance += bins[i];
		}
		m_spectrum.push_back(instant);
		m_normalisedSpectrum.push_back(instant);
		maxBandValue = instant.m_subBass > maxBandValue ? instant.m_subBass : maxBandValue;
		maxBandValue = instant.m_bass > maxBandValue ? instant.m_bass : maxBandValue;
		maxBandValue = instant.m_lowMidrange > maxBandValue ? instant.m_lowMidrange : maxBandValue;
		maxBandValue = instant.m_midrange > maxBandValue ? instant.m_midrange : maxBandValue;
		maxBandValue = instant.m_upperMidrange > maxBandValue ? instant.m_upperMidrange : maxBandValue;
		maxBandValue = instant.m_presence > maxBandValue ? instant.m_presence : maxBandValue;
		maxBandValue = instant.m_brilliance > maxBandValue ? instant.m_brilliance : maxBandValue;
	}

	for (auto& instant : m_normalisedSpectrum)
	{
		instant.m_subBass /= maxBandValue;
		instant.m_bass /= maxBandValue;
		instant.m_lowMidrange /= maxBandValue;
		instant.m_midrange /= maxBandValue;
		instant.m_upperMidrange /= maxBandValue;
		instant.m_presence /= maxBandValue;
		instant.m_brilliance /= maxBandValue;
	}
}

const std::vector<AudioSpectrumInstant>& AudioSpectrum::getSpectrum()
{
	return m_spectrum;
}

const std::vector<AudioSpectrumInstant>& AudioSpectrum::getNormalisedSpectrum()
{
	return m_normalisedSpectrum;
}

const AudioSpectrumInstant& AudioSpectrum::getSpectrumInstant(unsigned int _index) const
{
	return m_spectrum.at(_index);
}

const AudioSpectrumInstant& AudioSpectrum::getNormalisedSpectrumInstant(unsigned int _index) const
{
	return m_normalisedSpectrum.at(_index);
}

bool AudioSpectrum::isInitialised()
{
	return m_initialised;
}
