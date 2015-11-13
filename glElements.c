#include "glElements.h"

GLint ShaderStatus(GLuint shader, GLenum param) {
	GLint status, length;
	GLchar buffer[1024];

	glGetShaderiv(shader, param, &status);

	if (status != GL_TRUE) {
		glGetShaderInfoLog(shader, 1024, &length, buffer);
		fprintf(stderr, "Shader: %s\n", (const char*)buffer);
	}
	OPENGL_CHECK_FOR_ERRORS();
	return status;
}


GLint ShaderProgramStatus(GLuint program, GLenum param)
{
	GLint status, length;
	GLchar buffer[1024];
	glGetProgramiv(program, param, &status);

	if (status != GL_TRUE)
	{
		glGetProgramInfoLog(program, 1024, &length, buffer);
		fprintf(stderr, "Shader program: %s\n", (const char*)buffer);
	}

	OPENGL_CHECK_FOR_ERRORS();
	return status;
}

void ThrowExceptionOnGLError(int line, const char *file) {
  static char errMsg[512];

  GLenum gl_error = glGetError();

  if(gl_error == GL_NO_ERROR)
    return;

  switch(gl_error)
  {
  case GL_INVALID_ENUM:
    sprintf(errMsg, "GL_INVALID_ENUM file %s line %d\n", file, line);
    break;

  case GL_INVALID_VALUE:
    sprintf(errMsg, "GL_INVALID_VALUE file %s line %d\n",  file, line);
    break;

  case GL_INVALID_OPERATION:
    sprintf(errMsg, "GL_INVALID_OPERATION file %s line %d\n",  file, line);
    break;

  case GL_STACK_OVERFLOW:
    sprintf(errMsg, "GL_STACK_OVERFLOW file %s line %d\n",  file, line);
    break;

  case GL_STACK_UNDERFLOW:
    sprintf(errMsg, "GL_STACK_UNDERFLOW file %s line %d\n",  file, line);
    break;

  case GL_OUT_OF_MEMORY:
    sprintf(errMsg, "GL_OUT_OF_MEMORY file %s line %d\n",  file, line);
    break;

  /*case GL_TABLE_TOO_LARGE:
    sprintf(errMsg, "GL_TABLE_TOO_LARGE file %s line %d\n",  file, line);
    break;*/

  case GL_NO_ERROR:
    break;

  default:
    sprintf(errMsg, "Unknown error @ file %s line %d\n",  file, line);
    break;
  }

  if(gl_error != GL_NO_ERROR)
    fprintf(stderr, "!!!ERROR BUGURT\n%s", errMsg);
}

int loadSource(char * shaderName, char **textOut, int *textLen) {
	FILE *input;

	input = fopen(shaderName, "r");

	fseek(input, 0, SEEK_END);
	*textLen = ftell(input);
	rewind(input);

	*textOut = calloc(sizeof(*textOut), *textLen + 1);

	*textLen = fread(*textOut, sizeof(**textOut), *textLen, input);
	close(input);
	return 1;
}

void loadGLFunctions() {
    OPENGL_GET_PROC(glGetShaderiv_FUNC,             glGetShaderiv);
	OPENGL_GET_PROC(glGetShaderInfoLog_FUNC,        glGetShaderInfoLog);
	OPENGL_GET_PROC(glGetProgramiv_FUNC,            glGetProgramiv);
	OPENGL_GET_PROC(glGetProgramInfoLog_FUNC,       glGetProgramInfoLog);
	OPENGL_GET_PROC(glCreateProgram_FUNC,           glCreateProgram);
	OPENGL_GET_PROC(glCreateShader_FUNC,            glCreateShader);
	OPENGL_GET_PROC(glShaderSource_FUNC,            glShaderSource);
	OPENGL_GET_PROC(glCompileShader_FUNC,           glCompileShader);
	OPENGL_GET_PROC(glAttachShader_FUNC,            glAttachShader);
	OPENGL_GET_PROC(glLinkProgram_FUNC,             glLinkProgram);
	OPENGL_GET_PROC(glUseProgram_FUNC,              glUseProgram);
	OPENGL_GET_PROC(glGenVertexArrays_FUNC,         glGenVertexArrays);
	OPENGL_GET_PROC(glBindVertexArray_FUNC,         glBindVertexArray);
	OPENGL_GET_PROC(glGenBuffers_FUNC,              glGenBuffers);
	OPENGL_GET_PROC(glBindBuffer_FUNC,              glBindBuffer);
	OPENGL_GET_PROC(glBufferData_FUNC,              glBufferData);
	OPENGL_GET_PROC(glGetUniformLocation_FUNC,      glGetUniformLocation);
	OPENGL_GET_PROC(glUniformMatrix4fv_FUNC,        glUniformMatrix4fv);
	OPENGL_GET_PROC(glGetAttribLocation_FUNC,       glGetAttribLocation);
	OPENGL_GET_PROC(glVertexAttribPointer_FUNC,     glVertexAttribPointer);
	OPENGL_GET_PROC(glEnableVertexAttribArray_FUNC, glEnableVertexAttribArray);
}

void Matrix4Perspective(float *M, float fovy, float aspect, float znear, float zfar) {
        //Convert fovy from graduses to radians
        float f = 1 / tanf(fovy * M_PI / 360),
              A = (zfar + znear) / (znear - zfar),
              B = (2 * zfar * znear) / (znear - zfar);

        M[ 0] = f / aspect; M[ 1] =  0; M[ 2] =  0; M[ 3] =  0;
        M[ 4] = 0;          M[ 5] =  f; M[ 6] =  0; M[ 7] =  0;
        M[ 8] = 0;          M[ 9] =  0; M[10] =  A; M[11] =  B;
        M[12] = 0;          M[13] =  0; M[14] =  1; M[15] =  2.5;
}

void CompileShader(const char * name, GLuint * shader, GLenum shaderType) {
	//Initialization of shader
	*shader = glCreateShader(shaderType);                                        CHECK_GL_ERRORS

	//Variables for shader texts
    char *shaderSource;
    int sourceLength;

    //Read vertex shader
    loadSource(name, &shaderSource, &sourceLength);                              CHECK_GL_ERRORS

    //Compile vertex shader
    glShaderSource(*shader, 1, (const GLchar**)&shaderSource,
                (const GLint*)&sourceLength);                                    CHECK_GL_ERRORS
    glCompileShader(*shader);                                                    CHECK_GL_ERRORS

    //Release source memory
    free(shaderSource);

    //Check whether the compilation is successful
    if (ShaderStatus(*shader, GL_COMPILE_STATUS) != GL_TRUE)
        fprintf(stderr, "BUGURT!!! Line: %d\n", __LINE__);
}
