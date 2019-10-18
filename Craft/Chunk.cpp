#include "pch.h"
#include "Chunk.h"
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "Display.hpp"
#include <stdint.h>
#include <cstring>

typedef glm::tvec4<GLbyte> byte4;

Chunk::Chunk()
{
	memset(blocks, 0, sizeof(blocks));
	n_vertices = 0;
	changed = true;
	glGenBuffers(1, &vbo);
}

Chunk::~Chunk()
{
	glDeleteBuffers(1, &vbo);
}

int Chunk::get(int x, int y, int z)
{
	return blocks[x][y][y];
}

void Chunk::set(int x, int y, int z, int block_type)
{
	blocks[x][y][z] = block_type;
	changed = true;
}

void Chunk::update()
{
	changed = false;

	byte4 vertices[BLOCKS_X * BLOCKS_Y * BLOCKS_Z * 6 * 6];
	int i = 0;

	for (int x = 0; x < BLOCKS_X; ++x)
	{
		for (int y = 0; y < BLOCKS_Y; ++y)
		{
			for (int z = 0; z < BLOCKS_Z; ++z)
			{
				int block_type = blocks[x][y][z];
				int block_type_top = -block_type;
				int block_type_bottom = -block_type;
				
				// grass top and bottom
				if (block_type == 1)
				{
					block_type_top = -15;
					block_type_bottom = -2;
				}

				if (!block_type)
				{
					continue;
				}

				// from -x
				if (x == 0 || (x > 0 && !blocks[x - 1][y][z]))
				{
					vertices[i++] = byte4(x, y, z, block_type);
					vertices[i++] = byte4(x, y, z + 1, block_type);
					vertices[i++] = byte4(x, y + 1, z, block_type);
					vertices[i++] = byte4(x, y + 1, z, block_type);
					vertices[i++] = byte4(x, y, z + 1, block_type);
					vertices[i++] = byte4(x, y + 1, z + 1, block_type);
				}

				// from +x
				if (x == BLOCKS_X - 1 || (x < BLOCKS_X - 1 && !blocks[x + 1][y][z]))
				{
					vertices[i++] = byte4(x + 1, y, z, block_type);
					vertices[i++] = byte4(x + 1, y + 1, z, block_type);
					vertices[i++] = byte4(x + 1, y, z + 1, block_type);
					vertices[i++] = byte4(x + 1, y, z + 1, block_type);
					vertices[i++] = byte4(x + 1, y + 1, z, block_type);
					vertices[i++] = byte4(x + 1, y + 1, z + 1, block_type);
				}
				
				// from -y
				if (y == 0 || (y > 0 && !blocks[x][y - 1][z]))
				{
					vertices[i++] = byte4(x, y, z, block_type_bottom);
					vertices[i++] = byte4(x + 1, y, z, block_type_bottom);
					vertices[i++] = byte4(x + 1, y, z + 1, block_type_bottom);
					vertices[i++] = byte4(x + 1, y, z + 1, block_type_bottom);
					vertices[i++] = byte4(x, y, z + 1, block_type_bottom);
					vertices[i++] = byte4(x, y, z, block_type_bottom);
				}
				

				// from +y
				if (y == BLOCKS_Y - 1 || (y < BLOCKS_Y - 1 && !blocks[x][y + 1][z]))
				{
					vertices[i++] = byte4(x, y + 1, z, block_type_top);
					vertices[i++] = byte4(x + 1, y + 1, z + 1, block_type_top);
					vertices[i++] = byte4(x + 1, y + 1, z, block_type_top);
					vertices[i++] = byte4(x, y + 1, z, block_type_top);
					vertices[i++] = byte4(x, y + 1, z + 1, block_type_top);
					vertices[i++] = byte4(x + 1, y + 1, z + 1, block_type_top);
				}

				// from -z
				if (z == 0 || (z > 0 && !blocks[x][y][z - 1]))
				{
					vertices[i++] = byte4(x, y, z, block_type);
					vertices[i++] = byte4(x, y + 1, z, block_type);
					vertices[i++] = byte4(x + 1, y + 1, z, block_type);
					vertices[i++] = byte4(x + 1, y + 1, z, block_type);
					vertices[i++] = byte4(x + 1, y, z, block_type);
					vertices[i++] = byte4(x, y, z, block_type);
				}
				

				// from +z
				if (z == BLOCKS_Z - 1 || (z < BLOCKS_Z - 1 && !blocks[x][y][z + 1]))
				{
					vertices[i++] = byte4(x, y, z + 1, block_type);
					vertices[i++] = byte4(x + 1, y, z + 1, block_type);
					vertices[i++] = byte4(x + 1, y + 1, z + 1, block_type);
					vertices[i++] = byte4(x + 1, y + 1, z + 1, block_type);
					vertices[i++] = byte4(x, y + 1, z + 1, block_type);
					vertices[i++] = byte4(x, y, z + 1, block_type);
				}
			}
		}
	}

	n_vertices = i;
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, n_vertices * sizeof *vertices, vertices, GL_STATIC_DRAW);
}

void Chunk::render(Display* display)
{
	if (changed)
	{
		update();
	}

	if (!n_vertices)
	{
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(display->a_coord, 4, GL_BYTE, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, n_vertices);
}

SuperChunk::SuperChunk()
{
	memset(c, 0, sizeof c);
}

SuperChunk::~SuperChunk()
{
	for (int x = 0; x < CHUNKS_X; ++x)
	{
		for (int y = 0; y < CHUNKS_Y; ++y)
		{
			for (int z = 0; z < CHUNKS_Z; ++z)
			{
				delete c[x][y][z];
			}
		}
	}
}

int SuperChunk::get(int x, int y, int z)
{
	int chunk_x = x / BLOCKS_X;
	int chunk_y = y / BLOCKS_Y;
	int chunk_z = z / BLOCKS_Z;

	x %= BLOCKS_X;
	y %= BLOCKS_Y;
	z %= BLOCKS_Z;

	if (!c[chunk_x][chunk_y][chunk_z])
	{
		return 0;
	}
	else
	{
		return c[chunk_x][chunk_y][chunk_z]->get(x, y, z);
	}
}

void SuperChunk::set(int x, int y, int z, int block_type)
{
	int chunk_x = x / BLOCKS_X;
	int chunk_y = y / BLOCKS_Y;
	int chunk_z = z / BLOCKS_Z;

	x %= BLOCKS_X;
	y %= BLOCKS_Y;
	z %= BLOCKS_Z;

	if (!c[chunk_x][chunk_y][chunk_z])
	{
		c[chunk_x][chunk_y][chunk_z] = new Chunk();
	}

	c[chunk_x][chunk_y][chunk_z]->set(x, y, z, block_type);
}

void SuperChunk::render(Display* display)
{
	for (int x = 0; x < CHUNKS_X; ++x)
	{
		for (int y = 0; y < CHUNKS_Y; ++y)
		{
			for (int z = 0; z < CHUNKS_Z; ++z)
			{
				if (c[x][y][z])
				{
					display->set_model_translation(x*BLOCKS_X, y*BLOCKS_Y, z*BLOCKS_Z);
					c[x][y][z]->render(display);
				}
			}
		}
	}
}