#ifndef DEMO2_GLELEMENTS
#define DEMO2_GLELEMENTS


#include <stdio.h>

#include "types.h"
//#include "SDL/SDL.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>

//Constants
///////////////////////////////////////////////////////
enum {
    HALFSIZE = sizeof(float) / 2, //sizeof(half)
};

//Global variables
///////////////////////////////////////////////////////

//Shader
GLuint shaderProgram;
GLuint shadowProgram;
GLuint SSAOProgram;

//Vertex Array Object (VAO) for buffers
GLuint meshVAO;

//External buffera
//For verticies
float * extGLBufVertices;

//For colors
float * extGLBufAmbCol;
float * extGLBufDifCol;
float * extGLBufSpcCol;

//For normals
char * extGLBufNormInds;

//Internal buffers
//For verticies
GLuint intGlBufVerticies;

//For normals
GLuint intGLBufNormInds;

//For indirect illuminate
GLuint intGLRadiosity;

//Shadow map
GLuint shadowMap;
GLuint SSAOtex;

//Frame buffer object
GLuint fbo;
GLuint SSAOfbo;

//Matrix for object
float objectMatrix[16];

//Matrix for light
float lightMatrix[16];

//Matrix for camera
float camMatrix[16];

//Debug defines
//////////////////////////////////////////////////////
#define CHECK_GL_ERRORS ThrowExceptionOnGLError(__LINE__,__FILE__);

GLenum g_OpenGLError;
#define OPENGL_CHECK_FOR_ERRORS() \
        if ((g_OpenGLError = glGetError()) != GL_NO_ERROR) { \
                fprintf(stderr, "OpenGL error 0x%X\n", (unsigned)g_OpenGLError); \
                exit(1); \
        }



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
int LoadSource(char * shaderName, char **textOut, int *textLen);

//Fill perspective matrix
void Matrix4Perspective(float *M, float fovy, float aspect, float znear, float zfar);

//Fill shift matrix
void Matrix4Shift(float *M, vec3 shift);

//Fill rotate matrix
void Matrix4Rotate(float *M, vec3 a);

//Compilation of shader
void CompileShader(const char * name, GLuint * shader, GLenum shaderType);

//Pass uniform to shader
void SetUniform4f(GLuint prog, const char * name, vec4 v);
void SetUniform3f(GLuint prog, const char * name, vec3 v);
void SetUniform1f(GLuint prog, const char * name, float v);

void SetUniform3fv(GLuint prog, const char * name, int count, float * v);

void SetUniformMat4f(GLuint prog, const char * name, float * v);

void SetUniform1fInd(GLuint prog, const char * name, int ind, float v);
void SetUniform4fInd(GLuint prog, const char * name, int ind, vec4 v);

void SetUniform1i(GLuint prog, const char * name, int v);

//Load GL functions
///////////////////////////////////////////////////////

//Macros for loading functions
#define OPENGL_GET_PROC(p,n) \
	n = (p)SDL_GL_GetProcAddress(#n); \
	if (NULL == n) { \
				fprintf(stderr, "Loading extension '%s' fail\n", #n); \
			fprintf(stderr, "%s", SDL_GetError()); \
			exit(1); \
	}

//fprintf(stderr, "Loading extension '%s' fail (%d)\n", #n, GetLastError()); \

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
typedef void   (APIENTRY * glVertexAttribIPointer_FUNC)    (GLuint, GLint, GLenum, GLsizei, const GLvoid *);
typedef void   (APIENTRY * glEnableVertexAttribArray_FUNC) (GLuint);
typedef GLint  (APIENTRY * glUniform4fv_FUNC)              (GLint, GLsizei, const GLfloat *);
typedef GLint  (APIENTRY * glUniform3fv_FUNC)              (GLint, GLsizei, const GLfloat *);
typedef GLint  (APIENTRY * glUniform1f_FUNC)               (GLint, GLfloat);
typedef GLint  (APIENTRY * glUniform1i_FUNC)               (GLint, GLint);
typedef void   (APIENTRY * glGenFramebuffers_FUNC)         (GLsizei, GLuint *);
typedef void   (APIENTRY * glBindFramebuffer_FUNC)         (GLenum, GLuint);
typedef void   (APIENTRY * glFramebufferTexture2D_FUNC)    (GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (APIENTRY * glCheckFramebufferStatus_FUNC)  (GLenum);
typedef void   (APIENTRY * glDeleteFramebuffers_FUNC)      (GLsizei, GLuint *);
typedef void   (APIENTRY * glGenRenderbuffers_FUNC)        (GLsizei, GLuint *);
typedef void   (APIENTRY * glBindRenderbuffer_FUNC)        (GLenum, GLuint);
typedef void   (APIENTRY * glRenderbufferStorage_FUNC)     (GLenum, GLenum, GLsizei, GLsizei);
typedef void   (APIENTRY * glFramebufferRenderbuffer_FUNC) (GLenum, GLenum, GLenum, GLuint);
typedef void   (APIENTRY * glDeleteBuffers_FUNC)           (GLsizei, const GLuint *);
typedef void   (APIENTRY * glDeleteProgram_FUNC)           (GLuint);
typedef void   (APIENTRY * glDeleteVertexArrays_FUNC)      (GLsizei, const GLuint *);

#ifdef _WIN32
	typedef void   (APIENTRY * glActiveTexture_FUNC)           (GLenum);
#endif

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
glUniform4fv_FUNC              glUniform4fv;
glUniform3fv_FUNC              glUniform3fv;
glUniform1f_FUNC               glUniform1f;
glUniform1i_FUNC               glUniform1i;
glVertexAttribIPointer_FUNC    glVertexAttribIPointer;
glGenFramebuffers_FUNC         glGenFramebuffers;
glBindFramebuffer_FUNC         glBindFramebuffer;
glFramebufferTexture2D_FUNC    glFramebufferTexture2D;
glCheckFramebufferStatus_FUNC  glCheckFramebufferStatus;
glDeleteFramebuffers_FUNC      glDeleteFramebuffers;
glGenRenderbuffers_FUNC        glGenRenderbuffers;
glBindRenderbuffer_FUNC        glBindRenderbuffer;
glRenderbufferStorage_FUNC     glRenderbufferStorage;
glFramebufferRenderbuffer_FUNC glFramebufferRenderbuffer;
glDeleteBuffers_FUNC           glDeleteBuffers;
glDeleteProgram_FUNC           glDeleteProgram;
glDeleteVertexArrays_FUNC      glDeleteVertexArrays;

#ifdef _WIN32
	glActiveTexture_FUNC           glActiveTexture;
#endif

#endif
