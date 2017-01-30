#include "FourierTransform.h"

void freeFFTResult(FFTResult& _fftResult)
{
	for (auto* i : _fftResult.m_magnitudes)
	{
		delete[] i;
	}
	for (auto* i : _fftResult.m_samples)
	{
		delete[] i;
	}
}

//using this windowing func for now, need to research most suitable one
//although brief reading of one white paper makes it sound like this is the GOAT
void applyHannWindow(real* _samples, real* _output, unsigned int _sampleCount)
{
	for (unsigned int i = 0; i < _sampleCount; i++)
	{
		real multiplier = 0.5 * (1.0 - cos(2.0 * PI * i / (_sampleCount - 1)));
		_output[i] = multiplier * _samples[i];
	}
	return;
}

void applyHammingWindow(real* _samples, real* _output, unsigned int _sampleCount)
{
	for (unsigned int i = 0; i < _sampleCount; i++)
	{
		real multiplier = 0.54 - 0.46 * cos(2.0 * PI * i / (_sampleCount - 1));
		_output[i] = multiplier * _samples[i];
	}
	return;
}

void splitSamples(real* _samples, unsigned int _sampleCount)
{
	for (unsigned int i = 1, j = 1, k = 0; i < _sampleCount - 1; i += 2)
	{
		if (j > 1)
		{
			std::swap(_samples[j], _samples[i]);
			std::swap(_samples[j + 1], _samples[i + 1]);
		}
		k = _sampleCount;
		while (k >= 2 && j > k)
		{
			j -= k;
			k >>= 1;	//k/=2, props to numerical recipes
		}
		j += k;
	}
}

void computeFFT(real * _samples, real * _output, unsigned int _sampleCount)
{
	//the version in numerical recipes has some weird stuff going on with the array starting at 1
	//this version is all about starting at 0
	for (unsigned int i = 0; i < _sampleCount; i++)
	{
		_output[i * 2] = _samples[i];	//reals on even index, imag on odd
		_output[i * 2 + 1] = 0;			//imag part of output is 0 since audio samples aren't complex
	}

	//bit reversal
	unsigned int valueCount = _sampleCount << 1; //wow look at my smart way of *2 (because input sample size must be pow of 2)
	unsigned int j = 0;
	for (int i = 0; i < valueCount / 2; i += 2)	//loop through all pairs (each pair is a complex number of (a + bi) where b = 0)
	{
		//ok so this is a pretty neat loop which i didn't figure out on my own (i cry)
		//basically splits the samples up repeatedly into even and odd indexes Log2(N) times
		//this basically results in the bit representation of the indexes (not the elements at said index)
		//being flipped. i.e. 0001 becomes 1000 and 0100 becomes 0010
		//this is especially great cause it does everything in-place, so memory is not unnecessarily used
		if (j > i)
		{
			std::swap(_output[j], _output[i]); //swaps the real numbers
			if ((j / 2)<(valueCount / 4)) {
				std::swap(_output[(valueCount - (i + 2))], _output[(valueCount - (j + 2))]);
			}
		}
		//this next bit is the great part that makes j jump around to the correct places
		unsigned int m = valueCount / 2;
		while (m >= 2 && j >= m) {	//i wish i processed this in my head right-to-left every time it'd have saved me some headaches
			j -= m;
			m = m / 2;
		}
		j += m;
	}


	unsigned int mmax = 2;	//this doesn't really identify the subgroups (mentioned later, resolved earlier) but it gets
							//squared each time a subgroup in the following loops is DFT'd up
							//just saying this here so the while loop doesn't immediately look terrible

							//this is the Rader–Brenner algorithm the original documentation of which is proving hard to find!
	while (valueCount > mmax)	// should execute the loop log2(_sampleCount) times (yes _sampleCount, not valueCount, see the mmax << 1 and _sampleCount << 1 lines)
	{
		unsigned int istep = mmax << 1;	//4, 8, 16, 32 ... valueCount
		real theta = 2 * PI / mmax;		//resolve a bunch of values that can be reused across numerous DFTs
		real wtemp = sin(0.5*theta);	//solving this trig stuff early is especially good as it's slow as heck
		real wpr = -2.0*wtemp*wtemp;
		real wpi = sin(theta);
		real wr = 1.0;
		real wi = 0.0;

		//The arrangement of our _output array (which is also kind of an input array)
		//is such that the following loops iterate through the specific values needed for
		//each dft that needs to be performed on subgroups of samples
		//istep was set up earlier in such a way that the jumps would occur as intended for each subgroup
		//i can't help but feel that the people that figured this out are really quite smart
		for (unsigned int m = 1; m < mmax; m += 2) {
			for (unsigned int i = m; i <= valueCount; i += istep) {
				j = i + mmax;
				real tempr = wr*_output[j - 1] - wi*_output[j];
				real tempi = wr*_output[j] + wi*_output[j - 1];
				_output[j - 1] = _output[i - 1] - tempr;
				_output[j] = _output[i] - tempi;
				_output[i - 1] += tempr;
				_output[i] += tempi;
			}
			wtemp = wr;
			wr = wr * wpr - wi * wpi + wr;
			wi = wi * wpr + wtemp * wpi + wi;
		}
		mmax = istep;
	}
}

FFTResult computeRealToComplexFFT(real * _samples, unsigned int _sampleCount, unsigned int _sampleCountPerInstant)
{
	FFTResult output;
	unsigned int instantCount = _sampleCount / _sampleCountPerInstant;
	if (instantCount != (float)_sampleCount / _sampleCountPerInstant)
	{
		instantCount++;
	}
	
	output.m_sampleCount = _sampleCountPerInstant * instantCount;
	output.m_binCountPerInstant = _sampleCountPerInstant /2;
	output.m_samplesPerInstant = _sampleCountPerInstant;

	for (unsigned int iInstant = 0; iInstant < instantCount; iInstant++)
	{
		output.m_samples.push_back(new real[_sampleCountPerInstant * 2]);
		output.m_magnitudes.push_back(new real[_sampleCountPerInstant / 2]);	//second half is mirror of first half
		real* instant = new real[_sampleCountPerInstant];
		real* windowedInstant = new real[_sampleCountPerInstant];
		for (unsigned int i = 0; i < _sampleCountPerInstant; i++)
		{
			if (i + iInstant * _sampleCountPerInstant < _sampleCount)
			{
				//merge l and r channels into 1 by averaging
				instant[i] = _samples[i + iInstant * _sampleCountPerInstant];
			}
			else
			{
				//set excess samples to 0
				instant[i] = 0.0;
			}
		}
		//apply windowing function then do the FFT
		applyHannWindow(instant, windowedInstant, _sampleCountPerInstant);
		//applyHammingWindow(instant, windowedInstant, _sampleCountPerInstant); //for hamming window example
		computeFFT(windowedInstant, output.m_samples.back(), _sampleCountPerInstant);
		//computeFFT(instant, output.m_samples.back(), _sampleCountPerInstant); //without windowing

		//get the info we actually want from the FFT
		real maxMag = 0;
		int maxMagInd = 0;
		for (unsigned int i = 0; i < _sampleCountPerInstant / 2; i++)
		{
			//more efficient than, although equivalent to 20 * log10(sqrt(absoluteval^2))
			//because:  nlog10(x^y) == nylog10(x); in this case y = 1/2
			real realComp = output.m_samples.back()[i * 2];
			real imagComp = output.m_samples.back()[i * 2 + 1];
			real fftMag = sqrt(realComp * realComp + imagComp * imagComp);
			//fftMag /= (_sampleCountPerInstant / 2);
			//fftMag = 10 * log10(fftMag); //convert to decibels*/
			//fftMag = fabs(fftMag);
			//fftMag += 96;
			if (fftMag > maxMag && i != 0)	//ignore 0 for this cause it's the mean
			{
				maxMag = fftMag;	//store max magnitude
				maxMagInd = i;		//record its index (frequency bin)
			}
			output.m_magnitudes.back()[i] = (fftMag);
		}
		output.m_maxMagnitudeBins.push_back({ maxMagInd, maxMag });
		delete[] instant;
		delete[] windowedInstant;
	}
	std::cout << output.m_magnitudes.size() << " instants FFT'd\n";
	std::cout << output.m_binCountPerInstant << " bins per instant\n";
	return output;
}