#include "pch.h"
#include "Chunk.h"
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "Display.hpp"
#include <stdint.h>
#include <cstring>
#include <vector>
#include <iostream>

typedef glm::tvec4<GLbyte> byte4;

Chunk::Chunk()
{
	memset(blocks, 0, sizeof(blocks));
}

int Chunk::get(int x, int y, int z)
{
	return blocks[x][y][z];
}

void Chunk::set(int x, int y, int z, int block_type)
{
	blocks[x][y][z] = block_type;
}

SuperChunk::SuperChunk()
{
	memset(vertices_opaque, 0, sizeof(vertices_opaque));
	memset(vertices_transparent, 0, sizeof(vertices_transparent));
	n_opaque = 0;
	n_transparent = 0;

	glGenBuffers(1, &vbo_opaque);
	glGenBuffers(1, &vbo_transparent);

	max_x = CHUNKS_X * BLOCKS_X - 1;
	max_y = CHUNKS_Y * BLOCKS_Y - 1;
	max_z = CHUNKS_Z * BLOCKS_Z - 1;

	for (int x = 0; x < CHUNKS_X; ++x)
	{
		for (int y = 0; y < CHUNKS_Y; ++y)
		{
			for (int z = 0; z < CHUNKS_Z; ++z)
			{
				c[x][y][z] = new Chunk();
			}
		}
	}
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

	glDeleteBuffers(1, &vbo_opaque);
	glDeleteBuffers(1, &vbo_transparent);
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

	c[chunk_x][chunk_y][chunk_z]->set(x, y, z, block_type);

	changed = true;
}

void SuperChunk::update()
{
	changed = false;

	n_opaque = 0;
	n_transparent = 0;

	for (int x = 0; x < max_x; ++x)
	{
		for (int y = 0; y < max_y; ++y)
		{
			for (int z = 0; z < max_z; ++z)
			{
				int block_type = get(x, y, z);
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

				bool self_tp = block_type == 9 || block_type == 4;

				// has neighbor
				int x_next, y_next, z_next, x_prev, y_prev, z_prev;

				x_next = x == max_x - 1 ? 0 : get(x + 1, y, z);
				y_next = y == max_y - 1 ? 0 : get(x, y + 1, z);
				z_next = z == max_z - 1 ? 0 : get(x, y, z + 1);
				x_prev = x == 0 ? 0 : get(x - 1, y, z);
				y_prev = y == 0 ? 0 : get(x, y - 1, z);
				z_prev = z == 0 ? 0 : get(x, y, z - 1);

				// has transparent neighbor
				int x_next_tp = x_next == 9 || x_next == 4;
				int y_next_tp = y_next == 9 || y_next == 4;
				int z_next_tp = z_next == 9 || z_next == 4;
				int x_prev_tp = x_prev == 9 || x_prev == 4;
				int y_prev_tp = y_prev == 9 || y_prev == 4;
				int z_prev_tp = z_prev == 9 || z_prev == 4;

				byte4* vertices = self_tp ? vertices_transparent : vertices_opaque;
				long int &n_vertices = self_tp ? n_transparent : n_opaque;

				// from -x
				if ((x == 0 || !x_prev || x_prev_tp) && !(self_tp && x_prev))
				{
					vertices[n_vertices++] = byte4(x, y, z, block_type);
					vertices[n_vertices++] = byte4(x, y, z + 1, block_type);
					vertices[n_vertices++] = byte4(x, y + 1, z, block_type);
					vertices[n_vertices++] = byte4(x, y + 1, z, block_type);
					vertices[n_vertices++] = byte4(x, y, z + 1, block_type);
					vertices[n_vertices++] = byte4(x, y + 1, z + 1, block_type);
				}

				// from +x
				if ((!x_next || x_next_tp) && !(self_tp && x_next))
				{
					vertices[n_vertices++] = byte4(x + 1, y, z, block_type);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z, block_type);
					vertices[n_vertices++] = byte4(x + 1, y, z + 1, block_type);
					vertices[n_vertices++] = byte4(x + 1, y, z + 1, block_type);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z, block_type);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z + 1, block_type);
				}

				// from -y
				if ((!y_prev || y_prev_tp) && !(self_tp && y_prev))
				{
					vertices[n_vertices++] = byte4(x, y, z, block_type_bottom);
					vertices[n_vertices++] = byte4(x + 1, y, z, block_type_bottom);
					vertices[n_vertices++] = byte4(x + 1, y, z + 1, block_type_bottom);
					vertices[n_vertices++] = byte4(x + 1, y, z + 1, block_type_bottom);
					vertices[n_vertices++] = byte4(x, y, z + 1, block_type_bottom);
					vertices[n_vertices++] = byte4(x, y, z, block_type_bottom);
				}

				// from +y
				if ((!y_next || y_next_tp) && !(self_tp && y_next))
				{
					vertices[n_vertices++] = byte4(x, y + 1, z, block_type_top);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z + 1, block_type_top);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z, block_type_top);
					vertices[n_vertices++] = byte4(x, y + 1, z, block_type_top);
					vertices[n_vertices++] = byte4(x, y + 1, z + 1, block_type_top);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z + 1, block_type_top);
				}

				// from -z
				if ((!z_prev || z_prev_tp) && !(self_tp && z_prev))
				{
					vertices[n_vertices++] = byte4(x, y, z, block_type);
					vertices[n_vertices++] = byte4(x, y + 1, z, block_type);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z, block_type);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z, block_type);
					vertices[n_vertices++] = byte4(x + 1, y, z, block_type);
					vertices[n_vertices++] = byte4(x, y, z, block_type);
				}

				// from +z
				if ((!z_next || z_next_tp) && !(self_tp && z_next))
				{
					vertices[n_vertices++] = byte4(x, y, z + 1, block_type);
					vertices[n_vertices++] = byte4(x + 1, y, z + 1, block_type);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z + 1, block_type);
					vertices[n_vertices++] = byte4(x + 1, y + 1, z + 1, block_type);
					vertices[n_vertices++] = byte4(x, y + 1, z + 1, block_type);
					vertices[n_vertices++] = byte4(x, y, z + 1, block_type);
				}
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_opaque);
	glBufferData(GL_ARRAY_BUFFER, n_opaque * sizeof(byte4), vertices_opaque, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_transparent);
	glBufferData(GL_ARRAY_BUFFER, n_transparent * sizeof(byte4), vertices_transparent, GL_STATIC_DRAW);
}

void SuperChunk::render(Display* display)
{
	if (changed)
	{
		update();
	}

	display->set_model_translation(0, 0, 0);

	if (n_opaque > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo_opaque);
		glVertexAttribPointer(display->a_coord, 4, GL_BYTE, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, n_opaque);
	}
	if (n_transparent > 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo_transparent);
		glVertexAttribPointer(display->a_coord, 4, GL_BYTE, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, n_transparent);
	}
}