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
#define _USE_MATH_DEFINES
#include <math.h>

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
	set_instance();

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

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(window, &cursor_x, &cursor_y);
	glfwSetCursorPosCallback(window, cursor_position_wrapper);

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

	previous_time_fps = glfwGetTime();

	position = glm::vec3(BLOCKS_X*CHUNKS_X / 2.0, BLOCKS_Y*CHUNKS_Y, BLOCKS_Z*CHUNKS_Z / 2.0);

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
		update();
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

void Display::set_model_translation(int x, int y, int z)
{
	model = glm::translate(glm::mat4(1), glm::vec3(x, y, z));
	view = glm::lookAt(position, position + look_at, glm::vec3(0, 1, 0));
	projection = glm::perspective(glm::radians(60.0), 1.0 * width / height, 0.001, 1000.0);
	mvp = projection * view * model;
	glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
}

void Display::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	world->render(this);
	glfwSwapBuffers(window);
	glfwPollEvents();

	double current_time = glfwGetTime();
	frame_count++;
	if (current_time - previous_time_fps >= 1.0)
	{
		show_fps();
		frame_count = 0;
		previous_time_fps = current_time;
	}
}

void Display::update()
{
	double current_time = glfwGetTime();
	float d_time = current_time - previous_time_move;
	previous_time_move = current_time;

	float distance = speed * d_time;

	glm::vec3 right_dir = glm::vec3(-look_at.z, 0, look_at.x);

	if (move & left)
	{
		position -= right_dir * distance;
	}
	if (move & right)
	{
		position += right_dir * distance;
	}
	if (move & forward)
	{
		position += look_at * distance;
	}
	if (move & backward)
	{
		position -= look_at * distance;
	}

	render();
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
	else if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		move |= left;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		move |= right;
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		move |= forward;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		move |= backward;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		move &= ~left;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		move &= ~right;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		move &= ~forward;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		move &= ~backward;
	}
}

void Display::show_fps()
{
	std::string new_title = "FPS: ";
	new_title.append(std::to_string(frame_count));
	glfwSetWindowTitle(window, new_title.c_str());
}

void Display::cursor_position(GLFWwindow* window, double x, double y)
{
	double d_x = x - cursor_x;
	double d_y = y - cursor_y;
	const float sensitivity = 0.001;

	angles.x -= d_x * sensitivity;
	angles.y -= d_y * sensitivity;

	if (angles.x < -M_PI)
	{
		angles.x += M_PI * 2;
	}
	else if (angles.x > -M_PI)
	{
		angles.x -= M_PI * 2;
	}

	if (angles.y < -M_PI / 2)
	{
		angles.y = -M_PI / 2;
	}
	else if (angles.y > M_PI / 2)
	{
		angles.y = M_PI / 2;
	}

	look_at.x = sinf(angles.x) * cosf(angles.y);
	look_at.y = sinf(angles.y);
	look_at.z = cosf(angles.x) * cosf(angles.y);

	cursor_x = x;
	cursor_y = y;
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

void Display::cursor_position_wrapper(GLFWwindow* window, double x, double y)
{
	instance->cursor_position(window, x, y);
}