#include "pch.h"
#include "Display.hpp"
#include "Chunk.h"
#include "shader.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include <random>

void Display::make_world()
{	
	world = new SuperChunk;

	float x_factor = 1.0f / (BLOCKS_X*CHUNKS_X - 1);
	float z_factor = 1.0f / (BLOCKS_Z*CHUNKS_Z - 1);

	for (int x = 0; x < CHUNKS_X*BLOCKS_X; ++x)
	{
		for (int z = 0; z < CHUNKS_Z*BLOCKS_Z; ++z)
		{
			float noise = glm::perlin(glm::vec2(x * x_factor, z * z_factor));
			// scale 0 to BLOCKS_Y*CHUNKS_Y
			noise += 1.0f;
			noise /= 2.0f;
			noise *= BLOCKS_Y*CHUNKS_Y;
			int xz_height = (int)noise;

			for (int y = 0; y < xz_height; ++y)
			{
				world->set(x, y, z, rand() % 100);
			}
		}
	}
}

bool Display::setup(int argc, char* argv[])
{
	glfwSetErrorCallback(error_callback_wrapper);

	if (!glfwInit())
	{
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window)
	{
		std::cerr << "Error:\nGLFW window creation failed." << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback_wrapper);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cerr << "Error:\n" << glewGetErrorString(err) << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return false;
	}

    if (!setup_program()) {
		std::cerr << "Program setup failed." << std::endl;
        return false;
    }

	make_world();

	std::cout << "Setup successful." << std::endl;
	return true;
}

void Display::start()
{
	// settings that do not change while running
	glUseProgram(program);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.7, 1.0, 0.0);
	glEnableVertexAttribArray(a_coord);

	while (!glfwWindowShouldClose(window))
	{
		render();
	}

	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Display::setup_program()
{
    if ((vs = create_shader("vertex.glsl", GL_VERTEX_SHADER)) == 0) {
        return false;
    }
    if ((fs = create_shader("fragment.glsl", GL_FRAGMENT_SHADER)) == 0) {
        return false;
    }
    
    GLint link_status = GL_FALSE;
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        print_info_log(program);
        return false;
    }

    a_coord = glGetAttribLocation(program, "a_coord");
    if (a_coord == -1) {
        std::cerr << "Could not bind a_coord" << std::endl;
    }
    u_mvp = glGetUniformLocation(program, "u_mvp");
    if (u_mvp == -1) {
        std::cerr << "Could not bind u_mvp" << std::endl;
        return false;
    }
    
    return true;
}

void Display::update()
{
	
}

void Display::set_model_translation(int x, int y, int z)
{
	model = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
	view = glm::lookAt(glm::vec3(BLOCKS_X*CHUNKS_X / 2.0, BLOCKS_Y*CHUNKS_Y * 2.0, BLOCKS_Z*CHUNKS_Z * 2.0), glm::vec3(BLOCKS_X*CHUNKS_X / 2.0, BLOCKS_Y*CHUNKS_Y, BLOCKS_Z*CHUNKS_Z / 2.0), glm::vec3(0, 1, 0));
	projection = glm::perspective(45.0, 1.0 * width / height, 0.001, 1000.0);
	mvp = projection * view * model;
	glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
}

void Display::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	world->render(this);
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Display::error_callback(int error, const char* description)
{
	std::cerr << "Error:\n" << description << std::endl;
}

void Display::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

// Cannot give member functions as callbacks, wrap them
Display* Display::instance = NULL;

void Display::set_instance() {
	instance = this;
}

void Display::render_wrapper() {
	instance->render();
}

void Display::update_wrapper() {
	instance->update();
}

void Display::error_callback_wrapper(int error, const char* description) {
	instance->error_callback(error, description);
}

void Display::key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods) {
	instance->key_callback(window, key, scancode, action, mods);
}