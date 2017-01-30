#include "OutlierDetector.h"
#include <iostream>

OutlierDetector::OutlierDetector()
{
	m_history = nullptr;
}

OutlierDetector::~OutlierDetector()
{
	if (m_history)
	{
		delete m_history;
		m_history = nullptr;
	}
}

void OutlierDetector::detectOutliers(const real * _samples, unsigned int _sampleCount, unsigned int _instantSize, unsigned int _recordedHistory)
{
	real sensitivity = 0.1f;	//temp for now should adapt on its own later

	if (m_history)
	{
		delete m_history;
		m_history = nullptr;
	}
	unsigned int instantCount = _sampleCount / _instantSize;
	if ((float)_sampleCount / _instantSize != (float)((int)(_sampleCount / _instantSize)))
	{
		instantCount++;
	}

	m_history = new LoopedList<real>(_recordedHistory);

	//for each instant
	for (unsigned int i = 0; i < instantCount; i++)
	{ 
		real instantAverage = 0;
		//for each sample in instant
		unsigned int samplesCovered = 0;
		for (unsigned int s = 0; s < _instantSize; s++)
		{
			real sample = 0;
			unsigned int index = i * _instantSize + s;
			if (index < _sampleCount)
			{
				sample = _samples[index];
				samplesCovered++;
				if (i == 28)
				{
					//std::cout << "#" << s << ": " << sample << "\n";
				}
			}
			instantAverage += fabs(sample);
		}
		instantAverage /= (real)samplesCovered;
		real historyAverage = 0;
		int historyAveragesCovered = 0;
		unsigned int historySize = m_history->getSize();
		for (int h = 0; h < historySize; h++)
		{
			real curHistoryAverage = m_history->getRelative(-1 - h);
			//if (curHistoryAverage)
			//{
				historyAverage += curHistoryAverage;
				historyAveragesCovered++;
			//}
		}
		if (historyAveragesCovered > 0)
		{
			historyAverage /= historyAveragesCovered;
		}

		if (i < 10)
		{
			std::cout << "Instant #" << i << ": " << instantAverage << ", " << historyAverage << "\n";
		}
		if (i == 28)
		{
			std::cout << "Instant #" << i << ": " << instantAverage << ", " << historyAverage << "\n";
		}
		//if (instantAverage * instantAverage + historyAverage * historyAverage >= sensitivity * sensitivity)
		if(fabs(instantAverage) - fabs(historyAverage) >= sensitivity)
		{
			m_outlierIndexes.push_back(i);
		}

		m_history->add(instantAverage);
	}
}

const std::vector<unsigned int>& OutlierDetector::getOutlierIndexes()
{
	return m_outlierIndexes;
}
