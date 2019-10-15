#ifndef shader_hpp
#define shader_hpp

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

std::string read_file(std::string path);
void print_info_log(GLuint object);
GLuint create_shader(std::string path, GLenum type);

#endif