#ifndef CHUNK_H_
#define CHUNK_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Display.hpp"
#include <stdint.h>

constexpr int BLOCKS_X = 16;
constexpr int BLOCKS_Y = 16;
constexpr int BLOCKS_Z = 16;
constexpr int CHUNKS_X = 8;
constexpr int CHUNKS_Y = 3;
constexpr int CHUNKS_Z = 8;

typedef glm::tvec4<GLbyte> byte4;

struct SuperChunk;

struct Chunk
{
	int blocks[BLOCKS_X][BLOCKS_Y][BLOCKS_Z];

	int get(int x, int y, int z);
	void set(int x, int y, int z, int block_type);

	Chunk();
};

struct SuperChunk
{
	Chunk* c[CHUNKS_X][CHUNKS_Y][CHUNKS_Z];

	int max_x;
	int max_y;
	int max_z;

	byte4 vertices_opaque[BLOCKS_X * CHUNKS_X * BLOCKS_Y * CHUNKS_Y * BLOCKS_Z * CHUNKS_Z * 6 * 6];
	byte4 vertices_transparent[BLOCKS_X * BLOCKS_Y * BLOCKS_Z * 6 * 6];
	long int n_opaque;
	long int n_transparent;
	GLuint vbo_opaque;
	GLuint vbo_transparent;

	bool changed;

	int get(int x, int y, int z);
	void set(int x, int y, int z, int block_type);

	void update();
	void render(Display* display);

	SuperChunk();
	~SuperChunk();
};

#endif // !CHUNK_H_
