#include "pch.h"
#include "World.h"
#include <vector>

void World::prepare_tree_indices()
{
	int trunk = 6;
	int leaf = 7;

	for (int x = 0; x < 5; ++x)
	{
		for (int y = 0; y < 10; ++y)
		{
			tree_indices[x][y].fill(0);
		}
	}

	for (int y = 0; y < 9; y++)
	{
		tree_indices[2][y][2] = trunk;
	}

	for (int y = 3; y < 10; y++)
	{
		if (y < 9)
		{
			tree_indices[1][y][1] = leaf;
			tree_indices[1][y][3] = leaf;
			tree_indices[3][y][1] = leaf;
			tree_indices[3][y][3] = leaf;
		}

		tree_indices[2][y][1] = leaf;
		tree_indices[2][y][3] = leaf;
		tree_indices[1][y][2] = leaf;
		tree_indices[3][y][2] = leaf;
	}

	for (int y = 4; y < 9; y++)
	{
		if (y != 6 && y != leaf)
		{
			if (rand() < RAND_MAX * 0.5) tree_indices[1][y][0] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[2][y][0] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[3][y][0] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[1][y][4] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[2][y][4] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[3][y][4] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[0][y][1] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[0][y][2] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[0][y][3] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[4][y][1] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[4][y][2] = leaf;
			if (rand() < RAND_MAX * 0.5) tree_indices[4][y][3] = leaf;
		}	
	}

	tree_indices[2][9][2] = leaf;
}

std::array<std::array<std::array<int, 5>, 10>, 5> World::get_tree_indices()
{
	return tree_indices;
}

World::World()
{
	prepare_tree_indices();
}
