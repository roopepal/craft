#include "pch.h"
#include "Noise.h"
#include "glm/glm.hpp"
#include "glm/gtc/noise.hpp"
#include <vector>
#include <limits>

std::vector<std::vector<float>> Noise::perlin(
	int map_width, int map_height, int seed, float scale, int octaves,
	float persistence, float lacunarity, glm::vec2 offset)
{
	std::vector<std::vector<float>> noise_map;

	std::vector<glm::vec2> octave_offsets;

	for (int i = 0; i < octaves; i++)
	{
		int rand_x = rand() % 200000 - 100000 + offset.x;
		int rand_y = rand() % 200000 - 100000 + offset.y;
		rand_x /= map_width;
		rand_y /= map_height;
		octave_offsets.push_back(glm::vec2(rand_x, rand_y));
	}

	float max_noise = FLT_MIN;
	float min_noise = FLT_MAX;
	float half_width = map_width / 2.0f;
	float half_height = map_height / 2.0f;

	for (int x = 0; x < map_width; ++x)
	{
		noise_map.push_back(std::vector<float>());

		for (int y = 0; y < map_height; ++y)
		{
			float amplitude = 1;
			float frequency = 1;
			float noise_height = 0;

			for (int i = 0; i < octaves; ++i)
			{
				float sample_x = (x - half_width) / scale * frequency + octave_offsets.at(i).x;
				float sample_y = (y - half_height) / scale * frequency + octave_offsets.at(i).y;
				sample_x /= map_width;
				sample_y /= map_height;

				float perlin_value = glm::perlin(glm::vec2(sample_x, sample_y));
				noise_height += perlin_value * amplitude;

				amplitude *= persistence;
				frequency *= lacunarity;
			}

			if (noise_height > max_noise)
			{
				max_noise = noise_height;
			}
			else if (noise_height < min_noise)
			{
				min_noise = noise_height;
			}

			noise_map.at(x).push_back(noise_height);
		}
	}

	// scale everything to [0, 1]
	for (int x = 0; x < map_width; ++x)
	{
		for (int y = 0; y < map_height; ++y)
		{
			noise_map.at(x).at(y) = (noise_map.at(x).at(y) - min_noise) / (max_noise - min_noise);
		}
	}

	return noise_map;
}
