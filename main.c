#include "SDL.h"
#include <stdio.h>
#include <stdbool.h>

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
}

//Add buffers and other data to OpenGL
void PrepairGLBuffers() {
    //Add Vertex Array Object, which contains all buffers
    glGenVertexArrays(1, &meshVAO);                                              CHECK_GL_ERRORS

    //Initialization of external buffers
    extGLBufVertices = calloc(verticesCount, 3 * sizeof(*extGLBufVertices));
    extGLBufColors = calloc(verticesCount, 3 * sizeof(*extGLBufColors));
}

//Add uniforms to OpenGL
void AddGLUniforms() {
    //Projection Matrix
    //External buffer
	float projectionMatrix[16];
	//Internal buffer
	GLint projMatLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");  CHECK_GL_ERRORS
	const float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	//Fill matrix
	Matrix4Perspective(projectionMatrix, 45.0f, aspectRatio, 0.5f, 5.0f);

	//Send matrix to shader
	if (projMatLoc != -1)
		glUniformMatrix4fv(projMatLoc, 1, GL_TRUE, projectionMatrix);            CHECK_GL_ERRORS
}

//Function, which creates OpenGL context, sets parameters of scene and take data to OpenGL buffers
void PrepairOpenGL(SDL_Window *window) {
	//Create context for OpenGL
    SDL_GLContext *context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, context);

	//Load functions for OpenGL
	SDL_GL_LoadLibrary(NULL);
	loadGLFunctions();

    ReadCornellBox();

    //Shaders
    GLuint vertexShader, fragmentShader, geometryShader;

	//Compile all shaders
    CompileShader("shaders/vertex shader", &vertexShader, GL_VERTEX_SHADER);
    CompileShader("shaders/geometry shader", &geometryShader, GL_GEOMETRY_SHADER);
    CompileShader("shaders/fragment shader", &fragmentShader, GL_FRAGMENT_SHADER);

	//Initialize program
    shaderProgram  = glCreateProgram();                                          CHECK_GL_ERRORS

    //Attach shaders to program
    glAttachShader(shaderProgram, vertexShader);                                 CHECK_GL_ERRORS
    glAttachShader(shaderProgram, geometryShader);                               CHECK_GL_ERRORS
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
        for (int j = 0; j < scene[i].length; ++j) {
            //Add vertex
            extGLBufVertices[3 * (vertIter + j) + 0] = scene[i].vertices[j].x;
            extGLBufVertices[3 * (vertIter + j) + 1] = scene[i].vertices[j].y;
            extGLBufVertices[3 * (vertIter + j) + 2] = scene[i].vertices[j].z;
        }
        vertIter += scene[i].length;
    }
    //Set colors to external gl buffer
    //Set all walls to white
    for (int i = 0; i < 3 * verticesCount; ++i) {
        extGLBufColors[i] = 1.0f;
    }
    //Set left wall to red
    for (int i = scene[0].length; i < scene[0].length + scene[1].length; ++i) {
        extGLBufColors[3 * i + 1] = 0;
        extGLBufColors[3 * i + 2] = 0;
    }
    //Set right wall to green
    for (int i = scene[1].length + scene[0].length; i < scene[0].length + scene[1].length + scene[2].length; ++i) {
        extGLBufColors[3 * i + 0] = 0;
        extGLBufColors[3 * i + 2] = 0;
    }

    //Locations for buffers
    GLint verticesLocation, colorsLocation, normalsLocation;

    int bufferSize = 3 * sizeof(*extGLBufVertices) * verticesCount;

    glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS

    //Pass vertices to shader
    glGenBuffers(1, &intGlBufVerticies);                                         CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, intGlBufVerticies);                            CHECK_GL_ERRORS
	glBufferData(GL_ARRAY_BUFFER, bufferSize, extGLBufVertices, GL_STATIC_DRAW); CHECK_GL_ERRORS
    verticesLocation = glGetAttribLocation(shaderProgram, "position");           CHECK_GL_ERRORS
    glVertexAttribPointer(verticesLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);        CHECK_GL_ERRORS
    glEnableVertexAttribArray(verticesLocation);                                 CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                            CHECK_GL_ERRORS

    //Pass colors to shader
    glGenBuffers(1, &intGLBufColors);                                            CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, intGLBufColors);                               CHECK_GL_ERRORS
	glBufferData(GL_ARRAY_BUFFER, bufferSize, extGLBufColors, GL_STATIC_DRAW);   CHECK_GL_ERRORS
    colorsLocation = glGetAttribLocation(shaderProgram, "color");                CHECK_GL_ERRORS
    glVertexAttribPointer(colorsLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);          CHECK_GL_ERRORS
    glEnableVertexAttribArray(colorsLocation);                                   CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                            CHECK_GL_ERRORS

    glBindVertexArray(0);                                                        CHECK_GL_ERRORS
}

void DrawCornellBox(SDL_Window * window) {
    //Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                          CHECK_GL_ERRORS

	//Use buffers
	glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS

    glDrawArrays(GL_LINES_ADJACENCY, 0, verticesCount);                          CHECK_GL_ERRORS
    SDL_GL_SwapWindow(window);                                                   CHECK_GL_ERRORS
    glBindVertexArray(0);                                                        CHECK_GL_ERRORS
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
            }
        }
        DrawCornellBox(window);
    }

    //Destroy window and release resources
    SDL_DestroyWindow(window);

    //End work with SDL
    SDL_Quit();
    return 0;
}
