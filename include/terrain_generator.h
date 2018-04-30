#ifndef TERRAIN_GENERATOR_H
#define TERRAIN_GENERATOR_H

#include "generator/map_generator.h"
#include "parameters/parameter_loader.h"
#include "generator/color.h"
#include "generator/gradient.h"
#include "factory/fastnoise_factory.h"
#include "utils/utils.h"

#include <FastNoise.h>

#include <map>
#include <string>
#include <cmath>


class TerrainGenerator : public MapGenerator
{
public:
	enum Layers
	{
		Land,
		Elevation,
		Moisture,
		Temperature,
		BiomeMap,
		Final
	};

	enum class Biome
	{
		Tundra,
		BorealForest,
		BorealBarren,
		TemperatelForest,
		Savanna,
		GrassLand,
		Desert,
		Rainforest
	};

	TerrainGenerator()
	{

	}

	virtual void generate(GeneratorBufferList& buffers)
	{
		const auto startX = buffers[0].startX();
		const auto startY = buffers[0].startY();
		const auto endX = buffers[0].endX();
		const auto endY = buffers[0].endY();
		const auto height = buffers[0].getBuffer().getHeight();

		FastNoise whitenoise;
		whitenoise.SetNoiseType(FastNoise::WhiteNoise);

		for (int x = startX; x < endX; ++x)
		{
			for (int y = startY; y < endY; ++y)
			{
				auto sampleX = (float)x / scale_;
				auto sampleY = (float)y / scale_;


				auto land_mask = land_mask_.GetNoise(sampleX, sampleY);

				auto is_land = (land_mask > sea_level_);

				Color cl;
				Color ce;
				Color cm;
				Color ct;
				Color cb;
				Color cf;

				if (is_land)
				{
					cl = Color::from(land_dark_, land_light_, land_mask);

					auto elevation = utils::range(elevation_.GetNoise(sampleX, sampleY), -1.f, 1.f, 0.f, 1.f);
					ce = Color::from(Color(0), Color(1), elevation);

					auto moisture = utils::range(moisture_.GetNoise(sampleX, sampleY), -1.f, 1.f, 0.f, 1.f);
					cm = Color::from(water_light_, water_dark_, moisture);

					auto half = (float)height / 2.0f;
					auto lat_temperature = 1.f - (std::abs(sampleY - half) / half);
					auto temperature = (1.f - elevation) + (std::pow(lat_temperature, lat_falloff_));
					ct = Color::from(cold_, hot_, temperature / 2.f);

					auto biome = getBiome(temperature, moisture);
					cb = colorFromBiome(biome);

					// final color. TODO: replace with gradients
					cf = Color::from(cb, land_dark_, elevation);

					if (elevation > mountain_height_)
					{
						cf = Color::from(stone_dark_, stone_light_, elevation);
					}

					else if (hasTrees(biome))
					{
						auto tree_value = utils::range(whitenoise.GetNoise(sampleX, sampleY), -1.f, 1.f, 0.f, 1.f);

						if (tree_value <= tree_percent_)
						{
							cf = tree_;
						}
					}
				}
				else
				{
					cl = Color::from(water_light_, water_dark_, 1.0f - land_mask);
					cf = Color::from(water_light_, water_dark_, 1.0f - land_mask);
				}

				buffers[Land].write(x, y, cl);
				buffers[Elevation].write(x, y, ce);
				buffers[Moisture].write(x, y, cm);
				buffers[Temperature].write(x, y, ct);
				buffers[BiomeMap].write(x, y, cb);
				buffers[Final].write(x, y, cf);
			}
		}
	}
	
	bool hasTrees(Biome b)
	{
		return b == Biome::BorealForest || b == Biome::TemperatelForest;
	}

	Biome getBiome(float temperature, float moisture)
	{
		// tropical
		if (temperature >= t1_)
		{
			if (moisture >= m1_)
			{
				return Biome::Rainforest;
			}
			else if (moisture >= m2_)
			{
				return Biome::TemperatelForest;
			}
			else if (moisture >= m3_)
			{
				return Biome::GrassLand;
			}
			else
			{
				return Biome::Desert;
			}
		}
		// temperate
		else if (temperature >= t2_)
		{
			if (moisture >= m1_)
			{
				return Biome::TemperatelForest;
			}
			else if (moisture >= m2_)
			{
				return Biome::GrassLand;
			}
			else if (moisture >= m3_)
			{
				return Biome::GrassLand;
			}
			else
			{
				return Biome::Desert;
			}
		}
		// subarctic
		else if (temperature >= t3_)
		{
			if (moisture >= m2_)
			{
				return Biome::BorealForest;
			}
			else
			{
				return Biome::BorealBarren;
			}
		}
		// arctic
		else
		{
			return Biome::Tundra;
		}

		return Biome::GrassLand;
	}

	Color colorFromBiome(Biome b)
	{
		switch (b)
		{
		case TerrainGenerator::Biome::Tundra:
			return tundra_;
		case TerrainGenerator::Biome::BorealForest:
			return boreal_;
		case TerrainGenerator::Biome::BorealBarren:
			return boreal_barren_;
		case TerrainGenerator::Biome::TemperatelForest:
			return temperate_;
		case TerrainGenerator::Biome::Savanna:
			return savanna_;
		case TerrainGenerator::Biome::GrassLand:
			return grassland_;
		case TerrainGenerator::Biome::Desert:
			return desert_;
		case TerrainGenerator::Biome::Rainforest:
			return rainforest_;
		default:
			return grassland_;
		}
	}

	virtual void loadParams(ParameterLoader::GeneratorParameters& params) override
	{
		land_mask_ = FastNoiseFactory::create(getNoise(params, "land_mask"));
		moisture_ = FastNoiseFactory::create(getNoise(params, "moisture"));
		elevation_ = FastNoiseFactory::create(getNoise(params, "elevation"));

		land_light_ = Color::from(getColor(params, "land_light"));
		land_dark_ = Color::from(getColor(params, "land_dark"));
		water_light_ = Color::from(getColor(params, "water_light"));
		water_dark_ = Color::from(getColor(params, "water_dark"));
		grass_ = Color::from(getColor(params, "grass"));
		hot_ = Color::from(getColor(params, "hot"));
		cold_ = Color::from(getColor(params, "cold"));

		tundra_ = Color::from(getColor(params, "tundra"));
		boreal_ = Color::from(getColor(params, "boreal"));
		boreal_barren_ = Color::from(getColor(params, "boreal_barren"));
		temperate_ = Color::from(getColor(params, "temperate"));
		savanna_ = Color::from(getColor(params, "savanna"));
		grassland_ = Color::from(getColor(params, "grassland"));
		desert_ = Color::from(getColor(params, "desert"));
		rainforest_ = Color::from(getColor(params, "rainforest"));
		tree_ = Color::from(getColor(params, "tree"));
		stone_light_ = Color::from(getColor(params, "stone_light"));
		stone_dark_ = Color::from(getColor(params, "stone_dark"));


		sea_level_ = getFloat(params, "sea_level");
		scale_ = getFloat(params, "scale");
		tree_percent_ = getFloat(params, "tree_percent");
		mountain_height_ = getFloat(params, "mountain_height");

		t1_ = getFloat(params, "t1");
		t2_ = getFloat(params, "t2");
		t3_ = getFloat(params, "t3");

		m1_ = getFloat(params, "m1");
		m2_ = getFloat(params, "m2");
		m3_ = getFloat(params, "m3");

		lat_falloff_ = getFloat(params, "lat_falloff");
	}

	virtual std::string getName() const
	{
		return "Terrain";
	}

	virtual std::vector<std::string> getLayerNames() const
	{
		return { "Land", "Elevation", "Moisture", "Temperature", "Biomes", "Final" };
	}
private:
	FastNoise land_mask_;
	FastNoise elevation_;
	FastNoise moisture_;

	Color land_light_, land_dark_;
	Color water_light_, water_dark_;
	Color grass_;
	Color hot_;
	Color cold_;

	Color tundra_;
	Color boreal_;
	Color boreal_barren_;
	Color temperate_;
	Color savanna_;
	Color grassland_;
	Color desert_;
	Color rainforest_;
	Color tree_;
	Color stone_light_;
	Color stone_dark_;

	float sea_level_;
	float scale_;
	float tree_percent_;
	float mountain_height_;

	float t1_; // tropical
	float t2_; // temperate
	float t3_; // subarctic
	//float t4_; // arctic
	float m1_;
	float m2_;
	float m3_;
	float m4_;



	float lat_falloff_;
};

#endif // TERRAIN_GENERATOR_H