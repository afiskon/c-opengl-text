#include <GLXW/glxw.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "utils/definitions.h"
#include "utils/linearalg.h"

#include "utils/utils.h"
#include "utils/camera.h"
#include "utils/models.h"

// usually about 53 chars is enough, using 128 to be safe
static char STATUS_LINE_BUFF[128]; 

static const Vector POINT_LIGHT_POS = {{ -2.0f, 3.0f, 0.0f, 0.0f }};
static const Vector SPOT_LIGHT_POS = {{ 4.0f, 5.0f, 0.0f, 0.0f }};
static const Vector CAMERA_START_POS = {{ 0.0f, 0.0f, 5.0f, 0.0f }};

#define KEY_PRESS_CHECK_INTERVAL 250 // ms

// [0.0, 1.0], larger - more smoothing
#define FPS_SMOOTHING 0.95f 

#define TEXTURES_NUM 6
#define VAOS_NUM 6
#define VBOS_NUM 11

typedef struct
{
	bool windowInitialized;
	bool cameraInitialized;
	bool programIdInitialized;
	bool textProgramIdInitialized;
	bool textureArrayInitialized;
	bool vaoArrayInitialized;
	bool vboArrayInitialized;

	GLFWwindow* window;
	Camera* camera;
	GLuint programId;
	GLuint textProgramId;
	GLuint textureArray[TEXTURES_NUM];
	GLuint vaoArray[VAOS_NUM];
	GLuint vboArray[VBOS_NUM];
} CommonResources;

static void
windowSizeCallback(GLFWwindow * window, int width, int height)
{
	UNUSED(window);
	glViewport(0, 0, width, height);
}

static void
errorCallback(int code, const char* descr)
{
	fprintf(stderr, "ERROR: code = %d, descr = %s\n", code, descr);
}

static int
commonResourcesCreate(CommonResources* resources)
{
	// set *Initialized fields to false

	memset(resources, 0, sizeof(CommonResources));

	// initialize window

	if(glfwInit() == GL_FALSE)
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwSetErrorCallback(errorCallback);

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	glfwWindowHint(GLFW_SAMPLES, 4);

	resources->window = glfwCreateWindow(800, 600, "Demo", NULL, NULL);

	if(resources->window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window\n");
		return -1;
	}
	resources->windowInitialized = true;

	glfwMakeContextCurrent(resources->window);

	if(glxwInit())
	{
		fprintf(stderr, "Failed to init GLXW\n");
		return -1;
	}

	glfwSwapInterval(1);
	glfwSetWindowSizeCallback(resources->window, windowSizeCallback);
	glfwShowWindow(resources->window);

	// initialize camera
	resources->camera = cameraCreate(
			resources->window,
			CAMERA_START_POS,
			3.14f, // toward -Z
			0.0f // look at the horizon
		);

	if(!resources->camera)
	{
		fprintf(stderr, "Failed to create camera\n");
		return -1;
	}

	resources->cameraInitialized = true;

	// initialize programId

	bool errorFlag;

	// vertexShader, fragmentShader
	GLuint shaders[2];

	shaders[0] = loadShader("shaders/vertexShader.glsl",
		GL_VERTEX_SHADER,
		&errorFlag);
	if(errorFlag) {
		fprintf(stderr, "Failed to load vertex shader (invalid working "
			"directory?)\n");
		return -1;
	}

	shaders[1] = loadShader("shaders/fragmentShader.glsl",
		GL_FRAGMENT_SHADER,
		&errorFlag);
	if(errorFlag) {
		fprintf(stderr, "Failed to load fragment shader (invalid working "
			"directory?)\n");
		glDeleteShader(shaders[0]);
		return -1;
	}

	resources->programId = prepareProgram(
		shaders, sizeof(shaders)/sizeof(shaders[0]), &errorFlag);

	glDeleteShader(shaders[1]);
	glDeleteShader(shaders[0]);

	if(errorFlag) {
		fprintf(stderr, "Failed to prepare program\n");
		return -1;
	}

	resources->programIdInitialized = true;

	// initialize textProgramId

	shaders[0] = loadShader("shaders/textVertexShader.glsl",
		GL_VERTEX_SHADER,
		&errorFlag);
	if(errorFlag) {
		fprintf(stderr, "Failed to load text vertex shader (invalid working "
			"directory?)\n");
		return -1;
	}

	shaders[1] = loadShader("shaders/textFragmentShader.glsl",
		GL_FRAGMENT_SHADER,
		&errorFlag);
	if(errorFlag) {
		fprintf(stderr, "Failed to load text fragment shader (invalid working "
			"directory?)\n");
		glDeleteShader(shaders[0]);
		return -1;
	}

	resources->textProgramId = prepareProgram(
		shaders, sizeof(shaders)/sizeof(shaders[0]), &errorFlag);

	glDeleteShader(shaders[1]);
	glDeleteShader(shaders[0]);

	if(errorFlag) {
		fprintf(stderr, "Failed to prepare text program\n");
		return -1;
	}

	resources->textProgramIdInitialized = true;

	// initialize textureArray
	glGenTextures(TEXTURES_NUM, resources->textureArray);
	resources->textureArrayInitialized = true;

	// initialize vaoArray
	glGenVertexArrays(VAOS_NUM, resources->vaoArray);
	resources->vaoArrayInitialized = true;

	// initialize vboArray
	glGenBuffers(VBOS_NUM, resources->vboArray);
	resources->vboArrayInitialized = true;

	return 0;
}

static void
commonResourcesDestroy(CommonResources* resources)
{
	if(resources->windowInitialized)
		glfwDestroyWindow(resources->window);

	if(resources->cameraInitialized)
		cameraDestroy(resources->camera);

	if(resources->programIdInitialized)
		glDeleteProgram(resources->programId);

	if(resources->textProgramIdInitialized)
		glDeleteProgram(resources->textProgramId);

	if(resources->textureArrayInitialized)
		glDeleteTextures(TEXTURES_NUM, resources->textureArray);
	
	if(resources->vaoArrayInitialized)
		glDeleteVertexArrays(VAOS_NUM, resources->vaoArray);

	if(resources->vboArrayInitialized)
		glDeleteBuffers(VBOS_NUM, resources->vboArray);

	glfwTerminate();
}

static void
setupLights(GLuint programId, bool directionalLightEnabled,
	bool pointLightEnabled, bool spotLightEnabled)
{
	{
		Vector direction = {{ 0.0f, -1.0f, 1.0f, 0.0f }};
		vectorNormalizeInplace(&direction);

		setUniform3f(programId, "directionalLight.direction",
			direction.x, direction.y, direction.z);
		setUniform3f(programId, "directionalLight.color",
			1.0f, 1.0f, 1.0f);
		setUniform1f(programId, "directionalLight.ambientIntensity",
			((float)directionalLightEnabled)*0.1f);
		setUniform1f(programId, "directionalLight.diffuseIntensity",
			((float)directionalLightEnabled)*0.1f);
		setUniform1f(programId, "directionalLight.specularIntensity",
			((float)directionalLightEnabled)*1.0f);
	}

	{
		setUniform3f(programId, "pointLight.position",
			POINT_LIGHT_POS.x, POINT_LIGHT_POS.y, POINT_LIGHT_POS.z);
		setUniform3f(programId, "pointLight.color",
			1.0f, 0.0f, 0.0f);
		setUniform1f(programId, "pointLight.ambientIntensity",
			((float)pointLightEnabled) * 0.1f);
		setUniform1f(programId, "pointLight.diffuseIntensity",
			((float)pointLightEnabled) * 1.0f);
		setUniform1f(programId, "pointLight.specularIntensity",
			((float)pointLightEnabled) * 1.0f);
	}

	{
		Vector direction = {{ -0.5f, -1.0f, 0.0f, 0.0f }};
		vectorNormalizeInplace(&direction);

		setUniform3f(programId, "spotLight.direction",
			direction.x, direction.y, direction.z);
		setUniform3f(programId, "spotLight.position",
			SPOT_LIGHT_POS.x, SPOT_LIGHT_POS.y, SPOT_LIGHT_POS.z);
		setUniform1f(programId, "spotLight.cutoff", 
			cos(M_PI * 15.0f / 180.0f ));
		setUniform3f(programId, "spotLight.color",
			0.0f, 0.0f, 1.0f);
		setUniform1f(programId, "spotLight.ambientIntensity",
			((float)spotLightEnabled)*0.1f);
		setUniform1f(programId, "spotLight.diffuseIntensity",
			((float)spotLightEnabled)*20.0f);
		setUniform1f(programId, "spotLight.specularIntensity",
			((float)spotLightEnabled)*1.0f);
	}
}

static int
mainInternal(CommonResources* resources)
{
	// load textures

	GLuint grassTexture = resources->textureArray[0];
	GLuint skyboxTexture = resources->textureArray[1];
	GLuint towerTexture = resources->textureArray[2];
	GLuint garkGreenTexture = resources->textureArray[3];
	GLuint redTexture = resources->textureArray[4];
	GLuint blueTexture = resources->textureArray[5];

	if(!loadDDSTexture("textures/grass.dds", grassTexture))
		return -1;

	if(!loadDDSTexture("textures/skybox.dds", skyboxTexture))
		return -1;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(!loadDDSTexture("textures/tower.dds", towerTexture))
		return -1;

	loadOneColorTexture(0.05f, 0.5f, 0.1f, garkGreenTexture);
	loadOneColorTexture(1.0f, 0.0f, 0.0f, redTexture);
	loadOneColorTexture(0.0f, 0.0f, 1.0f, blueTexture);

	GLuint textVAO          = resources->vaoArray[ 0];
	GLuint grassVAO         = resources->vaoArray[ 1];
	GLuint skyboxVAO        = resources->vaoArray[ 2];
	GLuint towerVAO         = resources->vaoArray[ 3];
	GLuint torusVAO         = resources->vaoArray[ 4];
	GLuint sphereVAO        = resources->vaoArray[ 5];

	GLuint textVBO          = resources->vboArray[ 0];
	GLuint grassVBO         = resources->vboArray[ 1];
	GLuint grassIndicesVBO  = resources->vboArray[ 2];
	GLuint skyboxVBO        = resources->vboArray[ 3];
	GLuint skyboxIndicesVBO = resources->vboArray[ 4];
	GLuint towerVBO         = resources->vboArray[ 5];
	GLuint towerIndicesVBO  = resources->vboArray[ 6];
	GLuint torusVBO         = resources->vboArray[ 7];
	GLuint torusIndicesVBO  = resources->vboArray[ 8];
	GLuint sphereVBO        = resources->vboArray[ 9];
	GLuint sphereIndicesVBO = resources->vboArray[10];

	// prepare text

	static const GLfloat globVertexBufferData[] = {
	    -1.0f, -1.0f,  0.0f,
	     1.0f, -1.0f,  0.0f,
	     0.0f,  1.0f,  0.0f,
	};

	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(globVertexBufferData),
    	globVertexBufferData, GL_STATIC_DRAW);

    glBindVertexArray(textVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// load models

	GLsizei grassIndicesNumber, skyboxIndicesNumber, towerIndicesNumber,
		torusIndicesNumber, sphereIndicesNumber;
	GLenum grassIndexType, skyboxIndexType, towerIndexType, torusIndexType, 
		sphereIndexType;

	if(!modelLoad("models/grass.emd", grassVAO, grassVBO, grassIndicesVBO,
		&grassIndicesNumber, &grassIndexType))
		return -1;

	if(!modelLoad("models/skybox.emd", skyboxVAO, skyboxVBO, skyboxIndicesVBO,
		&skyboxIndicesNumber, &skyboxIndexType))
		return -1;

	if(!modelLoad("models/tower.emd", towerVAO, towerVBO, towerIndicesVBO,
		&towerIndicesNumber, &towerIndexType))
		return -1;

	if(!modelLoad("models/torus.emd", torusVAO, torusVBO, torusIndicesVBO,
		&torusIndicesNumber, &torusIndexType))
		return -1;

	if(!modelLoad("models/sphere.emd", sphereVAO, sphereVBO, sphereIndicesVBO,
		&sphereIndicesNumber, &sphereIndexType))
		return -1;

	Matrix projection = matrixPerspective(70.0f, 4.0f / 3.0f, 0.3f, 250.0f);

	GLint uniformMVP = getUniformLocation(resources->programId, "MVP");
	GLint uniformM = getUniformLocation(resources->programId, "M");
	GLint uniformTextureSample = getUniformLocation(
			resources->programId, 
			"textureSampler"
		);
	GLint uniformCameraPos = getUniformLocation(
			resources->programId, 
			"cameraPos"
		);
	GLint uniformMaterialSpecularFactor = getUniformLocation(
			resources->programId,
			"materialSpecularFactor"
		);
	GLint uniformMaterialSpecularIntensity = getUniformLocation(
			resources->programId,
			"materialSpecularIntensity"
		);
	GLint uniformMaterialEmission = getUniformLocation(
			resources->programId,
			"materialEmission"
		);

	glEnable(GL_DOUBLEBUFFER);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0, 0, 0, 1);

	glUseProgram(resources->programId);

	glUniform1i(uniformTextureSample, 0);

	bool directionalLightEnabled = true;
	bool pointLightEnabled = true;
	bool spotLightEnabled = true;
	bool wireframesModeEnabled = false;

	setupLights(resources->programId, directionalLightEnabled, 
		pointLightEnabled, spotLightEnabled);

	uint64_t startTimeMs = getCurrentTimeMs();
	uint64_t currentTimeMs = startTimeMs;
	uint64_t prevTimeMs = startTimeMs;
	uint64_t lastFpsCounterFlushTimeMs = startTimeMs;
	uint64_t lastKeyPressCheckMs = 0;
	float fps = 0.0;

	while(glfwWindowShouldClose(resources->window) == GL_FALSE)
	{
		if(glfwGetKey(resources->window, GLFW_KEY_Q) == GLFW_PRESS)
			break;

		currentTimeMs = getCurrentTimeMs();

		uint64_t startDeltaTimeMs = currentTimeMs - startTimeMs;
		uint64_t prevDeltaTimeMs = currentTimeMs - prevTimeMs;

		prevTimeMs = currentTimeMs;

		float rotationTimeMs = 100000.0;
		float currentRotation = (float)startDeltaTimeMs / rotationTimeMs;
		float islandAngle = 360.0f*(currentRotation - (long)currentRotation);

		// prevent devision by zero and/or very high FPS value right
		// after program start
		if(prevDeltaTimeMs > 0)
			fps = fps*FPS_SMOOTHING + (1.0f - FPS_SMOOTHING) *
				(1000.0f / (float)prevDeltaTimeMs);

		Vector cameraPos;
		cameraGetPosition(resources->camera, &cameraPos);

		// don't update fps to often or no one can read it
		if(currentTimeMs - lastFpsCounterFlushTimeMs > 200)
		{
			snprintf(STATUS_LINE_BUFF, sizeof(STATUS_LINE_BUFF),
					"FPS: %.1f, T: %u%09u, X: %.1f, Y: %.1f, Z: %.1f\n",
					fps,
					(uint32_t)(currentTimeMs / 1000000000),
					(uint32_t)(currentTimeMs % 1000000000),
					cameraPos.x, cameraPos.y, cameraPos.z
				);
			printf("%s", STATUS_LINE_BUFF);

			lastFpsCounterFlushTimeMs = currentTimeMs;
		}

		if(startDeltaTimeMs - lastKeyPressCheckMs > KEY_PRESS_CHECK_INTERVAL)
		{
			if(glfwGetKey(resources->window, GLFW_KEY_X) == GLFW_PRESS)
			{
				lastKeyPressCheckMs = startDeltaTimeMs;
				wireframesModeEnabled = !wireframesModeEnabled;
				if(wireframesModeEnabled)
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				else
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			if(glfwGetKey(resources->window, GLFW_KEY_M) == GLFW_PRESS)
			{
				lastKeyPressCheckMs = startDeltaTimeMs;
				bool enabled = cameraGetMouseInterceptionEnabled(
						resources->camera
					);
				cameraSetMouseInterceptionEnabled(resources->camera, !enabled);
			}

			if(glfwGetKey(resources->window, GLFW_KEY_1) == GLFW_PRESS)
			{
				lastKeyPressCheckMs = startDeltaTimeMs;
				directionalLightEnabled = !directionalLightEnabled;
				setupLights(resources->programId, directionalLightEnabled, 
					pointLightEnabled, spotLightEnabled);
			}

			if(glfwGetKey(resources->window, GLFW_KEY_2) == GLFW_PRESS)
			{
				lastKeyPressCheckMs = startDeltaTimeMs;
				pointLightEnabled = !pointLightEnabled;
				setupLights(resources->programId, directionalLightEnabled, 
					pointLightEnabled, spotLightEnabled);
			}

			if(glfwGetKey(resources->window, GLFW_KEY_3) == GLFW_PRESS)
			{
				lastKeyPressCheckMs = startDeltaTimeMs;
				spotLightEnabled = !spotLightEnabled;
				setupLights(resources->programId, directionalLightEnabled, 
					pointLightEnabled, spotLightEnabled);
			}
		}

		glUniform3f(uniformCameraPos, cameraPos.x, cameraPos.y, cameraPos.z);

		Matrix view;
		cameraGetViewMatrix(resources->camera, prevDeltaTimeMs, &view);

		Matrix vp = matrixMulMat(&view, &projection);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO implement ModelLoader and Model classes

		// tower

		Matrix tempTowerM = matrixIdentity();
		Matrix towerM = matrixRotate(&tempTowerM, islandAngle,
							0.0f, 1.0f, 0.0f);
		matrixTranslateInplace(&tempTowerM, -1.5f, -1.0f, -1.5f);
		towerM = matrixMulMat(&tempTowerM, &towerM);

		Matrix towerMVP = matrixMulMat(&towerM, &vp);

		glBindTexture(GL_TEXTURE_2D, towerTexture);
		glBindVertexArray(towerVAO);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &towerMVP.m[0]);
		glUniformMatrix4fv(uniformM, 1, GL_FALSE, &towerM.m[0]);

		glUniform1f(uniformMaterialSpecularFactor, 1.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, towerIndicesVBO);
		glDrawElements(GL_TRIANGLES, towerIndicesNumber, 
			towerIndexType, NULL);

		// torus

		Matrix tempTorusM = matrixIdentity();
		Matrix torusM = matrixRotate(&tempTorusM, (60.0f - 3.0f*islandAngle),
							0.0f, 1.0f, 0.0f);
		matrixTranslateInplace(&tempTorusM, 0.0f, 1.0f, 0.0f);
		torusM = matrixMulMat(&tempTorusM, &torusM);

		Matrix torusMVP = matrixMulMat(&torusM, &vp);

		glBindTexture(GL_TEXTURE_2D, garkGreenTexture);
		glBindVertexArray(torusVAO);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &torusMVP.m[0]);
		glUniformMatrix4fv(uniformM, 1, GL_FALSE, &torusM.m[0]);
		glUniform1f(uniformMaterialSpecularFactor, 1.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusIndicesVBO);
		glDrawElements(GL_TRIANGLES, torusIndicesNumber, 
			torusIndexType, NULL);

		// grass

		Matrix tempGrassM = matrixIdentity();
		Matrix grassM = matrixRotate(&tempGrassM, islandAngle,
							0.0f, 1.0f, 0.0f);
		matrixTranslateInplace(&tempGrassM, 0.0f, -1.0f, 0.0f);
		grassM = matrixMulMat(&tempGrassM, &grassM);

		Matrix grassMVP = matrixMulMat(&grassM, &vp);

		glBindTexture(GL_TEXTURE_2D, grassTexture);
		glBindVertexArray(grassVAO);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &grassMVP.m[0]);
		glUniformMatrix4fv(uniformM, 1, GL_FALSE, &grassM.m[0]);
		glUniform1f(uniformMaterialSpecularFactor, 32.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 2.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassIndicesVBO);
		glDrawElements(GL_TRIANGLES, grassIndicesNumber, 
			grassIndexType, NULL);

		// skybox

		Matrix tempSkyboxM = matrixIdentity();
		Matrix skyboxM = matrixIdentity();
		matrixTranslateInplace(&skyboxM,
			cameraPos.x, cameraPos.y, cameraPos.z);
		matrixScaleInplace(&tempSkyboxM, 100.0f, 100.0f, 100.0f);
		skyboxM = matrixMulMat(&tempSkyboxM, &skyboxM);

		Matrix skyboxMVP = matrixMulMat(&skyboxM, &vp);

		glBindTexture(GL_TEXTURE_2D, skyboxTexture);
		glBindVertexArray(skyboxVAO);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &skyboxMVP.m[0]);
		glUniformMatrix4fv(uniformM, 1, GL_FALSE, &skyboxM.m[0]);
		glUniform1f(uniformMaterialSpecularFactor, 1.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndicesVBO);
		glDrawElements(GL_TRIANGLES, skyboxIndicesNumber, 
			skyboxIndexType, NULL);

		// point light source
		if(pointLightEnabled) {
			Matrix pointLightM = matrixIdentity();
			matrixTranslateInplace(&pointLightM,
				POINT_LIGHT_POS.x, POINT_LIGHT_POS.y, POINT_LIGHT_POS.z);
			Matrix pointLightMVP = matrixMulMat(&pointLightM, &vp);

			glBindTexture(GL_TEXTURE_2D, redTexture);
			glBindVertexArray(sphereVAO);
			glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &pointLightMVP.m[0]);
			glUniformMatrix4fv(uniformM, 1, GL_FALSE, &pointLightM.m[0]);
			glUniform1f(uniformMaterialSpecularFactor, 1.0f);
			glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
			glUniform3f(uniformMaterialEmission, 0.5f, 0.5f, 0.5f);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
			glDrawElements(GL_TRIANGLES, sphereIndicesNumber,
				sphereIndexType, NULL);
		}

		// spot light source
		if(spotLightEnabled) {
			Matrix spotLightM = matrixIdentity();
			matrixTranslateInplace(&spotLightM,
				SPOT_LIGHT_POS.x, SPOT_LIGHT_POS.y, SPOT_LIGHT_POS.z);
			Matrix spotLightMVP = matrixMulMat(&spotLightM, &vp);

			glBindTexture(GL_TEXTURE_2D, blueTexture);
			glBindVertexArray(sphereVAO);
			glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &spotLightMVP.m[0]);
			glUniformMatrix4fv(uniformM, 1, GL_FALSE, &spotLightM.m[0]);
			glUniform1f(uniformMaterialSpecularFactor, 1.0f);
			glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
			glUniform3f(uniformMaterialEmission, 0.5f, 0.5f, 0.5f);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
			glDrawElements(GL_TRIANGLES, sphereIndicesNumber,
				sphereIndexType, NULL);
		}

		// TODO: glUseProgram with different shaders
		// render text (a few filled triangles first)

		glfwSwapBuffers(resources->window);
		glfwPollEvents();
	}

	return 0;
}

int
main()
{
	int code;
	CommonResources resources;

	if(commonResourcesCreate(&resources) == -1)
		code = 1;
	else
		code = mainInternal(&resources);

	commonResourcesDestroy(&resources);
	return code;
}