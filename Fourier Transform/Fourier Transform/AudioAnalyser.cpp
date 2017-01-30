#include "AudioAnalyser.h"

AudioAnalyser::AudioAnalyser()
{
	m_srcSamples = nullptr;
	m_srcSampleCount = 0;
	m_samples = nullptr;
	m_sampleCount = 0;
	m_normalisedFFTResult = nullptr;
}

AudioAnalyser::~AudioAnalyser()
{
	cleanup();
}

void AudioAnalyser::setSamples(real* _samples, unsigned int _sampleCount)
{
	if (m_srcSamples)
	{
		delete[] m_srcSamples;
		m_srcSamples = nullptr;
	}
	m_srcSamples = new real[_sampleCount];
	m_srcSampleCount = _sampleCount;
	memcpy(m_srcSamples, _samples, sizeof(real) * _sampleCount);
}

void AudioAnalyser::cleanup()
{
	if (m_srcSamples)
	{
		delete[] m_srcSamples;
		m_srcSamples = nullptr;
	}
	if (m_samples)
	{
		delete[] m_samples;
		m_samples = nullptr;
	}
	freeFFTResult(m_fftResult);
	if (m_normalisedFFTResult)
	{
		freeFFTResult(*m_normalisedFFTResult);
		delete m_normalisedFFTResult;
		m_normalisedFFTResult = nullptr;
	}
}

void AudioAnalyser::analyseAudio(unsigned int _instantSize)
{
	analyseAudio(_instantSize / 2, _instantSize);
}

void AudioAnalyser::analyseAudio(unsigned int _binCount, unsigned int _instantSize)
{
	unsigned int intendedInstantSize = _binCount * 2;
	//round to nearest pow 2
	unsigned int actualInstantSize = (int)(log2(intendedInstantSize) + 0.5);
	actualInstantSize = pow(2, actualInstantSize);
	real sampleInterval = (real)_instantSize / actualInstantSize;
	std::cout << actualInstantSize << "\n";
	unsigned int cutSamplesLength = (m_srcSampleCount / sampleInterval);
	std::cout << m_srcSampleCount << ", " << cutSamplesLength << "\n";
	//round correctly this time
	if ((real)cutSamplesLength != m_srcSampleCount / sampleInterval)
	{
		cutSamplesLength++;
	}
	
	real* cutSamples = new real[cutSamplesLength];
	for (real i = 0; i < cutSamplesLength; i++)
	{
		real sample = 0;
		int jIterations = 0;
		for (int j = 0; j < sampleInterval; j++)
		{
			if (i*sampleInterval + j < m_srcSampleCount)
			{
				jIterations++;
				sample += m_srcSamples[(int)(sampleInterval*i + j)];
			}
		}
		if (jIterations != 0)
		{
			sample /= jIterations;
		}
		cutSamples[(int)i] = sample;
	}
	//merge audio channels into 1
	m_sampleCount = cutSamplesLength/2;
	if (m_samples)
	{
		delete[] m_samples;
	}
	m_samples = new real[m_sampleCount];
	for (int i = 0; i < m_sampleCount; i++)
	{
		m_samples[i] = cutSamples[i * 2] + cutSamples[(i * 2) + 1];
		m_samples[i] /= 2.0f;
	}
	unsigned int binCount = actualInstantSize / 2;
	int currentInstant = 0;

	//perform fft on merged samples
	m_fftResult = computeRealToComplexFFT(m_samples, m_sampleCount, actualInstantSize);

	m_beatDetector.detectOutliers(m_samples, m_sampleCount, actualInstantSize, 50);
	std::cout << m_sampleCount << " samples in short list\n";
	std::cout << m_srcSampleCount << " samples in full list\n";
}

const real* AudioAnalyser::getSourceSamples()
{
	return m_srcSamples;
}

unsigned int AudioAnalyser::getSourceSampleCount()
{
	return m_srcSampleCount;
}

const real * AudioAnalyser::getSamples()
{
	return m_samples;
}

unsigned int AudioAnalyser::getSampleCount()
{
	return m_sampleCount;
}

const FFTResult& AudioAnalyser::getFFTResult()
{
	return m_fftResult;
}

const FFTResult& AudioAnalyser::getNormalisedFFTResult()
{
	if (!m_normalisedFFTResult)
	{
		m_normalisedFFTResult = new FFTResult;
		m_normalisedFFTResult->m_binCountPerInstant = m_fftResult.m_binCountPerInstant;
		m_normalisedFFTResult->m_maxMagnitudeBins = m_fftResult.m_maxMagnitudeBins;
		m_normalisedFFTResult->m_sampleCount = m_fftResult.m_sampleCount;
		m_normalisedFFTResult->m_samplesPerInstant = m_fftResult.m_samplesPerInstant;
		for (unsigned int i = 0; i < m_fftResult.m_magnitudes.size(); i++)
		{
			unsigned int size = m_fftResult.m_samplesPerInstant / 2;
			m_normalisedFFTResult->m_magnitudes.push_back(new real[size]);
			memcpy(m_normalisedFFTResult->m_magnitudes.at(i), m_fftResult.m_magnitudes.at(i), sizeof(real) * size);
		}
		for (unsigned int i = 0; i < m_fftResult.m_samples.size(); i++)
		{
			unsigned int size = m_fftResult.m_samplesPerInstant;
			m_normalisedFFTResult->m_samples.push_back(new real[size]);
			memcpy(m_normalisedFFTResult->m_samples.at(i), m_fftResult.m_samples.at(i), sizeof(real) * size);
		}

		real highestMag = 0;
		for (auto& maxMagBin : m_normalisedFFTResult->m_maxMagnitudeBins)
		{
			highestMag = (maxMagBin.m_magnitude > highestMag) ? maxMagBin.m_magnitude : highestMag;
		}
		for (auto& binMag : m_normalisedFFTResult->m_magnitudes)
		{
			for (int i = 0; i < m_normalisedFFTResult->m_samplesPerInstant / 2; i++)
			{
				binMag[i] /= highestMag;
			}
		}
		for (auto& maxMagBin : m_normalisedFFTResult->m_maxMagnitudeBins)
		{
			maxMagBin.m_magnitude /= highestMag;
		}
	}
	return *m_normalisedFFTResult;
}

const AudioSpectrum & AudioAnalyser::getAudioSpectrum()
{
	if(!m_audioSpectrum.isInitialised())
	{ 
		m_audioSpectrum.generateSpectrum(m_fftResult);
	}
	return m_audioSpectrum;
}

AudioInstantData AudioAnalyser::getInstantDataAtIndex(unsigned int _index)
{
	AudioInstantData data;
	data.m_audioSpectrum = &(getAudioSpectrum().getNormalisedSpectrumInstant(_index));
	data.m_frequencyMagnitudes = m_fftResult.m_magnitudes.at(_index);
	data.m_binCount = m_fftResult.m_binCountPerInstant;
	data.m_potentialBeat = false;
	for (auto& i : m_beatDetector.getOutlierIndexes())
	{
		if (i == _index)
		{
			data.m_potentialBeat = true;
		}
	}
	data.m_samples = &m_samples[m_fftResult.m_samplesPerInstant * _index];
	data.m_sampleSize = m_fftResult.m_samplesPerInstant;
	return data;
}
