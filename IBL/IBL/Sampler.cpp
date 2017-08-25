#include "Sampler.h"


Sampler::Sampler(int numSamples)
{
	samples.resize(numSamples); // equal to initialize vector???
	int sqrtNumSamples = sqrt(numSamples);
	GenerateSamples(sqrtNumSamples);
}

Sampler::~Sampler()
{
	samples.clear();
	samples.shrink_to_fit();
}
