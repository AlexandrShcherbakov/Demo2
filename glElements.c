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

int LoadSource(char * shaderName, char **textOut, int *textLen) {
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

void LoadGLFunctions() {
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
	OPENGL_GET_PROC(glUniform4fv_FUNC,              glUniform4fv);
	OPENGL_GET_PROC(glUniform3fv_FUNC,              glUniform3fv);
	OPENGL_GET_PROC(glUniform1f_FUNC,               glUniform1f);
	OPENGL_GET_PROC(glUniform1i_FUNC,               glUniform1i);
	OPENGL_GET_PROC(glVertexAttribIPointer_FUNC,    glVertexAttribIPointer);
	OPENGL_GET_PROC(glGenFramebuffers_FUNC,         glGenFramebuffers);
	OPENGL_GET_PROC(glBindFramebuffer_FUNC,         glBindFramebuffer);
	OPENGL_GET_PROC(glFramebufferTexture2D_FUNC,    glFramebufferTexture2D);
	OPENGL_GET_PROC(glCheckFramebufferStatus_FUNC,  glCheckFramebufferStatus);
	OPENGL_GET_PROC(glDeleteFramebuffers_FUNC,      glDeleteFramebuffers);
	OPENGL_GET_PROC(glGenRenderbuffers_FUNC,        glGenRenderbuffers);
	OPENGL_GET_PROC(glBindRenderbuffer_FUNC,        glBindRenderbuffer);
	OPENGL_GET_PROC(glRenderbufferStorage_FUNC,     glRenderbufferStorage);
	OPENGL_GET_PROC(glFramebufferRenderbuffer_FUNC, glFramebufferRenderbuffer);
	OPENGL_GET_PROC(glDeleteBuffers_FUNC,           glDeleteBuffers);
	OPENGL_GET_PROC(glDeleteProgram_FUNC,           glDeleteProgram);
	OPENGL_GET_PROC(glDeleteVertexArrays_FUNC,      glDeleteVertexArrays);

  #ifdef _WIN32
  OPENGL_GET_PROC(glActiveTexture_FUNC,           glActiveTexture);
  #endif
}

//Fill perspective matrix 4x4
void Matrix4Perspective(float *M, float fovy, float aspect, float znear, float zfar) {
	//Convert fovy from graduses to radians
	float f = 1 / tanf(fovy * M_PI / 360),
			A = (zfar + znear) / (znear - zfar),
			B = (2 * zfar * znear) / (znear - zfar);

	M[ 0] = f / aspect; M[ 1] =  0; M[ 2] =  0; M[ 3] =  0;
	M[ 4] = 0;          M[ 5] =  f; M[ 6] =  0; M[ 7] =  0;
	M[ 8] = 0;          M[ 9] =  0; M[10] =  A; M[11] =  B;
	M[12] = 0;          M[13] =  0; M[14] = -1; M[15] =  0;
}

//Fill shift matrix 4x4
void Matrix4Shift(float *M, vec3 shift) {
	M[ 0] = 1; M[ 1] = 0; M[ 2] = 0; M[ 3] = shift.x;
	M[ 4] = 0; M[ 5] = 1; M[ 6] = 0; M[ 7] = shift.y;
	M[ 8] = 0; M[ 9] = 0; M[10] = 1; M[11] = shift.z;
	M[12] = 0; M[13] = 0; M[14] = 0; M[15] =       1;
}

void FillRotateMatrix3(float *M, vec3 v, float a) {
    float ca = cos(a), sa = sin(a);
    float nca = 1 - ca;
    M[0] =       ca + nca * v.x * v.x; M[1] = nca * v.x * v.y - sa * v.z; M[2] = nca * v.x * v.y + sa * v.y;
    M[3] = nca * v.y * v.x + sa * v.z; M[4] =       ca + nca * v.y * v.y; M[5] = nca * v.y * v.z - sa * v.x;
    M[6] = nca * v.z * v.x - sa * v.y; M[7] = nca * v.z * v.y + sa * v.x; M[8] = ca + nca * v.z * v.z;
}

vec3 MultMatrix3Vec(float *M, vec3 v) {
    return v3(M[0] * v.x + M[1] * v.y + M[2] * v.z,
              M[3] * v.x + M[4] * v.y + M[5] * v.z,
              M[6] * v.x + M[7] * v.y + M[8] * v.z);
}

void MultMatrix3(float *M1, float *M2, float *Mres) {
    Mres[0] = M1[0] * M2[0] + M1[1] * M2[3] + M1[2] * M2[6];
    Mres[1] = M1[0] * M2[1] + M1[1] * M2[4] + M1[2] * M2[7];
    Mres[2] = M1[0] * M2[2] + M1[1] * M2[5] + M1[2] * M2[8];
    Mres[3] = M1[3] * M2[0] + M1[4] * M2[3] + M1[5] * M2[6];
    Mres[4] = M1[3] * M2[1] + M1[4] * M2[4] + M1[5] * M2[7];
    Mres[5] = M1[3] * M2[2] + M1[4] * M2[5] + M1[5] * M2[8];
    Mres[6] = M1[6] * M2[0] + M1[7] * M2[3] + M1[8] * M2[6];
    Mres[7] = M1[6] * M2[1] + M1[7] * M2[4] + M1[8] * M2[7];
    Mres[8] = M1[6] * M2[2] + M1[7] * M2[5] + M1[8] * M2[8];
}

void MultMatrix4(float *M1, float *M2, float *Mres) {
	float Mloc[16];
	for (int i = 0; i < 16; ++i) {
        Mloc[i] = 0;
    }
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
			for (int h = 0; h < 4; ++h) {
                Mloc[i * 4 + j] += M1[i * 4 + h] * M2[h * 4 + j];
			}
        }
    }
    for (int i = 0; i < 16; ++i) {
        Mres[i] = Mloc[i];
    }
}

void Matrix4Rotate(float *M, vec3 a) {
    vec3 y = v3(0, 1, 0);
    vec3 z = v3(0, 0, 1);
    float Mx[9], My[9], Mz[9], Mxy[9], Mxyz[9];
    FillRotateMatrix3(Mx, v3(1, 0, 0), a.x);
    y = MultMatrix3Vec(Mx, y);
    FillRotateMatrix3(My, y, a.y);
    MultMatrix3(My, Mx, Mxy);
    z = MultMatrix3Vec(Mxy, z);
    FillRotateMatrix3(Mz, z, a.z);
    MultMatrix3(Mz, Mxy, Mxyz);
    M[ 0] = Mxyz[0]; M[ 1] = Mxyz[1]; M[ 2] = Mxyz[2]; M[ 3] = 0;
    M[ 4] = Mxyz[3]; M[ 5] = Mxyz[4]; M[ 6] = Mxyz[5]; M[ 7] = 0;
    M[ 8] = Mxyz[6]; M[ 9] = Mxyz[7]; M[10] = Mxyz[8]; M[11] = 0;
    M[12] =       0; M[13] =       0; M[14] =       0; M[15] = 1;
}

void CompileShader(const char * name, GLuint * shader, GLenum shaderType) {
    printf("%s compilation start\n", name);
	//Initialization of shader
	*shader = glCreateShader(shaderType);                                        CHECK_GL_ERRORS

	//Variables for shader texts
    char *shaderSource;
    int sourceLength;

    //Read vertex shader
    LoadSource(name, &shaderSource, &sourceLength);                              CHECK_GL_ERRORS

    //Compile vertex shader
    glShaderSource(*shader, 1, (const GLchar**)&shaderSource,
                (const GLint*)&sourceLength);                                    CHECK_GL_ERRORS
    glCompileShader(*shader);                                                    CHECK_GL_ERRORS

    //Release source memory
    free(shaderSource);

    //Check whether the compilation is successful
    if (ShaderStatus(*shader, GL_COMPILE_STATUS) != GL_TRUE) {
        fprintf(stderr, "BUGURT!!! Line: %d\n", __LINE__);
        exit(1);
    }
}

void SetUniform4f(GLuint prog, const char * name, vec4 v) {
    GLint location = glGetUniformLocation(prog, name);                           CHECK_GL_ERRORS
    if (location != -1) {
        glUniform4fv(location, 1, &v);                                           CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", name);
    }
}

void SetUniform3f(GLuint prog, const char * name, vec3 v) {
    GLint location = glGetUniformLocation(prog, name);                           CHECK_GL_ERRORS
    if (location != -1) {
        glUniform3fv(location, 1, &v);                                           CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", name);
    }
}

void SetUniform1f(GLuint prog, const char * name, float v) {
    GLint location = glGetUniformLocation(prog, name);                           CHECK_GL_ERRORS
    if (location != -1) {
        glUniform1f(location, v);                                                CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", name);
    }
}

void SetUniform1i(GLuint prog, const char * name, int v) {
    GLint location = glGetUniformLocation(prog, name);                           CHECK_GL_ERRORS
    if (location != -1) {
        glUniform1i(location, v);                                                CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", name);
    }
}

void SetUniform3fv(GLuint prog, const char * name, int count, float * v) {
    GLint location = glGetUniformLocation(prog, name);                           CHECK_GL_ERRORS
    if (location != -1) {
        glUniform3fv(location, count, v);                                        CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", name);
    }
}

void SetUniformMat4f(GLuint prog, const char * name, float * v) {
	GLint location = glGetUniformLocation(prog, name);                           CHECK_GL_ERRORS
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_TRUE, v);                             CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", name);
    }
}

void SetUniform4fInd(GLuint prog, const char * name, int ind, vec4 v) {
    char * newName = calloc(1024, 1);
    sprintf(newName, name, ind);
    GLint location = glGetUniformLocation(prog, newName);                        CHECK_GL_ERRORS
    if (location != -1) {
        glUniform4fv(location, 1, (GLfloat *)&v);                                           CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", newName);
    }
    free(newName);
}

void SetUniform1fInd(GLuint prog, const char * name, int ind, float v) {
    char * newName = calloc(1024, 1);
    sprintf(newName, name, ind);
    GLint location = glGetUniformLocation(prog, newName);                        CHECK_GL_ERRORS
    if (location != -1) {
        glUniform1f(location, v);                                             CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location: %s not found!\n", newName);
    }
    free(newName);
}
