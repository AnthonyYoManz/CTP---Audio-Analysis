//fouriertransform.h has all the maths
//i wrote a load more comments than perhaps necessary to show I UNDERSTAND WHAT'S GOING ON (to some degree)
//felt i had to do this since there is ~very~ similar code floating around the internet, i assume this is how
//it always goes with algorithms

//this file is a bodge job just to get the jam drawn on screen to see
//maths is in the other header
//anything good in here is commented

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <math.h>
#include <limits>
#include <vector>
#include "Aliases.h"
#include "FourierTransform.h"
#include "AudioAnalyser.h"

int main()
{
	bool tunePlaying = false;
	const unsigned int instantSize = 1024;

	sf::SoundBuffer tune;
	std::cout << "Welcome to my FFT thingy. Quick explanation:\n";
	std::cout << "The waveform on top is the current instant's samples.\n";
	std::cout << "The graph on the bottom is the frequency domain graph of the above instant.\n";
	std::cout << "You can use the arrow keys to manually change the current instant.\n";
	std::cout << "Alternatively, press the space bar to play/stop the audio track on the current instant.\n";
	std::cout << "Select an audio track (440Hz track will be defaulted to in the case of erroneous input): \n";
	std::cout << "1) 440Hz sin wave\n2) 20000Hz sin wave\n3) Some tune from Necrodancer\n4) Some tune from a Mario game\n";
	int userInput = 0;
	std::cin >> userInput;

	//make window
	sf::RenderWindow window(sf::VideoMode(1280, 480), "FFT example");

	switch (userInput)
	{
	case 2:
	{
		tune.loadFromFile("overlaid waves.ogg");
		break;
	}
	case 3:
	{
		tune.loadFromFile("tune.ogg");
		break;
	}
	case 4:
	{
		tune.loadFromFile("other tune.ogg");
		break;
	}
	default:
	{
		tune.loadFromFile("sin440.ogg");
		break;
	}
	}

	real* samples = new real[tune.getSampleCount()];
	for (int i = 0; i < tune.getSampleCount(); i++)
	{
		samples[i] = tune.getSamples()[i] / (real)std::numeric_limits<sf::Int16>::max();
	}
	int sampleCount = tune.getSampleCount() / 2;

	int currentInstant = 0;
	float secondsPerInstant = (float)instantSize / tune.getSampleRate();
	
	AudioAnalyser audioAnalyser;
	audioAnalyser.setSamples(samples, tune.getSampleCount());
	audioAnalyser.analyseAudio(512, 1024);
	//audioAnalyser.analyseAudio(1024);

	const FFTResult& fftResult = audioAnalyser.getFFTResult();
	sf::Sound player(tune);
	player.setLoop(true);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			if (event.type == sf::Event::KeyPressed)
			{
				switch (event.key.code)
				{
				case sf::Keyboard::Space:
				{
					if (tunePlaying)
					{
						player.stop();
					}
					else
					{
						player.setPlayingOffset(sf::seconds(currentInstant*secondsPerInstant));
						player.play();
					}
					tunePlaying = !tunePlaying;
					break;
				}
				case sf::Keyboard::Right:
				{
					if (!tunePlaying)
					{
						currentInstant++;
					}
					break;
				}
				case sf::Keyboard::Left:
				{
					if (!tunePlaying)
					{
						currentInstant--;
					}
					break;
				}
				default:
				{
					break;
				}
				}
				if (currentInstant < 0)
				{
					currentInstant = fftResult.m_magnitudes.size()-1;
				}
				if (currentInstant >= fftResult.m_magnitudes.size())
				{
					currentInstant = 0;
				}
				std::cout << "Key pressed. Current Instant: " << currentInstant << "\n";
			}
		}
		if (tunePlaying)
		{
			currentInstant = player.getPlayingOffset().asSeconds() / secondsPerInstant;
		}

		window.clear();
		sf::Vertex verts[1024];
		float samplesCovered = fftResult.m_binCountPerInstant * 2;
		float sampleInterval = samplesCovered / 1024.f;
		int screenEdgeOffset = (1280 - 1024) / 2.f;
	
		for (int i = 0; i < 1024; i++)
		{
			
			float averageSample = 0.f;
			int sampleIndex = (i*sampleInterval + currentInstant*samplesCovered) * 2;
			if (sampleIndex < audioAnalyser.getShortSampleCount())
			{
				averageSample = audioAnalyser.getShortSamples()[sampleIndex];
			}
			verts[i] = sf::Vertex(sf::Vector2f(i + screenEdgeOffset, averageSample * 120 + 120), sf::Color(80, 80, 255));
		}
		sampleInterval = fftResult.m_binCountPerInstant / 512.f;
		
		for (int i = 0; i < 512; i++)
		{
			float averageSample = fftResult.m_magnitudes.at(currentInstant)[(int)(i*sampleInterval)];// *2.f;
			sf::RectangleShape binBar = sf::RectangleShape({ 1, averageSample + 1 });
			binBar.setFillColor({ 80, 80, 255 });
			binBar.setPosition(i*2 + screenEdgeOffset, 470);
			binBar.setOrigin(1 / 2.f, averageSample + 1);
			window.draw(binBar);
		}
		window.draw(verts, 1024, sf::LineStrip);
		window.display();
	}

	delete[] samples;
	return 0;
}