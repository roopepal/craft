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

    GLuint program;
    GLint vs;
    GLint fs;

    glm::mat4 model;
	glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 mvp;

    void render(void);
    void update(void);
	void error_callback(int error, const char* description);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    // Cannot give member functions as callbacks, wrap them
	static void render_wrapper(void);
	static void update_wrapper(void);
	static void error_callback_wrapper(int error, const char* description);
	static void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
	
	void set_instance();
    bool setup_program();

	// TODO: move to a World class
	SuperChunk* world;
	void make_world();

public:
	GLint a_coord;
	GLint u_mvp;

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
