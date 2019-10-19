#ifndef CHUNK_H_
#define CHUNK_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Display.hpp"
#include <stdint.h>

constexpr int BLOCKS_X = 16;
constexpr int BLOCKS_Y = 16;
constexpr int BLOCKS_Z = 16;
constexpr int CHUNKS_X = 1;
constexpr int CHUNKS_Y = 1;
constexpr int CHUNKS_Z = 1;

typedef glm::tvec4<GLbyte> byte4;

struct Chunk
{
	int blocks[BLOCKS_X][BLOCKS_Y][BLOCKS_Z];
	byte4 vertices_opaque[BLOCKS_X * BLOCKS_Y * BLOCKS_Z * 6 * 6];
	byte4 vertices_transparent[BLOCKS_X * BLOCKS_Y * BLOCKS_Z * 6 * 6];
	int n_opaque;
	int n_transparent;
	GLuint vbo_opaque;
	GLuint vbo_transparent;
	bool changed;

	int get(int x, int y, int z);
	void set(int x, int y, int z, int block_type);

	void update();
	void render(Display* display);

	Chunk();
	~Chunk();
};

struct SuperChunk
{
	Chunk* c[CHUNKS_X][CHUNKS_Y][CHUNKS_Z];

	int get(int x, int y, int z);
	void set(int x, int y, int z, int block_type);

	void render(Display* display);

	SuperChunk();
	~SuperChunk();
};

#endif // !CHUNK_H_
