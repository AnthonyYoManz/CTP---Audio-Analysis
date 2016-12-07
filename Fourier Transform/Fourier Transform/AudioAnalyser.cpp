#include "AudioAnalyser.h"

AudioAnalyser::AudioAnalyser()
{
	m_samples = nullptr;
	m_sampleCount = 0;
	m_shortSamples = nullptr;
	m_shortSampleCount = 0;
}

AudioAnalyser::~AudioAnalyser()
{
	cleanup();
}

void AudioAnalyser::setSamples(real* _samples, unsigned int _sampleCount)
{
	if (m_samples)
	{
		delete[] m_samples;
		m_samples = nullptr;
	}
	m_samples = new real[_sampleCount];
	m_sampleCount = _sampleCount;
	memcpy(m_samples, _samples, sizeof(real)*_sampleCount);
}

void AudioAnalyser::cleanup()
{
	if (m_samples)
	{
		delete[] m_samples;
		m_samples = nullptr;
	}
	if (m_shortSamples)
	{
		delete[] m_shortSamples;
		m_shortSamples = nullptr;
	}
	freeFFTResult(m_fftResult);
}

void AudioAnalyser::analyseAudio(unsigned int _instantSize)
{
	m_fftResult = compute2ChannelRealTo1ChannelComplexFFT(m_samples, m_sampleCount, _instantSize);
}

void AudioAnalyser::analyseAudio(unsigned int _binCount, unsigned int _instantSize)
{
	unsigned int intendedInstantSize = _binCount * 2;
	unsigned int actualInstantSize = (int)(log2(intendedInstantSize) + 0.5);	//round to nearest pow 2
	actualInstantSize = pow(2, actualInstantSize);
	real sampleInterval = (real)_instantSize / actualInstantSize;
	std::cout << actualInstantSize << "\n";
	unsigned int cutSamplesLength = (m_sampleCount / sampleInterval);
	std::cout << m_sampleCount << ", " << cutSamplesLength << "\n";
	//round correctly
	if ((real)cutSamplesLength != m_sampleCount / sampleInterval)
	{
		cutSamplesLength++;
	}
	if (m_shortSamples)
	{
		delete[] m_shortSamples;
	}
	m_shortSamples = new real[cutSamplesLength];
	m_shortSampleCount = cutSamplesLength;
	for (real i = 0; i < cutSamplesLength; i++)
	{
		real sample = 0;
		int jIterations = 0;
		for (int j = 0; j < sampleInterval; j++)
		{
			if (i*sampleInterval + j < m_sampleCount)
			{
				jIterations++;
				sample += m_samples[(int)(sampleInterval*i + j)];
			}
		}
		if (jIterations != 0)
		{
			sample /= jIterations;
		}
		m_shortSamples[(int)i] = sample;
	}
	m_fftResult = compute2ChannelRealTo1ChannelComplexFFT(m_shortSamples, cutSamplesLength, actualInstantSize);
	std::cout << m_shortSampleCount << " samples in short list\n";
	std::cout << m_sampleCount << " samples in full list\n";
}

const real* AudioAnalyser::getSamples()
{
	return m_samples;
}

unsigned int AudioAnalyser::getSampleCount()
{
	return m_sampleCount;
}

const real * AudioAnalyser::getShortSamples()
{
	return m_shortSamples;
}

unsigned int AudioAnalyser::getShortSampleCount()
{
	return m_shortSampleCount;
}

const FFTResult& AudioAnalyser::getFFTResult()
{
	return m_fftResult;
}
