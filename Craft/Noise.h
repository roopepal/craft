#pragma once
#include "glm/glm.hpp"
#include <vector>

class Noise
{
public:
	static std::vector<std::vector<float>> perlin(
		int map_width, int map_height, int seed, float scale, int octaves,
		float persistence, float lacunarity, glm::vec2 offset);
};
