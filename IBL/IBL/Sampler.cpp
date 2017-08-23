#include "Sampler.h"


Sampler::Sampler(int numSamples)
{
	samples.resize(numSamples); // equal to initialize vector???
	GenerateSamples(numSamples);
}

Sampler::~Sampler()
{
	samples.clear();
	samples.shrink_to_fit();
}
