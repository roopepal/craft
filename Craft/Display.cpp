#include "pch.h"
#include "Display.hpp"
#include "Chunk.h"
#include "Noise.h"
#include "shader.hpp"
#include "lodepng.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>

void Display::make_world()
{	
	world = new SuperChunk;

	float x_factor = 1.0f / (BLOCKS_X*CHUNKS_X);
	float z_factor = 1.0f / (BLOCKS_Z*CHUNKS_Z);

	std::vector<std::vector<float>> noise_map = Noise::perlin(
		BLOCKS_X*CHUNKS_X, BLOCKS_Z*CHUNKS_Z, 1234, 1.0f, 4, 0.5f, 2, glm::vec2(0, 0));

	for (int x = 0; x < CHUNKS_X*BLOCKS_X; ++x)
	{
		for (int z = 0; z < CHUNKS_Z*BLOCKS_Z; ++z)
		{
			float noise = noise_map.at(x).at(z) * BLOCKS_Y * CHUNKS_Y;
			int xz_height = int(noise);

			int block_type;
			int below = 0;

			const int max_y = BLOCKS_Y * CHUNKS_Y;

			for (int y = 0; y < xz_height; ++y)
			{
				// rock bottom
				if (y < max_y * 0.1)
				{
					block_type = 3;
				}
				// rock almost bottom with 40% chance
				else if (y < max_y * 0.13)
				{
					if (rand() < RAND_MAX * 0.4)
					{
						block_type = 3;
					}
					else
					{
						block_type = 1;
					}
				}
				// dirt and grass
				else if (y < max_y * 0.7)
				{
					block_type = 1;
				}
				// mountain top rock
				else if (y < max_y * 0.8)
				{
					if (rand() < RAND_MAX * 0.1)
					{
						block_type = 16;
					}
					else
					{
						block_type = 3;
					}
				}
				else if (y < max_y * 0.85)
				{
					if (rand() < RAND_MAX * 0.5)
					{
						block_type = 16;
					}
					else
					{
						block_type = 3;
					}
				}
				// mountain top snow
				else
				{
					block_type = 16;
				}
				

				world->set(x, y, z, block_type);
				
				// change below grass to dirt if needed
				if (y > 0 && below == 1 && (block_type == 1 || block_type == 3))
				{
					world->set(x, y - 1, z, 2);
				}
				//*/
				below = block_type;
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.5, 0.7, 1.0, 1.0);
	glEnableVertexAttribArray(a_coord);

	while (!glfwWindowShouldClose(window))
	{
		update();
		render();
	}

	glDeleteTextures(1, &texture);
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

	load_texture_from_file("texture.png");
    
    return true;
}

void Display::load_texture_from_file(const char* path)
{
	std::vector<unsigned char> image;
	unsigned int texture_w, texture_h, lodepng_error;
	lodepng_error = lodepng::decode(image, texture_w, texture_h, path);
	if (lodepng_error)
	{
		std::cout << "decoder error " << lodepng_error << ": " << lodepng_error_text(lodepng_error) << std::endl;
	}

	// flip vertically, OpenGL expects origin bottom left
	std::vector<unsigned char> texture_image;
	for (int y = texture_h - 1; y > -1; --y)
	{
		for (int x = 0; x < texture_w; ++x)
		{
			for (int rgba = 0; rgba < 4; ++rgba)
			{
				texture_image.push_back( image.at(y * texture_w * 4 + x * 4 + rgba) );
			}
		}
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_w, texture_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texture_image[0]);
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

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(u_texture, 0);
	glBindTexture(GL_TEXTURE_2D, texture);

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

	//std::cout << "Position: " << glm::to_string(position) << std::endl;
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

	if (angles.y < -M_PI / 2 + 0.001)
	{
		angles.y = -M_PI / 2 + 0.001;
	}
	else if (angles.y > M_PI / 2 - 0.001)
	{
		angles.y = M_PI / 2 - 0.001;
	}

	look_at.x = sinf(angles.x) * cosf(angles.y);
	look_at.y = sinf(angles.y);
	look_at.z = cosf(angles.x) * cosf(angles.y);

	//std::cout << "Looking at: " << glm::to_string(look_at) << std::endl;
	
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