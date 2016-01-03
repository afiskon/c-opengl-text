#include <GLXW/glxw.h>

// TODO: clean code + delete link to glm repository
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <assert.h>

#include "utils/constants.h"
#include "utils/linearalg.h"

#include "utils/utils.h"
#include "utils/camera.h"
#include "utils/models.h"

// TODO: "compress" repository
// TODO: use Assimp C interface

static const glm::vec3 POINT_LIGHT_POS_OLD(-2.0f, 3.0f, 0.0f);
static const glm::vec3 SPOT_LIGHT_POS_OLD(4.0f, 5.0f, 0.0f);

static const Vector4 POINT_LIGHT_POS_NEW = {{ -2.0f, 3.0f, 0.0f, 0.0f }};
static const Vector4 SPOT_LIGHT_POS_NEW = {{ 4.0f, 5.0f, 0.0f, 0.0f }};
static const Vector4 CAMERA_START_POS = {{ 0.0f, 0.0f, 5.0f, 0.0f }};

static const uint64_t KEY_PRESS_CHECK_INTERVAL_MS = 250;
static const float FPS_SMOOTHING = 0.9; // larger - more smoothing

static_assert(FPS_SMOOTHING >= 0.0 && FPS_SMOOTHING <= 1.0,
	"Invalid FPS_SMOOTHING value");

#define TEXTURES_NUM 6
#define VAOS_NUM 5
#define VBOS_NUM 10

struct CommonResources
{
	bool windowInitialized;
	bool cameraInitialized;
	bool programIdInitialized;
	bool textureArrayInitialized;
	bool vaoArrayInitialized;
	bool vboArrayInitialized;

	GLFWwindow* window;
	Camera* camera;
	GLuint programId;
	GLuint textureArray[TEXTURES_NUM];
	GLuint vaoArray[VAOS_NUM];
	GLuint vboArray[VBOS_NUM];
};

static void
windowSizeCallback(GLFWwindow *, int width, int height)
{
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

	shaders[0] = loadShader("shaders/vertexShader.glsl", GL_VERTEX_SHADER,
		&errorFlag);
	if(errorFlag) {
		fprintf(stderr, "Failed to load vertex shader (invalid working "
			"directory?)\n");
		return -1;
	}

	shaders[1] = loadShader("shaders/fragmentShader.glsl", GL_FRAGMENT_SHADER,
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
		Vector4 direction = {{ 0.0f, -1.0f, 1.0f, 0.0f }};
		normalizevec4(&direction);

		setUniform3f(programId, "directionalLight.direction",
			direction.x, direction.y, direction.z);
		setUniform3f(programId, "directionalLight.color",
			1.0f, 1.0f, 1.0f);
		setUniform1f(programId, "directionalLight.ambientIntensity",
			float(directionalLightEnabled)*0.1f);
		setUniform1f(programId, "directionalLight.diffuseIntensity",
			float(directionalLightEnabled)*0.1f);
		setUniform1f(programId, "directionalLight.specularIntensity",
			float(directionalLightEnabled)*1.0f);
	}

	{
		setUniform3f(programId, "pointLight.position",
			POINT_LIGHT_POS_NEW.x, POINT_LIGHT_POS_NEW.y, POINT_LIGHT_POS_NEW.z);
		setUniform3f(programId, "pointLight.color",
			1.0f, 0.0f, 0.0f);
		setUniform1f(programId, "pointLight.ambientIntensity",
			float(pointLightEnabled) * 0.1f);
		setUniform1f(programId, "pointLight.diffuseIntensity",
			float(pointLightEnabled) * 1.0f);
		setUniform1f(programId, "pointLight.specularIntensity",
			float(pointLightEnabled) * 1.0f);
	}

	{
		Vector4 direction = {{ -0.5f, -1.0f, 0.0f, 0.0f }};
		normalizevec4(&direction);

		setUniform3f(programId, "spotLight.direction",
			direction.x, direction.y, direction.z);
		setUniform3f(programId, "spotLight.position",
			SPOT_LIGHT_POS_NEW.x, SPOT_LIGHT_POS_NEW.y, SPOT_LIGHT_POS_NEW.z);
		setUniform1f(programId, "spotLight.cutoff", 
			cos(M_PI * 15.0f / 180.0f ));
		setUniform3f(programId, "spotLight.color",
			0.0f, 0.0f, 1.0f);
		setUniform1f(programId, "spotLight.ambientIntensity",
			float(spotLightEnabled)*0.1f);
		setUniform1f(programId, "spotLight.diffuseIntensity",
			float(spotLightEnabled)*20.0f);
		setUniform1f(programId, "spotLight.specularIntensity",
			float(spotLightEnabled)*1.0f);
	}
}

static void
debugCompareMatrices(glm::mat4 Old, Matrix New)
{
	bool error = false;
    for(int ti = 0; ti < 4; ti++)
        for(int tj = 0; tj < 4; tj++)
        	if(fabs(Old[ti][tj] - New.m[ti*4 + tj]) > 0.0001f ) {
        		fprintf(stderr, "ERROR: Old != New,"
        			" ti = %d, tj = %d, old = %f, new = %f, delta = %f\n", ti, tj, Old[ti][tj], New.m[ti*4 + tj], fabs(Old[ti][tj] - New.m[ti*4 + tj]));
        		error = true;
        	}
    if(error) exit(1);
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

	// load models

	GLuint grassVAO = resources->vaoArray[0];
	GLuint skyboxVAO = resources->vaoArray[1];
	GLuint towerVAO = resources->vaoArray[2];
	GLuint torusVAO = resources->vaoArray[3];
	GLuint sphereVAO = resources->vaoArray[4];

	GLuint grassVBO = resources->vboArray[0];
	GLuint grassIndicesVBO = resources->vboArray[1];
	GLuint skyboxVBO = resources->vboArray[2];
	GLuint skyboxIndicesVBO = resources->vboArray[3];
	GLuint towerVBO = resources->vboArray[4];
	GLuint towerIndicesVBO = resources->vboArray[5];
	GLuint torusVBO = resources->vboArray[6];
	GLuint torusIndicesVBO = resources->vboArray[7];
	GLuint sphereVBO = resources->vboArray[8];
	GLuint sphereIndicesVBO = resources->vboArray[9];

	GLsizei grassIndicesNumber, skyboxIndicesNumber, towerIndicesNumber,
		torusIndicesNumber, sphereIndicesNumber;
	GLenum grassIndexType, skyboxIndexType, towerIndexType, torusIndexType, 
		sphereIndexType;

	if(!modelLoad("models/grass.emd", grassVAO, grassVBO, grassIndicesVBO, &grassIndicesNumber, &grassIndexType))
		return -1;

	if(!modelLoad("models/skybox.emd", skyboxVAO, skyboxVBO, skyboxIndicesVBO, &skyboxIndicesNumber, &skyboxIndexType))
		return -1;

	if(!modelLoad("models/tower.emd", towerVAO, towerVBO, towerIndicesVBO, &towerIndicesNumber, &towerIndexType))
		return -1;

	if(!modelLoad("models/torus.emd", torusVAO, torusVBO, torusIndicesVBO, &torusIndicesNumber, &torusIndexType))
		return -1;

	if(!modelLoad("models/sphere.emd", sphereVAO, sphereVBO, sphereIndicesVBO, &sphereIndicesNumber, &sphereIndexType))
		return -1;

	glm::mat4 projectionOld = glm::perspective(70.0f, 4.0f / 3.0f, 0.3f, 250.0f);

	// OK: Matrix projectionNew = perspective(70.0f, 4.0f / 3.0f, 0.3f, 250.0f);

	GLint uniformMVP = getUniformLocation(resources->programId, "MVP");
	GLint uniformM = getUniformLocation(resources->programId, "M");
	GLint uniformTextureSample = getUniformLocation(resources->programId, 
		"textureSampler");
	GLint uniformCameraPos = getUniformLocation(resources->programId, 
		"cameraPos");

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
			fps = fps*FPS_SMOOTHING + (1.0 - FPS_SMOOTHING) *
				(1000.0 / (float)prevDeltaTimeMs);

		// don't update fps to often or no one can read it
		if(currentTimeMs - lastFpsCounterFlushTimeMs > 200)
		{
			printf("FPS: %f, currentTimeMs = 0x%08x%08x\n",
					fps,
					(uint32_t)(currentTimeMs >> 32),
					(uint32_t)(currentTimeMs & 0xFFFFFFFF)
				);
			lastFpsCounterFlushTimeMs = currentTimeMs;
		}

		if(startDeltaTimeMs - lastKeyPressCheckMs > KEY_PRESS_CHECK_INTERVAL_MS)
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

		Vector4 cameraPosNew;
		cameraGetPosition(resources->camera, &cameraPosNew);

		glm::vec3 cameraPosOld(cameraPosNew.x, cameraPosNew.y, cameraPosNew.z);


		glUniform3f(uniformCameraPos, cameraPosNew.x, cameraPosNew.y, cameraPosNew.z);

		
		Matrix viewNew;
		cameraGetViewMatrix(resources->camera, prevDeltaTimeMs, &viewNew);

        glm::mat4 viewOld;
        for(int ti = 0; ti < 4; ti++)
            for(int tj = 0; tj < 4; tj++)
                viewOld[ti][tj] = viewNew.m[ti*4 + tj];

		glm::mat4 vpOld = projectionOld * viewOld;
		// OK: Matrix vpNew = multiplymat4(&viewNew, &projectionNew);




		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO implement ModelLoader and Model classes

		// tower

		glm::mat4 towerMOld = glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) *
			glm::translate(-1.5f, -1.0f, -1.5f);
		glm::mat4 towerMVPOld = vpOld * towerMOld;

		Matrix tempTowerM = identitymat();
		Matrix towerMNew = rotate(&tempTowerM /* identity */,
							islandAngle, { 0.0f, 1.0f, 0.0f, 0.0f });

		translate(&tempTowerM, -1.5f, -1.0f, -1.5f);
		towerMNew = multiplymat4(&tempTowerM, &towerMNew); // OK!

		debugCompareMatrices(towerMOld, towerMNew);

		// Matrix towerMVPNew = multiplymat4(&towerMNew, &vpNew);
		// debugCompareMatrices(towerMVPOld, towerMVPNew);

		glBindTexture(GL_TEXTURE_2D, towerTexture);
		glBindVertexArray(towerVAO);
		// glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &towerMVPNew.m[0]);
		// glUniformMatrix4fv(uniformM, 1, GL_FALSE, &towerM.m[0]);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &towerMVPOld[0][0]);
        glUniformMatrix4fv(uniformM, 1, GL_FALSE, &towerMOld[0][0]);

		glUniform1f(uniformMaterialSpecularFactor, 1.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, towerIndicesVBO);
		glDrawElements(GL_TRIANGLES, towerIndicesNumber, 
			towerIndexType, NULL);

		// torus

		glm::mat4 torusM = glm::translate(0.0f, 1.0f, 0.0f) *
			glm::rotate((60.0f - 3.0f*islandAngle), 0.0f, 0.5f, 0.0f);
		glm::mat4 torusMVP = vpOld * torusM;

		glBindTexture(GL_TEXTURE_2D, garkGreenTexture);
		glBindVertexArray(torusVAO);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &torusMVP[0][0]);
		glUniformMatrix4fv(uniformM, 1, GL_FALSE, &torusM[0][0]);
		glUniform1f(uniformMaterialSpecularFactor, 1.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torusIndicesVBO);
		glDrawElements(GL_TRIANGLES, torusIndicesNumber, 
			torusIndexType, NULL);

		// grass

		glm::mat4 grassM = glm::rotate(islandAngle, 0.0f, 1.0f, 0.0f) *
			glm::translate(0.0f, -1.0f, 0.0f);
		glm::mat4 grassMVP = vpOld * grassM;

		glBindTexture(GL_TEXTURE_2D, grassTexture);
		glBindVertexArray(grassVAO);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &grassMVP[0][0]);
		glUniformMatrix4fv(uniformM, 1, GL_FALSE, &grassM[0][0]);
		glUniform1f(uniformMaterialSpecularFactor, 32.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 2.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grassIndicesVBO);
		glDrawElements(GL_TRIANGLES, grassIndicesNumber, 
			grassIndexType, NULL);

		// skybox

		glm::mat4 skyboxM = glm::translate(cameraPosOld) *
			glm::scale(100.0f,100.0f,100.0f);
		glm::mat4 skyboxMVP = vpOld * skyboxM;

		glBindTexture(GL_TEXTURE_2D, skyboxTexture);
		glBindVertexArray(skyboxVAO);
		glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &skyboxMVP[0][0]);
		glUniformMatrix4fv(uniformM, 1, GL_FALSE, &skyboxM[0][0]);
		glUniform1f(uniformMaterialSpecularFactor, 1.0f);
		glUniform1f(uniformMaterialSpecularIntensity, 0.0f);
		glUniform3f(uniformMaterialEmission, 0.0f, 0.0f, 0.0f);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxIndicesVBO);
		glDrawElements(GL_TRIANGLES, skyboxIndicesNumber, 
			skyboxIndexType, NULL);

		// point light source
		if(pointLightEnabled) {
			glm::mat4 pointLightM = glm::translate(POINT_LIGHT_POS_OLD);
			glm::mat4 pointLightMVP = vpOld * pointLightM;

			glBindTexture(GL_TEXTURE_2D, redTexture);
			glBindVertexArray(sphereVAO);
			glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &pointLightMVP[0][0]);
			glUniformMatrix4fv(uniformM, 1, GL_FALSE, &pointLightM[0][0]);
			glUniform1f(uniformMaterialSpecularFactor, 1.0f);
			glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
			glUniform3f(uniformMaterialEmission, 0.5f, 0.5f, 0.5f);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
			glDrawElements(GL_TRIANGLES, sphereIndicesNumber,
				sphereIndexType, NULL);
		}

		// spot light source
		if(spotLightEnabled) {
			glm::mat4 spotLightM = glm::translate(SPOT_LIGHT_POS_OLD);
			glm::mat4 spotLightMVP = vpOld * spotLightM;

			glBindTexture(GL_TEXTURE_2D, blueTexture);
			glBindVertexArray(sphereVAO);
			glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, &spotLightMVP[0][0]);
			glUniformMatrix4fv(uniformM, 1, GL_FALSE, &spotLightM[0][0]);
			glUniform1f(uniformMaterialSpecularFactor, 1.0f);
			glUniform1f(uniformMaterialSpecularIntensity, 1.0f);
			glUniform3f(uniformMaterialEmission, 0.5f, 0.5f, 0.5f);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndicesVBO);
			glDrawElements(GL_TRIANGLES, sphereIndicesNumber,
				sphereIndexType, NULL);
		}

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