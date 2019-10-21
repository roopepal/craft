#pragma once
#include <vector>
#include <array>

constexpr int GRASS = 1;
constexpr int DIRT = 2;
constexpr int ROCK = 3;
constexpr int WATER = 4;
constexpr int SAND = 5;
constexpr int WOOD = 6;
constexpr int LEAF = 7;
constexpr int TNT = 8;
constexpr int GLASS = 9;
constexpr int GOLD = 10;
constexpr int DIAMOND = 11;
constexpr int REDSTONE = 12;
constexpr int BRICK = 13;
constexpr int COAL = 14;
constexpr int GRASS_TOP = 15;
constexpr int SNOW = 16;

class World
{
public:
	std::array<std::array<std::array<int, 5>, 10>, 5> tree_indices;
	std::array<std::array<std::array<int, 5>, 10>, 5> get_tree_indices();
	void prepare_tree_indices();

	World();
};

