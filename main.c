#include "SDL.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

//#include "../GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _WIN32
#include <windows.h>
#include "../GL/wglext.h"
#endif
#include "../GL/glext.h"

#include "../clew/clew.h"

#include <cl_gl.h>

//Include my libraries
#include "types.h"
#include "parameters.h"
#include "glElements.h"
#include "clElements.h"


void ReadCornellBox() {
    //Open file with scene
    FILE *file = fopen("scenes\\1.txt", "r");
    if (!file) {
        file = fopen("scenes/1.txt", "r");
    }
    //Read count of polygons in scene
    fscanf(file, "%d", &polygonCount);

    //Array of polygons in scene
    scene = calloc(polygonCount, sizeof(*scene));
    verticesCount = 0;
    //Array of patched polygons of scene
    pt_scene = calloc(polygonCount, sizeof(*pt_scene));
    ptVerticesCount = 0;

    patchCount = 0;

    for (int i = 0; i < polygonCount; ++i) {
        scene[i] = readPolygon(file);
        pt_scene[i] = SplitPolygonToPatches(scene[i], PATCH_COUNT);
        //Update verticies count
        verticesCount += scene[i].length;

        //Update verticies count for patches
        for (int j = 0; j < pt_scene[i].axis1 * pt_scene[i].axis2; ++j) {
            ptVerticesCount += pt_scene[i].patches[j].length;
        }
        patchCount += pt_scene[i].axis1 * pt_scene[i].axis2;
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
    actualShadowMap = true;
}

//Add buffers and other data to OpenGL
void PrepairGLBuffers() {
    //Add Vertex Array Object, which contains all buffers
    glGenVertexArrays(1, &meshVAO);                                              CHECK_GL_ERRORS

    //Initialization of external buffers
    extGLBufVertices = calloc(3 * (ptVerticesCount - 2), 4 * sizeof(*extGLBufVertices));
    extGLBufAmbCol   = calloc(3 * (ptVerticesCount - 2), 3 * sizeof(*extGLBufAmbCol));
    extGLBufDifCol   = calloc(3 * (ptVerticesCount - 2), 3 * sizeof(*extGLBufDifCol));
    extGLBufSpcCol   = calloc(3 * (ptVerticesCount - 2), 3 * sizeof(*extGLBufSpcCol));
    extGLBufNormInds = calloc(3 * (ptVerticesCount - 2), sizeof(*extGLBufNormInds));
}


void PrepairShadowMap() {
    //Free old resources
    glDeleteFramebuffers(1, &fbo);                                               CHECK_GL_ERRORS
    glDeleteTextures(1, &shadowMap);                                             CHECK_GL_ERRORS

	//Create framebuffer
    glGenFramebuffers(1, &fbo);                                                  CHECK_GL_ERRORS
    //Create shadow map
    glGenTextures(1, &shadowMap);                                                CHECK_GL_ERRORS
    //Create texture for shadow map
    glBindTexture(GL_TEXTURE_2D, shadowMap);                                     CHECK_GL_ERRORS
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, SHADOWMAP_EDGE,
				SHADOWMAP_EDGE, 0, GL_RGBA, GL_FLOAT, NULL);           CHECK_GL_ERRORS
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);            CHECK_GL_ERRORS
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);            CHECK_GL_ERRORS
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);         CHECK_GL_ERRORS
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);         CHECK_GL_ERRORS

	GLuint renderBuffer;

	glGenRenderbuffers(1, &renderBuffer);                                        CHECK_GL_ERRORS
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);                           CHECK_GL_ERRORS
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, SHADOWMAP_EDGE, SHADOWMAP_EDGE); CHECK_GL_ERRORS
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);                                      CHECK_GL_ERRORS
	glBindTexture(GL_TEXTURE_2D, shadowMap);                                     CHECK_GL_ERRORS
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                          GL_DEPTH_ATTACHMENT,
                          GL_RENDERBUFFER,
                          renderBuffer);                                         CHECK_GL_ERRORS

	//Bind shadowmap to framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);                                 CHECK_GL_ERRORS
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
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
	glUseProgram(shadowProgram);
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

    SetUniformMat4f(shadowProgram, "camMatrix", lightMatrix);

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
    SetUniform3f(shaderProgram, "lg.spotPosition", spotLightPosition);
    SetUniform3f(shaderProgram, "lg.spotDirection", spotLightDirection);
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
    GLuint vertexShader, fragmentShader;

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
    if (ShaderProgramStatus(shaderProgram, GL_LINK_STATUS) != GL_TRUE) {
        fprintf(stderr, "BUGURT!!! Line: %d\n", __LINE__);
        exit(1);
    }

    //Shadow Map program
    GLuint shadowVert, shadowFrag;

	CompileShader("shaders/shadowMap.vert", &shadowVert, GL_VERTEX_SHADER);
	CompileShader("shaders/shadowMap.frag", &shadowFrag, GL_FRAGMENT_SHADER);

    shadowProgram = glCreateProgram();                                           CHECK_GL_ERRORS

    glAttachShader(shadowProgram, shadowVert);                                   CHECK_GL_ERRORS
    glAttachShader(shadowProgram, shadowFrag);                                   CHECK_GL_ERRORS

    glLinkProgram(shadowProgram);                                                CHECK_GL_ERRORS

    //Choose our program to execution
    glUseProgram(shaderProgram);                                                 CHECK_GL_ERRORS

    //Enable z-buffer
    glEnable(GL_DEPTH_TEST);                                                     CHECK_GL_ERRORS

    PrepairGLBuffers();
    AddGLUniforms();
    PrepairShadowMap();
}


void PrepairCLKernels(cl_program program) {

    computeLightEmission = clCreateKernel(program, "ComputeLightEmission", &cl_err); CHECK_CL(cl_err);
	CHECK_CL(clSetKernelArg(computeLightEmission, 0, sizeof(halfCLExcident), &halfCLExcident));
	CHECK_CL(clSetKernelArg(computeLightEmission, 1, sizeof(intCLPatchesPoints), &intCLPatchesPoints));
    CHECK_CL(clSetKernelArg(computeLightEmission, 2, sizeof(intCLLightMatrix), &intCLLightMatrix));
    CHECK_CL(clSetKernelArg(computeLightEmission, 3, sizeof(intCLLightParameters), &intCLLightParameters));
    CHECK_CL(clSetKernelArg(computeLightEmission, 4, sizeof(clShadowMap), &clShadowMap));
    CHECK_CL(clSetKernelArg(computeLightEmission, 5, sizeof(halfCLReflection), &halfCLReflection));

    convertROFloatToHalf = clCreateKernel(program, "FloatToHalfROBuffers", &cl_err); CHECK_CL(cl_err);
    CHECK_CL(clSetKernelArg(convertROFloatToHalf, 0, sizeof(intCLFormFactor), &intCLFormFactor));
    CHECK_CL(clSetKernelArg(convertROFloatToHalf, 1, sizeof(halfCLFormFactor), &halfCLFormFactor));
    CHECK_CL(clSetKernelArg(convertROFloatToHalf, 2, sizeof(intCLIndices), &intCLIndices));
    CHECK_CL(clSetKernelArg(convertROFloatToHalf, 3, sizeof(intCLMatReflection), &intCLMatReflection));
    CHECK_CL(clSetKernelArg(convertROFloatToHalf, 4, sizeof(halfCLReflection), &halfCLReflection));

    sendRays = clCreateKernel(program, "SendRays", &cl_err);                     CHECK_CL(cl_err);
    CHECK_CL(clSetKernelArg(sendRays, 0, sizeof(halfCLExcident), &halfCLExcident));
    CHECK_CL(clSetKernelArg(sendRays, 1, sizeof(halfCLFormFactor), &halfCLFormFactor));
    CHECK_CL(clSetKernelArg(sendRays, 2, sizeof(halfCLReflection), &halfCLReflection));
    CHECK_CL(clSetKernelArg(sendRays, 3, sizeof(halfCLIncident), &halfCLIncident));

    sendRaysV3 = clCreateKernel(program, "SendRaysV3", &cl_err);                 CHECK_CL(cl_err);
    CHECK_CL(clSetKernelArg(sendRaysV3, 0, sizeof(halfCLExcident), &halfCLExcident));
    CHECK_CL(clSetKernelArg(sendRaysV3, 1, sizeof(halfCLFormFactor), &halfCLFormFactor));
    CHECK_CL(clSetKernelArg(sendRaysV3, 2, sizeof(halfCLPreIncident), &halfCLPreIncident));

    cl_int optimSize = patchCount / OPTIMIZE_CONST;

    reduceIncident = clCreateKernel(program, "ReduceIncident", &cl_err);         CHECK_CL(cl_err);
    CHECK_CL(clSetKernelArg(reduceIncident, 0, sizeof(halfCLPreIncident), &halfCLPreIncident));
    CHECK_CL(clSetKernelArg(reduceIncident, 1, sizeof(optimSize), &optimSize));

    replaceIncident = clCreateKernel(program, "ReplaceIncident", &cl_err);       CHECK_CL(cl_err);
    CHECK_CL(clSetKernelArg(replaceIncident, 0, sizeof(halfCLPreIncident), &halfCLPreIncident));
    CHECK_CL(clSetKernelArg(replaceIncident, 1, sizeof(halfCLReflection), &halfCLReflection));
    CHECK_CL(clSetKernelArg(replaceIncident, 2, sizeof(halfCLIncident), &halfCLIncident));
    CHECK_CL(clSetKernelArg(replaceIncident, 3, sizeof(optimSize), &optimSize));
}


void PrepairCLBuffers() {
	//Buffer for form-factors
    intCLFormFactor = clCreateBuffer(clContext, CL_MEM_READ_ONLY,
                sizeof(*formFactors) * patchCount * patchCount, NULL, &cl_err);  CHECK_CL(cl_err);

	//Buffer for form-factors with half presition for speed
    halfCLFormFactor = clCreateBuffer(clContext, CL_MEM_READ_WRITE,
                            HALFSIZE * patchCount * patchCount, NULL, &cl_err);  CHECK_CL(cl_err);

	//Buffer for view matrix from light position
	intCLLightMatrix = clCreateBuffer(clContext, CL_MEM_READ_ONLY,
									sizeof(*lightMatrix) * 16, NULL, &cl_err);         CHECK_CL(cl_err);

    //Buffer for light source parameters: Inner angle, outter angle, light position
    intCLLightParameters = clCreateBuffer(clContext, CL_MEM_READ_ONLY,
					sizeof(float) * LIGHT_PARAMETERS_COUNT, NULL, &cl_err);      CHECK_CL(cl_err);

	//Buffer for emission of patches
	halfCLExcident = clCreateBuffer(clContext, CL_MEM_READ_WRITE,
									HALFSIZE * 4 * patchCount, NULL, &cl_err);   CHECK_CL(cl_err);

	//Buffer for patches vertices
	intCLPatchesPoints = clCreateFromGLBuffer(clContext, CL_MEM_READ_WRITE,
										          intGlBufVerticies, &cl_err);   CHECK_CL(cl_err);

	//Texture width shadow map
    clShadowMap = clCreateFromGLTexture2D(clContext, CL_MEM_READ_WRITE,
										GL_TEXTURE_2D, 0, shadowMap, &cl_err);   CHECK_CL(cl_err);

    //Buffer for reflection of patches
    halfCLReflection = clCreateBuffer(clContext, CL_MEM_READ_WRITE,
                                    HALFSIZE * 4 * patchCount, NULL, &cl_err);   CHECK_CL(cl_err);

    //Indices for patches
    intCLIndices = clCreateBuffer(clContext, CL_MEM_READ_ONLY,
                                    sizeof(int) * patchCount, NULL, &cl_err);    CHECK_CL(cl_err);

    //Reflections of materials
    intCLMatReflection = clCreateBuffer(clContext, CL_MEM_READ_ONLY,
                  sizeof(scene[0].mat.ambient) * polygonCount, NULL, &cl_err);   CHECK_CL(cl_err);

    //Incident buffer, which equal deposit for one iteration of radiosity
    halfCLIncident = clCreateFromGLBuffer(clContext, CL_MEM_READ_WRITE,
                                                    intGLRadiosity, &cl_err);    CHECK_CL(cl_err);

    //Buffer needed for optimization which count incident vector by parallel reduction
    halfCLPreIncident = clCreateBuffer(clContext, CL_MEM_READ_WRITE,
        HALFSIZE * patchCount * patchCount * 4 / OPTIMIZE_CONST, NULL, &cl_err); CHECK_CL(cl_err);

}


void FillCLBuffers() {
    //Fill form-factors
    CHECK_CL(clEnqueueWriteBuffer(clProg, intCLFormFactor, CL_TRUE, 0, sizeof(*formFactors) * patchCount * patchCount, formFactors, 0, NULL, NULL));

    //Fill indices
    int indices[patchCount];
    for (int i = 0; i < polygonCount; ++i) {
        for (int j = 0; j < PATCH_COUNT * PATCH_COUNT; ++j) {
            indices[i * PATCH_COUNT * PATCH_COUNT + j] = i;
        }
    }
    CHECK_CL(clEnqueueWriteBuffer(clProg, intCLIndices, CL_TRUE, 0, sizeof(*indices) * patchCount, indices, 0, NULL, NULL));

    //Fill material reflectance
    vec4 refl[polygonCount];
    for (int i = 0; i < polygonCount; ++i) {
        refl[i] = scene[i].mat.ambient;
        refl[i].w = 0;
    }
    CHECK_CL(clEnqueueWriteBuffer(clProg, intCLMatReflection, CL_TRUE, 0, sizeof(*refl) * polygonCount, refl, 0, NULL, NULL));

    //Fill half form-factors and reflectance on GPU
    CHECK_CL(clEnqueueNDRangeKernel(clProg, convertROFloatToHalf, 1, 0, &patchCount, NULL, 0, NULL, NULL));
	CHECK_CL(clFinish(clProg));
}

void PrepairOpenCL() {
    clewInit(L"OpenCL.dll");

    cl_device_id device_id;             // compute device id
	cl_platform_id platforms[30];
	int platfcnt;
    int err;

    CHECK_CL(clGetPlatformIDs(30, platforms, &platfcnt));

    CHECK_CL(clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &device_id, NULL));

    char clInfo[256 * 4];
    clGetDeviceInfo(device_id, CL_DEVICE_OPENCL_C_VERSION, 256 * 4, clInfo, NULL);
    printf("%s\n", clInfo);
    clGetDeviceInfo(device_id, CL_DEVICE_NAME, 256 * 4, clInfo, NULL);
    printf("%s\n", clInfo);

    cl_context_properties properties[] = {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), // WGL Context
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), // WGL HDC
    CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0],// OpenCL platform
    0 };

    // Create a context using the supported devices
    clContext = clCreateContext(properties, 1, &device_id, NULL, 0, &cl_err);      CHECK_CL(cl_err);

    clProg = clCreateCommandQueue(clContext, device_id, 0, &cl_err);               CHECK_CL(cl_err);

    //Load source
    char *KernelSource;
	int sourceLen;
    LoadSource("kernels/kernel", &KernelSource, &sourceLen);

    //Create program
    cl_program program;
    program = clCreateProgramWithSource(clContext, 1,
                                        &KernelSource, &sourceLen, &cl_err);     CHECK_CL(cl_err);
    //Compile program
    CHECK_CL(err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL));
    size_t log_size;
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *log = (char *) malloc(log_size);
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
    fprintf(stderr, "%s\n", log);
    if (err == CL_BUILD_PROGRAM_FAILURE) {
        exit(1);
    }

    PrepairCLBuffers();
    PrepairCLKernels(program);
    FillCLBuffers();
}

void PassCornellBoxDataToGL() {
    //Set vertices and normals to external gl buffers
    for (int i = 0, vertIter = 0; i < polygonCount; ++i) {
        for (int h = 0; h < pt_scene[i].axis1 * pt_scene[i].axis2; ++h) {
            patch * pt = &pt_scene[i].patches[h];
            for (int j = 1, vertShift = 0; j < pt->length - 1; ++j) {
                if (i == 5) {
                    vertShift += 4;
                    continue;
                }
                //Add first vertex
                extGLBufVertices[4 * (vertIter + vertShift) + 0] = pt->vertices[0].x;
                extGLBufVertices[4 * (vertIter + vertShift) + 1] = pt->vertices[0].y;
                extGLBufVertices[4 * (vertIter + vertShift) + 2] = pt->vertices[0].z;
                //Add index of normal
                extGLBufNormInds[vertIter + vertShift] = i;
                vertShift++;
                //Add second vertex
                extGLBufVertices[4 * (vertIter + vertShift) + 0] = pt->vertices[j].x;
                extGLBufVertices[4 * (vertIter + vertShift) + 1] = pt->vertices[j].y;
                extGLBufVertices[4 * (vertIter + vertShift) + 2] = pt->vertices[j].z;
                //Add index of normal
                extGLBufNormInds[vertIter + vertShift] = i;
                vertShift++;
                //Add third vertex
                extGLBufVertices[4 * (vertIter + vertShift) + 0] = pt->vertices[j + 1].x;
                extGLBufVertices[4 * (vertIter + vertShift) + 1] = pt->vertices[j + 1].y;
                extGLBufVertices[4 * (vertIter + vertShift) + 2] = pt->vertices[j + 1].z;
                //Add index of normal
                extGLBufNormInds[vertIter + vertShift] = i;
                vertShift++;
            }
            vertIter += 3 * (pt->length - 2);
        }
    }

    //Locations for buffers
    GLint verticesLocation, normalsLocation, radiosityLocation;

    //int bufferSize = 3 * sizeof(*extGLBufVertices) * 3 * (verticesCount - 2);
    int bufferSize = 4 * sizeof(*extGLBufVertices) * 3 * (ptVerticesCount - 2);

    glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS

    //Pass vertices to shader
    glGenBuffers(1, &intGlBufVerticies);                                         CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, intGlBufVerticies);                            CHECK_GL_ERRORS
	glBufferData(GL_ARRAY_BUFFER, bufferSize, extGLBufVertices, GL_STATIC_DRAW); CHECK_GL_ERRORS
	verticesLocation = glGetAttribLocation(shaderProgram, "position");           CHECK_GL_ERRORS
	if (verticesCount != -1) {
		glVertexAttribPointer(verticesLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);    CHECK_GL_ERRORS
		glEnableVertexAttribArray(verticesLocation);                             CHECK_GL_ERRORS
	} else {
        fprintf(stderr, "Loaction position not found\n");
	}
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                            CHECK_GL_ERRORS

    //Pass normals to shader
    glGenBuffers(1, &intGLBufNormInds);                                          CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, intGLBufNormInds);                             CHECK_GL_ERRORS
    glBufferData(GL_ARRAY_BUFFER,
              3 * (ptVerticesCount - 2) * sizeof(*extGLBufNormInds),
              extGLBufNormInds, GL_STATIC_DRAW);                                 CHECK_GL_ERRORS
	normalsLocation = glGetAttribLocation(shaderProgram, "v_polInd");            CHECK_GL_ERRORS
    if (normalsLocation != -1) {
		glVertexAttribIPointer(normalsLocation, 1, GL_UNSIGNED_BYTE, 0, 0);      CHECK_GL_ERRORS
		glEnableVertexAttribArray(normalsLocation);                              CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location v_polInd not found\n");
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                            CHECK_GL_ERRORS

    //Radiosity part buffer
    glGenBuffers(1, &intGLRadiosity);                                            CHECK_GL_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, intGLRadiosity);                               CHECK_GL_ERRORS
    glBufferData(GL_ARRAY_BUFFER, 3 * (ptVerticesCount - 2) * sizeof(vec4) / 2
                 , NULL, GL_STATIC_DRAW);                                        CHECK_GL_ERRORS
    radiosityLocation = glGetAttribLocation(shaderProgram, "radioPart");         CHECK_GL_ERRORS
    if (radiosityLocation != -1) {
		glVertexAttribPointer(radiosityLocation, 4, GL_HALF_FLOAT, GL_FALSE, sizeof(vec4) / 2, 0);   CHECK_GL_ERRORS
		glEnableVertexAttribArray(radiosityLocation);                            CHECK_GL_ERRORS
    } else {
        fprintf(stderr, "Location radioPart not found\n");
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);                                            CHECK_GL_ERRORS

    glBindVertexArray(0);                                                        CHECK_GL_ERRORS
}

void PassShadowMap() {
    //Bind shadowmap to framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);                                 CHECK_GL_ERRORS
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
							GL_TEXTURE_2D, shadowMap, 0);                        CHECK_GL_ERRORS

    glViewport(0, 0, SHADOWMAP_EDGE, SHADOWMAP_EDGE);                            CHECK_GL_ERRORS
	glUseProgram(shadowProgram);                                                 CHECK_GL_ERRORS
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                          CHECK_GL_ERRORS
	glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS
	glDrawArrays(GL_TRIANGLES, 0, 3 * (ptVerticesCount - 2));                    CHECK_GL_ERRORS

	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);                    CHECK_GL_ERRORS
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "FB error, status: 0x%x\n", Status);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);                                   CHECK_GL_ERRORS

	glUseProgram(shaderProgram);                                                 CHECK_GL_ERRORS
    glActiveTexture(GL_TEXTURE0);                                                CHECK_GL_ERRORS
    glBindTexture(GL_TEXTURE_2D, shadowMap);                                     CHECK_GL_ERRORS
    SetUniform1i(shaderProgram, "shadowMap", 0);
    glUseProgram(0);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void ComputeEmission() {
	//Capture gl buffer with patches veritces
	CHECK_CL(clEnqueueAcquireGLObjects(clProg, 1, &intCLPatchesPoints, 0, 0, 0));
	//Pass light matrix to GPU
	CHECK_CL(clEnqueueWriteBuffer(clProg, intCLLightMatrix, CL_TRUE, 0, sizeof(*lightMatrix) * 16, lightMatrix, 0, NULL, NULL));
	//Pass light parameters to GPU
    lightParams[0] = cos(spotAngIn * M_PI / 180.0f);
    lightParams[1] = cos(spotAngOut * M_PI / 180.0f);
    lightParams[2] = spotLightPosition.x;
    lightParams[3] = spotLightPosition.y;
    lightParams[4] = spotLightPosition.z;
    lightParams[5] = spotLightDirection.x;
    lightParams[6] = spotLightDirection.y;
    lightParams[7] = spotLightDirection.z;
	CHECK_CL(clEnqueueWriteBuffer(clProg, intCLLightParameters, CL_TRUE, 0, sizeof(lightParams), lightParams, 0, NULL, NULL));

	//Capture shadow map texture
    CHECK_CL(clEnqueueAcquireGLObjects(clProg, 1, &clShadowMap, 0, 0, 0));

	//Run kernel
	CHECK_CL(clEnqueueNDRangeKernel(clProg, computeLightEmission, 1, 0, &patchCount, NULL, 0, NULL, NULL));
	CHECK_CL(clFinish(clProg));

    //Free shadow map
    CHECK_CL(clEnqueueReleaseGLObjects(clProg, 1, &clShadowMap, 0, 0, 0));

    //Free gl buffer
    CHECK_CL(clEnqueueReleaseGLObjects(clProg, 1, &intCLPatchesPoints, 0, 0, 0));
}

void ComputeRadiosity() {
    CHECK_CL(clEnqueueAcquireGLObjects(clProg, 1, &halfCLIncident, 0, 0, 0));

    CHECK_CL(clEnqueueNDRangeKernel(clProg, sendRays, 1, 0, &patchCount, NULL, 0, NULL, NULL));
	CHECK_CL(clFinish(clProg));

	CHECK_CL(clEnqueueReleaseGLObjects(clProg, 1, &halfCLIncident, 0, 0, 0));
}

void ComputeRadiosityOptimize() {
    CHECK_CL(clEnqueueAcquireGLObjects(clProg, 1, &halfCLIncident, 0, 0, 0));

    cl_event event;
    int sizes[] = {patchCount, patchCount / OPTIMIZE_CONST};
    CHECK_CL(clEnqueueNDRangeKernel(clProg, sendRaysV3, 2, 0, sizes, NULL, 0, NULL, &event));
    CHECK_CL(clWaitForEvents(1, &event));

    /*sizes[1] = patchCount / 2 / OPTIMIZE_CONST;
    CHECK_CL(clEnqueueNDRangeKernel(clProg, reduceIncident, 2, 0, sizes, NULL, 0, NULL, NULL));
    CHECK_CL(clFinish(clProg));
    sizes[1] = sizes[1] / 2 + sizes[1] % 2;
    CHECK_CL(clEnqueueNDRangeKernel(clProg, reduceIncident, 2, 0, sizes, NULL, 0, NULL, NULL));
    CHECK_CL(clFinish(clProg));*/
    for (int i = patchCount / 2 / OPTIMIZE_CONST; i > 1; i = i / 2 + i % 2) {
        sizes[1] = i;
        CHECK_CL(clEnqueueNDRangeKernel(clProg, reduceIncident, 2, 0, sizes, NULL, 0, NULL, &event));
        CHECK_CL(clWaitForEvents(1, &event));
    }
    //printf("OK\n");
    CHECK_CL(clEnqueueNDRangeKernel(clProg, replaceIncident, 1, 0, &patchCount, NULL, 0, NULL, NULL));
    CHECK_CL(clFinish(clProg));
	CHECK_CL(clEnqueueReleaseGLObjects(clProg, 1, &halfCLIncident, 0, 0, 0));
}

float computeFormFactorForPatches(patch p1, patch p2, int pl1, int pl2) {
	float result = 0;
	int cnt = 0;
	float xx[MONTE_KARLO_ITERATIONS_COUNT];
	float yy[MONTE_KARLO_ITERATIONS_COUNT];
	for (int i = 0; i < MONTE_KARLO_ITERATIONS_COUNT; ++i) {
        //Hammersley Point Set
        float u = 0;
        int kk = i;

        for (float p = 0.5; kk; p *= 0.5, kk >>= 1)
            if (kk & 1)
                u += p;

        float v = (i + 0.5) / MONTE_KARLO_ITERATIONS_COUNT;
        xx[i] = u;
        yy[i] = v;
	}
    for (int i = 0; i < MONTE_KARLO_ITERATIONS_COUNT; ++i) {
        for (int j = 0; j < MONTE_KARLO_ITERATIONS_COUNT; ++j) {
            float iter_res = 0;

            vec3 on_p1 = sum(p1.vertices[0], sum(mult(sub(p1.vertices[1], p1.vertices[0]), xx[i]), mult(sub(p1.vertices[3], p1.vertices[0]), yy[i])));
            vec3 on_p2 = sum(p2.vertices[0], sum(mult(sub(p2.vertices[1], p2.vertices[0]), xx[j]), mult(sub(p2.vertices[3], p2.vertices[0]), yy[j])));

            vec3 r = sub(on_p1, on_p2);
            float lr = length(r);
            if (fabs(lr) < DBL_EPSILON) {
                cnt++;
                continue;
            }

            //Visibility function
            int flag = 0;
            for (int j = 0; j < polygonCount && !flag; ++j) {
                if (j == pl1 || j == pl2) continue;
                flag = checkIntersection(scene[j], on_p1, on_p2);
            }
            if (flag)
                continue;

            iter_res = cosV(r, p2.normal);
            iter_res *= cosV(mult(r, -1), p1.normal);
            if (iter_res < 0) continue;
            iter_res /=  lr * lr;
            if (iter_res > 10) continue;
            result += iter_res;
        }
    }
    result /= MONTE_KARLO_ITERATIONS_COUNT * MONTE_KARLO_ITERATIONS_COUNT - cnt;
    result /= M_PI;
	result *= square(p1) * square(p2);
    return result;
}

int computeFormFactorForPolygons(int p1, int p2) {
    for (int i1 = 0; i1 < pt_scene[p1].axis1 * pt_scene[p1].axis2; ++i1) {
		for (int i2 = 0; i2 < pt_scene[p2].axis1 * pt_scene[p2].axis2; ++i2) {
			int ff_ind1 = (p2 * PATCH_COUNT * PATCH_COUNT + i2) * patchCount
						+ p1 * PATCH_COUNT * PATCH_COUNT + i1;
			int ff_ind2 = (p1 * PATCH_COUNT * PATCH_COUNT + i1) * patchCount
						+ p2 * PATCH_COUNT * PATCH_COUNT + i2;
			float ff = computeFormFactorForPatches(pt_scene[p1].patches[i1],
											pt_scene[p2].patches[i2], p1, p2);
			formFactors[ff_ind1] = ff / square(pt_scene[p1].patches[i1]);
			formFactors[ff_ind2] = ff / square(pt_scene[p2].patches[i2]);
        }
    }
    return 1;
}

int computeFormFactorForScene() {
	printf("FF start\n");
    for (int i = 0; i < polygonCount; ++i) {
        for (int j = i + 1; j < polygonCount; ++j) {
			clock_t tm = clock();
            computeFormFactorForPolygons(i, j);
            printf("%d %d %f\n", i, j, (float)(clock() - tm) / CLOCKS_PER_SEC);
        }
    }
    return 1;
}

void ReadOrComputeFormFactors() {
	formFactors = calloc(sizeof(*formFactors), patchCount * patchCount);
	char ff_file[50];
    sprintf(ff_file, "ff\\%d", patchCount);
    FILE *formfactfile;
    if ((formfactfile = fopen(ff_file, "rb")) == NULL) {
		computeFormFactorForScene();
        formfactfile = fopen(ff_file, "wb");
        for (int i = 0; i < patchCount; ++i) {
            for (int j = 0; j < patchCount; ++j) {
				fwrite(&formFactors[i * patchCount + j], sizeof(formFactors[i * patchCount + j]), 1, formfactfile);
            }
        }
    } else {
    	printf("Form-factor already computed\n");
		for (int i = 0; i < patchCount; ++i) {
            for (int j = 0; j < patchCount; ++j) {
				fread(&formFactors[i * patchCount + j], sizeof(formFactors[i * patchCount + j]), 1, formfactfile);
            }
        }
    }
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
		SetUniformsForShadowMap();
		PassShadowMap();
		SetStandartCamera();
		ComputeEmission();
		//ComputeRadiosity();
		ComputeRadiosityOptimize();
		//actualShadowMap = true;

	}
	actualShadowMap = false;
	glUseProgram(shaderProgram);                                                 CHECK_GL_ERRORS
	//Use buffers
	glBindVertexArray(meshVAO);                                                  CHECK_GL_ERRORS

	//Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                          CHECK_GL_ERRORS

    glDrawArrays(GL_TRIANGLES, 0, 3 * (ptVerticesCount - 2));                    CHECK_GL_ERRORS
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
    ReadOrComputeFormFactors();
    PrepairOpenCL();

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
        static int t = 0;
        //if (t) exit(0);
        t++;
    }
    //Destroy window and release resources
    SDL_DestroyWindow(window);

    //End work with SDL
    SDL_Quit();
    return 0;
}
