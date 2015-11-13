#ifndef DEMO2_GLELEMENTS
#define DEMO2_GLELEMENTS

#include <stdio.h>

#include "SDL.h"
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _WIN32
#include <windows.h>
#include "../GL/wglext.h"
#endif
#include "../GL/glext.h"

//Constants
///////////////////////////////////////////////////////
enum {
    HALFSIZE = sizeof(float) / 2, //sizeof(half)
};

//Global variables
///////////////////////////////////////////////////////

//Shader
GLuint shaderProgram;

//Vertex Array Object (VAO) for buffers
GLuint meshVAO;

//External buffera
//For verticies
float * extGLBufVertices;

//For colors
float * extGLBufColors;

//Internal buffers
//For verticies
GLuint intGlBufVerticies;

//For colors
GLuint intGLBufColors;

//For normals
GLuint intGLBufNormals;

//Debug defines
//////////////////////////////////////////////////////
#define CHECK_GL_ERRORS ThrowExceptionOnGLError(__LINE__,__FILE__);

GLenum g_OpenGLError;
#define OPENGL_CHECK_FOR_ERRORS() \
        if ((g_OpenGLError = glGetError()) != GL_NO_ERROR) \
                fprintf(stderr, "OpenGL error 0x%X\n", (unsigned)g_OpenGLError);



//Check functions
//////////////////////////////////////////////////////

//Check shader for errors
GLint ShaderStatus(GLuint shader, GLenum param);

//Check shader program for compilation errors
GLint ShaderProgramStatus(GLuint program, GLenum param);

//Print error string
void ThrowExceptionOnGLError(int line, const char *file);


//Other functions
///////////////////////////////////////////////////////

//Load shader
int loadSource(char * shaderName, char **textOut, int *textLen);

//Fill perspective matrix
void Matrix4Perspective(float *M, float fovy, float aspect, float znear, float zfar);

//Compilation of shader
void CompileShader(const char * name, GLuint * shader, GLenum shaderType);

//Load GL functions
///////////////////////////////////////////////////////

//Macros for loading functions
#define OPENGL_GET_PROC(p,n) \
	n = (p)SDL_GL_GetProcAddress(#n); \
	if (NULL == n) { \
			fprintf(stderr, "Loading extension '%s' fail (%d)\n", #n, GetLastError()); \
			fprintf(stderr, "%s", SDL_GetError()); \
			return 0; \
	}

//Functions types definition
typedef void   (APIENTRY * glGetShaderiv_FUNC)             (GLuint, GLenum, GLint *);
typedef void   (APIENTRY * glGetShaderInfoLog_FUNC)        (GLuint, GLsizei, GLsizei *, GLchar *);
typedef void   (APIENTRY * glGetProgramiv_FUNC)            (GLuint, GLenum, GLint *);
typedef void   (APIENTRY * glGetProgramInfoLog_FUNC)       (GLuint, GLsizei, GLsizei *, GLchar *);
typedef GLuint (APIENTRY * glCreateProgram_FUNC)           (void);
typedef GLuint (APIENTRY * glCreateShader_FUNC)            (GLenum);
typedef void   (APIENTRY * glShaderSource_FUNC)            (GLuint, GLsizei, const GLchar * const *, const GLint *);
typedef void   (APIENTRY * glCompileShader_FUNC)           (GLuint);
typedef void   (APIENTRY * glAttachShader_FUNC)            (GLuint, GLuint);
typedef void   (APIENTRY * glLinkProgram_FUNC)             (GLuint);
typedef void   (APIENTRY * glUseProgram_FUNC)              (GLuint);
typedef void   (APIENTRY * glGenVertexArrays_FUNC)         (GLsizei, GLuint *);
typedef void   (APIENTRY * glBindVertexArray_FUNC)         (GLuint);
typedef void   (APIENTRY * glGenBuffers_FUNC)              (GLsizei, GLuint);
typedef void   (APIENTRY * glBindBuffer_FUNC)              (GLenum, GLuint);
typedef void   (APIENTRY * glBufferData_FUNC)              (GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef GLint  (APIENTRY * glGetUniformLocation_FUNC)      (GLuint, const GLchar *);
typedef void   (APIENTRY * glUniformMatrix4fv_FUNC)        (GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLint  (APIENTRY * glGetAttribLocation_FUNC)       (GLuint, const GLchar *);
typedef void   (APIENTRY * glVertexAttribPointer_FUNC)     (GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *);
typedef void   (APIENTRY * glEnableVertexAttribArray_FUNC) (GLuint);

//Functions pointers definition
glGetShaderiv_FUNC             glGetShaderiv;
glGetShaderInfoLog_FUNC        glGetShaderInfoLog;
glGetProgramiv_FUNC            glGetProgramiv;
glGetProgramInfoLog_FUNC       glGetProgramInfoLog;
glCreateProgram_FUNC           glCreateProgram;
glCreateShader_FUNC            glCreateShader;
glShaderSource_FUNC            glShaderSource;
glCompileShader_FUNC           glCompileShader;
glAttachShader_FUNC            glAttachShader;
glLinkProgram_FUNC             glLinkProgram;
glUseProgram_FUNC              glUseProgram;
glGenVertexArrays_FUNC         glGenVertexArrays;
glBindVertexArray_FUNC         glBindVertexArray;
glGenBuffers_FUNC              glGenBuffers;
glBindBuffer_FUNC              glBindBuffer;
glBufferData_FUNC              glBufferData;
glGetUniformLocation_FUNC      glGetUniformLocation;
glUniformMatrix4fv_FUNC        glUniformMatrix4fv;
glGetAttribLocation_FUNC       glGetAttribLocation;
glVertexAttribPointer_FUNC     glVertexAttribPointer;
glEnableVertexAttribArray_FUNC glEnableVertexAttribArray;

#endif
