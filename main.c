#include "SDL.h"
#include <stdio.h>
#include <stdbool.h>

#include <GL/gl.h>
#include <GL/glu.h>
//#include "../GL/glew.h"

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
    spotLightDirection = v3(0, -1, 0);

    //Hardcoded start rotate
    rotate = v3(0, 0, 0);
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


//Add uniforms to OpenGL
void AddGLUniforms() {
    //Projection Matrix
	float projectionMatrix[16];
	const float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	//Fill matrix
	Matrix4Perspective(projectionMatrix, 45.0f, aspectRatio, 0.5f, 5.0f);

	//Pass matrix to shader
	SetUniformMat4f(shaderProgram, "projectionMatrix", projectionMatrix);

	//Shift matrix
	float shiftMatrix[16];
	//Fill shift matrix
	Matrix4Shift(shiftMatrix, v3(0.0f, 0.0f, -1.4f));
	//Pass shift matrix
	SetUniformMat4f(shaderProgram, "shiftMatrix", shiftMatrix);

    //Scene color
    SetUniform4f(shaderProgram, "sceneColor", v4(0.1f, 0.1f, 0.1f, 1.0f));

    //Set spotlight source
    SetUniform4f(shaderProgram, "lg.ambient", v4(0.0f, 0.0f, 0.0f, 1.0f));
    SetUniform4f(shaderProgram, "lg.diffuse", v4(2.0f, 2.0f, 2.0f, 1.0f));
    SetUniform4f(shaderProgram, "lg.specular", v4(1.0f, 1.0f, 1.0f, 1.0f));
    SetUniform3f(shaderProgram, "lg.spotPosition", v3(0.0f, 1.0f, 0.0f));
    SetUniform3f(shaderProgram, "lg.spotDirection", spotLightDirection);
    SetUniform1f(shaderProgram, "lg.spotCosCutoff", 0.7f);
    SetUniform1f(shaderProgram, "lg.innerConeCos", 0.9f);

    //Set array of normals
    vec3 * normals = calloc(polygonCount, sizeof(*normals));
    for (int i = 0; i < polygonCount; ++i) {
        normals[i] = scene[i].normal;
    }
    SetUniform3fv(shaderProgram, "norm", polygonCount, normals);

    //Set viewer position
    SetUniform3f(shaderProgram, "viewer", v3(0.0f, 0.0f, 1.0f));

    //Set materials
    for(int i = 0; i < polygonCount; ++i){
        SetUniform4fInd(shaderProgram, "maters[%d].ambient", i, scene[i].mat.ambient);
        SetUniform4fInd(shaderProgram, "maters[%d].diffuse", i, scene[i].mat.diffuse);
        SetUniform4fInd(shaderProgram, "maters[%d].specular", i, scene[i].mat.specular);
        SetUniform1fInd(shaderProgram, "maters[%d].shininess", i, scene[i].mat.shininess);
    }
}

void ChangeUniforms() {
	//Update spot light direction
	SetUniform3f(shaderProgram, "lg.spotDirection", spotLightDirection);

	//Shift matrix
	float rotateMatrix[16];
	//Fill shift matrix
	Matrix4Rotate(rotateMatrix, rotate);
	//Pass shift matrix
	SetUniformMat4f(shaderProgram, "rotateMatrix", rotateMatrix);
}

//Function, which creates OpenGL context, sets parameters of scene and take data to OpenGL buffers
void PrepairOpenGL(SDL_Window *window) {
	//Create context for OpenGL
    SDL_GLContext *context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

	//Load functions for OpenGL
	SDL_GL_LoadLibrary(NULL);
	loadGLFunctions();

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
    glEnable(GL_DEPTH_TEST);

    PrepairGLBuffers();
    AddGLUniforms();
}


void PassCornellBoxDataToGL() {
    //Set vertices and normals to external gl buffers
    for (int i = 0, vertIter = 0; i < polygonCount; ++i) {
        for (int j = 1, vertShift = 0; j < scene[i].length - 1; ++j) {
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


void DrawCornellBox(SDL_Window * window) {
    //Update uniforms
    ChangeUniforms();
    //Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                          CHECK_GL_ERRORS

	//Use buffers
	glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS

    glDrawArrays(GL_TRIANGLES, 0, 3 * (verticesCount - 2));                      CHECK_GL_ERRORS
    SDL_GL_SwapWindow(window);                                                   CHECK_GL_ERRORS
    glBindVertexArray(0);                                                        CHECK_GL_ERRORS
}


void HandleKeyDown(SDL_Keycode code) {
    if (code == SDLK_UP) {
        spotLightDirection.z -= 0.1;
    } else if (code == SDLK_DOWN) {
    	spotLightDirection.z += 0.1;
    } else if (code == SDLK_LEFT) {
    	spotLightDirection.x -= 0.1;
    } else if (code == SDLK_RIGHT) {
    	spotLightDirection.x += 0.1;
    } else if (code == SDLK_d) {
        rotate.y -= 0.1;
    } else if (code == SDLK_a) {
        rotate.y += 0.1;
    } else if (code == SDLK_w) {
        rotate.x += 0.1;
    } else if (code == SDLK_s) {
        rotate.x -= 0.1;
    }
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

    //Main loop
    while( !quit ) {
        //Wait for event
        while( SDL_PollEvent( &e ) != 0 ) {
            if( e.type == SDL_QUIT ) { //Check if user close window
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
            	HandleKeyDown(e.key.keysym.sym);
            }
        }
        DrawCornellBox(window);
        //break;
    }

    //Destroy window and release resources
    SDL_DestroyWindow(window);

    //End work with SDL
    SDL_Quit();
    return 0;
}
