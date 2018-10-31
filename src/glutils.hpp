#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <GL/glew.h>

namespace GLUtils {
int checkForOpenGLError(const char *, int);

void dumpGLInfo(bool dumpExtensions = false);

void debugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * msg, const void * param );
}

#endif // GLUTILS_HPP
