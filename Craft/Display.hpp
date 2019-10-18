#ifndef display_hpp
#define display_hpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

struct SuperChunk;

class Display {
private:
	static Display* instance;
    int width;
    int height;
    GLFWwindow* window;
    const char* title;

	int frame_count = 0;
	double previous_time_fps;

    GLuint program;
	GLuint texture;
    GLint vs;
    GLint fs;

	double cursor_x;
	double cursor_y;

	float speed = 10.0;
	double previous_time_move = 0;

	int move = 0;
	const int left = 1;
	const int right = 2;
	const int forward = 4;
	const int backward = 8;

	glm::vec3 position;
	glm::vec2 angles;
	glm::vec3 look_at; // direction
    glm::mat4 model;
	glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 mvp;

    void render(void);
    void update(void);
	void error_callback(int error, const char* description);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void cursor_position(GLFWwindow* window, double x, double y);
	// Cannot give member functions as callbacks, wrap them
	static void render_wrapper(void);
	static void update_wrapper(void);
	static void error_callback_wrapper(int error, const char* description);
	static void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursor_position_wrapper(GLFWwindow* window, double x, double y);

	void load_texture_from_file(const char* path);
	void set_instance();
    bool setup_program();
	void show_fps();

	// TODO: move to a World class
	SuperChunk* world;
	void make_world();

public:
	GLint a_coord;
	GLint u_mvp;
	GLint u_texture;

	void set_model_translation(int x, int y, int z);
    bool setup(int argc, char* argv[]);
	void start();
    
    Display(const char* title, int width, int height)
        : title(title)
        , width(width)
        , height(height)
    {} 
};

#endif /* display_hpp */
