#ifndef FASTNOISE_FACTORY_H
#define FASTNOISE_FACTORY_H

#include <FastNoise.h>
#include "parameters/noise_parameters.h"

class FastNoiseFactory
{
public:
	static FastNoise create(const NoiseParameters& params)
	{
		FastNoise noise;
		noise.SetNoiseType(static_cast<FastNoise::NoiseType>(params.noise_type));
		noise.SetFractalType(static_cast<FastNoise::FractalType>(params.fractal_type));
		noise.SetInterp(static_cast<FastNoise::Interp>(params.interp_type));
		noise.SetSeed(params.seed);
		noise.SetFractalOctaves(params.octaves);
		noise.SetFractalGain(params.gain);
		noise.SetFractalLacunarity(params.lacunarity);

		return noise;
	}
};

#endif // FASTNOISE_FACTORY_H