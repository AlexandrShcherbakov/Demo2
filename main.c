#include "SDL.h"
#include <stdio.h>
#include <stdbool.h>

//#include "../GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _WIN32
#include <windows.h>
#include "../GL/wglext.h"
#endif
#include "../GL/glext.h"

//Include my libraries
#include "types.h"
#include "parameters.h"
#include "glElements.h"


void ReadCornellBox() {
    //Open file with scene
    FILE *file = fopen("scenes/1.txt", "r");
    if (!file) {
        file = fopen("scenes\\1.txt", "r");
    }
    //Read count of polygons in scene
    fscanf(file, "%d", &polygonCount);

    //Array of polygons in scene
    scene = calloc(polygonCount, sizeof(*scene));
    for (int i = 0; i < polygonCount; ++i) {
        scene[i] = readPolygon(file);
        //Update verticies count
        verticesCount += scene[i].length;
    }

    //Hardcoded direction of spotlight
    spotLightDirection = v3(0, -1.0f, 0);

    //Hardcoded start rotate
    rotate = v3(0, 0, 0);

    //Hardcoded position of spotlight
    spotLightPosition = v3(0.0f, 1.0f, 0.0f);

    //Hardcoded view point
    viewPoint = v3(0.0f, 0.0f, 1.4f);

    //Hardcoded view direction
    viewDirection = v3(0.0f, 0.0f, -1.0f);

    //Standart view angle
    viewAngle = 45.0f;

    //Spot light parameters
    spotAngIn = 25.0f;
    spotAngOut = 45.0f;

    //Need to create shadow map
    actualShadowMap = false;
}

//Add buffers and other data to OpenGL
void PrepairGLBuffers() {
    //Add Vertex Array Object, which contains all buffers
    glGenVertexArrays(1, &meshVAO);                                              CHECK_GL_ERRORS

    //Initialization of external buffers
    extGLBufVertices = calloc(3 * (verticesCount - 2), 3 * sizeof(*extGLBufVertices));
    extGLBufAmbCol   = calloc(3 * (verticesCount - 2), 3 * sizeof(*extGLBufAmbCol));
    extGLBufDifCol   = calloc(3 * (verticesCount - 2), 3 * sizeof(*extGLBufDifCol));
    extGLBufSpcCol   = calloc(3 * (verticesCount - 2), 3 * sizeof(*extGLBufSpcCol));
    extGLBufNormInds = calloc(3 * (verticesCount - 2), sizeof(*extGLBufNormInds));
}


void PrepairShadowMap() {
    //Free old resources
    glDeleteFramebuffers(1, &fbo);                                               CHECK_GL_ERRORS
    glDeleteTextures(1, &shadowMap);                                             CHECK_GL_ERRORS

	glUseProgram(shaderProgram);                                                 CHECK_GL_ERRORS
	//Create framebuffer
    glGenFramebuffers(1, &fbo);                                                  CHECK_GL_ERRORS
    //Create shadow map
    glGenTextures(1, &shadowMap);                                                CHECK_GL_ERRORS
    //Create texture for shadow map
    glBindTexture(GL_TEXTURE_2D, shadowMap);                                     CHECK_GL_ERRORS
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_EDGE,
				SHADOWMAP_EDGE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);           CHECK_GL_ERRORS
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);            CHECK_GL_ERRORS
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);            CHECK_GL_ERRORS
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);         CHECK_GL_ERRORS
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);         CHECK_GL_ERRORS
	//Bind shadowmap to framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);                                 CHECK_GL_ERRORS
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							GL_TEXTURE_2D, shadowMap, 0);                        CHECK_GL_ERRORS
}


//Add uniforms to OpenGL
void AddGLUniforms() {
    //Scene color
    SetUniform4f(shaderProgram, "sceneColor", v4(0.5f, 0.5f, 0.5f, 1.0f));

    //Set spotlight source
    SetUniform4f(shaderProgram, "lg.ambient", v4(0.0f, 0.0f, 0.0f, 1.0f));
    SetUniform4f(shaderProgram, "lg.diffuse", v4(2.0f, 2.0f, 2.0f, 1.0f));
    SetUniform4f(shaderProgram, "lg.specular", v4(1.0f, 1.0f, 1.0f, 1.0f));
    SetUniform3f(shaderProgram, "lg.spotPosition", spotLightPosition);
    SetUniform3f(shaderProgram, "lg.spotDirection", spotLightDirection);
    SetUniform1f(shaderProgram, "lg.spotCosCutoff", cos(spotAngOut));
    SetUniform1f(shaderProgram, "lg.innerConeCos", cos(spotAngIn));

    //Set array of normals
    vec3 * normals = calloc(polygonCount, sizeof(*normals));
    for (int i = 0; i < polygonCount; ++i) {
        normals[i] = scene[i].normal;
    }
    SetUniform3fv(shaderProgram, "norm", polygonCount, normals);

    //Set materials
    for(int i = 0; i < polygonCount; ++i){
        SetUniform4fInd(shaderProgram, "maters[%d].ambient", i, scene[i].mat.ambient);
        SetUniform4fInd(shaderProgram, "maters[%d].diffuse", i, scene[i].mat.diffuse);
        SetUniform4fInd(shaderProgram, "maters[%d].specular", i, scene[i].mat.specular);
        SetUniform1fInd(shaderProgram, "maters[%d].shininess", i, scene[i].mat.shininess);
    }
}

void SetUniformsForShadowMap() {
	glUseProgram(shaderProgram);
	//Projection Matrix
	float projectionMatrix[16];
	const float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	//Fill matrix
	Matrix4Perspective(projectionMatrix, spotAngOut * 3, 1, 0.05f, 5.0f);

	//Shift matrix
	float shiftMatrix[16];
	//Fill shift matrix
	Matrix4Shift(shiftMatrix, mult(spotLightPosition, -1));

	//Rotate matrix
	float rotateMatrix[16];
	//Fill rotate matrix
	Matrix4Rotate(rotateMatrix, CamViewerRotation);

	MultMatrix4(projectionMatrix, rotateMatrix, lightMatrix);
    MultMatrix4(lightMatrix, shiftMatrix, lightMatrix);

    SetUniformMat4f(shaderProgram, "camMatrix", lightMatrix);
    SetUniformMat4f(shaderProgram, "lightMatrix", lightMatrix);

    //Set viewer position
    SetUniform3f(shaderProgram, "viewer", spotLightPosition);

    SetUniform3f(shaderProgram, "lg.spotPosition", spotLightPosition);
    SetUniform3f(shaderProgram, "lg.spotDirection", spotLightDirection);
    glUseProgram(0);
}

void SetStandartCamera() {
	glUseProgram(shaderProgram);
	//Projection Matrix
	float projectionMatrix[16];
	const float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	//Fill matrix
	Matrix4Perspective(projectionMatrix, viewAngle, aspectRatio, 0.5f, 5.0f);

	//Shift matrix
	float shiftMatrix[16];
	//Fill shift matrix
	Matrix4Shift(shiftMatrix, mult(viewPoint, -1));

	//Rotate matrix
	float rotateMatrix[16];
	//Fill rotate matrix
	Matrix4Rotate(rotateMatrix, rotate);

	MultMatrix4(projectionMatrix, rotateMatrix, objectMatrix);
    MultMatrix4(objectMatrix, shiftMatrix, objectMatrix);

    SetUniformMat4f(shaderProgram, "camMatrix", objectMatrix);
    SetUniformMat4f(shaderProgram, "lightMatrix", lightMatrix);

	//Set viewer position
    SetUniform3f(shaderProgram, "viewer", viewPoint);
    glUseProgram(0);
}

void ChangeUniforms() {
	glUseProgram(shaderProgram);
	//Update spot light direction
	SetUniform3f(shaderProgram, "lg.spotDirection", spotLightDirection);

	SetStandartCamera();
	glUseProgram(0);
}

//Function, which creates OpenGL context, sets parameters of scene and take data to OpenGL buffers
void PrepairOpenGL(SDL_Window *window) {
	//Create context for OpenGL
    SDL_GLContext *context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

	//Load functions for OpenGL
	SDL_GL_LoadLibrary(NULL);
	LoadGLFunctions();

	//Information about hardware
	printf("GPU Vendor: %s\n", glGetString(GL_VENDOR));
	printf("GPU Name  : %s\n", glGetString(GL_RENDERER));
	printf("GL_VER    : %s\n", glGetString(GL_VERSION));
	printf("GLSL_VER  : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    ReadCornellBox();

    //Shaders
    GLuint vertexShader, fragmentShader, geometryShader;

	//Compile all shaders
    CompileShader("shaders/vertex.vert", &vertexShader, GL_VERTEX_SHADER);
    CompileShader("shaders/fragment.frag", &fragmentShader, GL_FRAGMENT_SHADER);

	//Initialize program
    shaderProgram  = glCreateProgram();                                          CHECK_GL_ERRORS

    //Attach shaders to program
    glAttachShader(shaderProgram, vertexShader);                                 CHECK_GL_ERRORS
    glAttachShader(shaderProgram, fragmentShader);                               CHECK_GL_ERRORS

    glLinkProgram(shaderProgram);                                                CHECK_GL_ERRORS

    //Check whether the link is successful
    if (ShaderProgramStatus(shaderProgram, GL_LINK_STATUS) != GL_TRUE)
        fprintf(stderr, "BUGURT!!! Line: %d\n", __LINE__);

    //Choose our program to execution
    glUseProgram(shaderProgram);                                                 CHECK_GL_ERRORS

    //Enable z-buffer
    glEnable(GL_DEPTH_TEST);                                                     CHECK_GL_ERRORS

    PrepairGLBuffers();
    AddGLUniforms();
    PrepairShadowMap();
}


void PassCornellBoxDataToGL() {
    //Set vertices and normals to external gl buffers
    for (int i = 0, vertIter = 0; i < polygonCount; ++i) {
        for (int j = 1, vertShift = 0; j < scene[i].length - 1; ++j) {
            if (i == 5) {
                vertShift += 3;
                continue;
            }
            //Add first vertex
            extGLBufVertices[3 * (vertIter + vertShift) + 0] = scene[i].vertices[0].x;
            extGLBufVertices[3 * (vertIter + vertShift) + 1] = scene[i].vertices[0].y;
            extGLBufVertices[3 * (vertIter + vertShift) + 2] = scene[i].vertices[0].z;
            //Add index of normal
            extGLBufNormInds[vertIter + vertShift] = i;
            vertShift++;
            //Add second vertex
            extGLBufVertices[3 * (vertIter + vertShift) + 0] = scene[i].vertices[j].x;
            extGLBufVertices[3 * (vertIter + vertShift) + 1] = scene[i].vertices[j].y;
            extGLBufVertices[3 * (vertIter + vertShift) + 2] = scene[i].vertices[j].z;
            //Add index of normal
            extGLBufNormInds[vertIter + vertShift] = i;
            vertShift++;
            //Add third vertex
            extGLBufVertices[3 * (vertIter + vertShift) + 0] = scene[i].vertices[j + 1].x;
            extGLBufVertices[3 * (vertIter + vertShift) + 1] = scene[i].vertices[j + 1].y;
            extGLBufVertices[3 * (vertIter + vertShift) + 2] = scene[i].vertices[j + 1].z;
            //Add index of normal
            extGLBufNormInds[vertIter + vertShift] = i;
            vertShift++;
        }
        vertIter += 3 * (scene[i].length - 2);
    }

    //Locations for buffers
    GLint verticesLocation, normalsLocation;

    int bufferSize = 3 * sizeof(*extGLBufVertices) * 3 * (verticesCount - 2);

    glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS

    //Pass vertices to shader
    glGenBuffers(1, &intGlBufVerticies);                                         CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, intGlBufVerticies);                            CHECK_GL_ERRORS
	glBufferData(GL_ARRAY_BUFFER, bufferSize, extGLBufVertices, GL_STATIC_DRAW); CHECK_GL_ERRORS
	verticesLocation = glGetAttribLocation(shaderProgram, "position");           CHECK_GL_ERRORS
	if (verticesCount != -1) {
		glVertexAttribPointer(verticesLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);    CHECK_GL_ERRORS
		glEnableVertexAttribArray(verticesLocation);                             CHECK_GL_ERRORS
	} else {
        fprintf(stderr, "Loaction position not found\n");
	}
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                            CHECK_GL_ERRORS

    //Pass normals to shader
    glGenBuffers(1, &intGLBufNormInds);                                          CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, intGLBufNormInds);                             CHECK_GL_ERRORS
	glBufferData(GL_ARRAY_BUFFER,
              3 * (verticesCount - 2) * sizeof(*extGLBufNormInds),
              extGLBufNormInds, GL_STATIC_DRAW);                                 CHECK_GL_ERRORS
	normalsLocation = glGetAttribLocation(shaderProgram, "v_polInd");            CHECK_GL_ERRORS
    if (normalsLocation != -1) {
		glVertexAttribIPointer(normalsLocation, 1, GL_UNSIGNED_BYTE, 0, 0);      CHECK_GL_ERRORS
		glEnableVertexAttribArray(normalsLocation);                              CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location v_polInd not found\n");
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                            CHECK_GL_ERRORS

    glBindVertexArray(0);                                                        CHECK_GL_ERRORS
}

void PassShadowMap() {
    glViewport(0, 0, SHADOWMAP_EDGE, SHADOWMAP_EDGE);                            CHECK_GL_ERRORS
	glUseProgram(shaderProgram);                                                 CHECK_GL_ERRORS
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                          CHECK_GL_ERRORS
	glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS
	glDrawArrays(GL_TRIANGLES, 0, 3 * (verticesCount - 2));

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);                    CHECK_GL_ERRORS
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "FB error, status: 0x%x\n", Status);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);                                   CHECK_GL_ERRORS

    glActiveTexture(GL_TEXTURE0);                                                CHECK_GL_ERRORS
    glBindTexture(GL_TEXTURE_2D, shadowMap);                                     CHECK_GL_ERRORS
    SetUniform1i(shaderProgram, "shadowMap", 0);
    glUseProgram(0);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void DrawCornellBox(SDL_Window * window) {
    //Update uniforms
    ChangeUniforms();
	//Use shadow map
	if (!actualShadowMap) {
		spotLightDirection = normalize(spotLightDirection);
		CamViewerRotation = v3(M_PI / 2, 0, 0);
        CamViewerRotation.x += asin(normalize(spotLightDirection).z);
        CamViewerRotation.z -= asin(normalize(spotLightDirection).x);
        PrepairShadowMap();
		SetUniformsForShadowMap();
		PassShadowMap();
		SetStandartCamera();
		actualShadowMap = true;
	}
	glUseProgram(shaderProgram);                                                 CHECK_GL_ERRORS
	//Use buffers
	glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS

	//Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                          CHECK_GL_ERRORS

    glDrawArrays(GL_TRIANGLES, 0, 3 * (verticesCount - 2));                      CHECK_GL_ERRORS
    SDL_GL_SwapWindow(window);                                                   CHECK_GL_ERRORS
    glBindVertexArray(0);                                                        CHECK_GL_ERRORS
}


void HandleKeyDown(SDL_Keycode code, bool *quit) {
    if (code == SDLK_UP) {
        spotLightPosition.z -= 0.01;
        actualShadowMap = false;
    } else if (code == SDLK_DOWN) {
    	spotLightPosition.z += 0.01;
    	actualShadowMap = false;
    } else if (code == SDLK_LEFT) {
    	spotLightPosition.x -= 0.01;
    	actualShadowMap = false;
    } else if (code == SDLK_RIGHT) {
    	spotLightPosition.x += 0.01;
    	actualShadowMap = false;
    } else if (code == SDLK_d) {
        viewPoint.x += 0.1;
    } else if (code == SDLK_a) {
        viewPoint.x -= 0.1;
    } else if (code == SDLK_w) {
    	viewPoint.z -= 0.1;
    } else if (code == SDLK_s) {
        viewPoint.z += 0.1;
    } else if (code == SDLK_ESCAPE) {
        *quit = true;
    }
}

void MoveCameraByMouse(SDL_Window * window) {
    int x, y;
    int cX = SCREEN_WIDTH / 2, cY = SCREEN_HEIGHT / 2;
    SDL_GetMouseState(&x, &y);
    rotate.y += (float)(x - cX) / 100.0f;
    rotate.x += (float)(y - cY) / 100.0f;
    SDL_WarpMouseInWindow(window, cX, cY);
}


int main(int argc, char* argv[]) {
    //Pointer to window
    SDL_Window *window;

    //Initialization of SDL
    SDL_Init( SDL_INIT_EVERYTHING );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    //Create of window
    window = SDL_CreateWindow("Demo 2",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    //Exit if window wasn`t create
    if (window == NULL) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    //Flag: true - if user close window; false - else
    bool quit = false;
    //Variable for events which creates in runtime
    SDL_Event e;

    PrepairOpenGL(window);
	PassCornellBoxDataToGL();

    SDL_GL_SetSwapInterval(1);
    SDL_WarpMouseInWindow(window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    SDL_ShowCursor(0);

    //Main loop
    while( !quit ) {
        //Wait for event
        while( SDL_PollEvent( &e ) != 0 ) {
            if( e.type == SDL_QUIT ) { //Check if user close window
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
            	HandleKeyDown(e.key.keysym.sym, &quit);
            }
        }
        MoveCameraByMouse(window);
        DrawCornellBox(window);
    }
    //Destroy window and release resources
    SDL_DestroyWindow(window);

    //End work with SDL
    SDL_Quit();
    return 0;
}
