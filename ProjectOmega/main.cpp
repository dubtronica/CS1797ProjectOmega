#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <glutil.h>

#define BLUR_PASSES 10

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// settings
const unsigned int SCR_WIDTH = 1000, SCR_HEIGHT = 1000;

// camera configuration at startup
glm::vec3
cameraPos(0.f, 0.f, 2.f),			// position
cameraFront(0.f, 0.f, 0.f),		// where the camera is looking at
cameraUp(0.f, 1.f, 0.f);			// up vector (since the camera starts horizontally, it's as simple as the y-axis

// FPP camera settings
GLfloat
deltaTime = 0.f,					// time elapsed since last frame
lastFrame = 0.f,					// time the last frame was rendered (second 0 is at runtime)
sensitivity = 1.f,				// depending on your mouse input, so play around with this value
mouse_lastX = SCR_WIDTH / 2.f,		// to keep track of mouse position
mouse_lastY = SCR_HEIGHT / 2.f,		// to keep track of mouse position
yaw = -90.f,						// camera yaw 
pitch = 0;	// camera pitch

int selection = 1;

bool
firstMouse = true, refMode = true;			// fixing the mouse location upon startup (check camera slides)                    // input management (so the key is not repeated until released)

float skybox[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

struct Vertexture {
	GLfloat x, y, z, s, t;
};

Vertexture full_screens[] = {
	{-1, 1, 0, 0, 1},
	{-1, -1, 0, 0, 0},
	{1, 1, 0, 1, 1},
	{1, -1, 0, 1, 0}
};

vector<string> faces = { "textures/px.jpg", "textures/nx.jpg", "textures/py.jpg", "textures/ny.jpg", "textures/pz.jpg", "textures/nz.jpg" };

GLuint loadCubemap(vector<string> f) {
	GLuint texID;
	glGenTextures(1, &texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	int width, height, n;
	for (int i = 0; i < f.size(); i++) {
		unsigned char *data = stbi_load(f[i].c_str(), &width, &height, &n, 0);
		
		if (data)
		{
			switch (i)
			{
			case 0:
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			case 1:
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			case 2:
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			case 3:
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			case 4:
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			case 5:
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			default:
				break;
			}
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texID;
}

int main() {

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// -------------------- 
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CS 179.7: DOFDOF", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// OpenGL stuff
	{						// flip images upon loading (for textures)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// captures the mouse cursor / hides it when the window is in focus
		glfwSetCursorPosCallback(window, mouse_callback);				// tell GLFW which method to call whenever the mouse moves
		glfwSwapInterval(1);											// to keep it simple, makes the perFragment run at 60fps
		glEnable(GL_DEPTH_TEST);										// enable depth test
		glEnable(GL_CULL_FACE);											// enable face culling
		glCullFace(GL_BACK);											// cull back-facing polygons (GL_BACK by default but calling anyway)
		glEnable(GL_CLIP_DISTANCE0);
	}

	
	unsigned int skyboxVAO, skyboxVBO;
	{
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skybox), &skybox, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}

	GLuint sphereVAO, sphereVBO;
	auto sphere = genSphere(0.1125f, 50);
	{
		glGenVertexArrays(1, &sphereVAO);
		glGenBuffers(1, &sphereVBO);
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);

		glBufferData(GL_ARRAY_BUFFER, sphere.size() * sizeof(Vertex), sphere.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x1));
	}

	GLuint cubeVAO, cubeVBO;
	auto cube = genCube(0.5f, 50);
	{
		glGenVertexArrays(1, &cubeVAO);
		glBindVertexArray(cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBindVertexArray(cubeVAO);

		glBufferData(GL_ARRAY_BUFFER, cube.size() * sizeof(Vertex), cube.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x1));
	}

	GLuint cubeTexVAO, cubeTexVBO;
	auto texcube = genTexCube(0.5f, 1);
	//auto texcube = genTexPlane(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::vec3(-1, -1, 0), 1);
	{
		glGenVertexArrays(1, &cubeTexVAO);
		glBindVertexArray(cubeTexVAO);
		glGenBuffers(1, &cubeTexVBO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeTexVBO);
		glBindVertexArray(cubeTexVAO);

		glBufferData(GL_ARRAY_BUFFER, texcube.size() * sizeof(NewVertex), texcube.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(NewVertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(NewVertex), (void*)offsetof(NewVertex, x1));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(NewVertex), (void*)offsetof(NewVertex, u));
	}

	GLuint planeVAO, planeVBO;
	auto plane = genPlane(glm::vec3(.2, .4, .3), glm::vec3(.3, .4, .2), glm::vec3(0, -.5, 0), 100);
	{
		glGenVertexArrays(1, &planeVAO);
		glBindVertexArray(planeVAO);
		glGenBuffers(1, &planeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBindVertexArray(planeVAO);

		glBufferData(GL_ARRAY_BUFFER, plane.size() * sizeof(Vertex), plane.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x1));
	}

	GLuint newPlaneVAO, newPlaneVBO;
	//auto plane = genPlane(glm::vec3(.2, .4, .3), glm::vec3(.3, .4, .2), glm::vec3(0, -.5, 0), 100);
	auto newPlane = genTexPlane(glm::vec3(0, 0, .5), glm::vec3(.5, 0, 0), glm::vec3(-.25f, 0.2, -.25f), 100);
	{
		glGenVertexArrays(1, &newPlaneVAO);
		glBindVertexArray(newPlaneVAO);
		glGenBuffers(1, &newPlaneVBO);
		glBindBuffer(GL_ARRAY_BUFFER, newPlaneVBO);
		glBindVertexArray(newPlaneVAO);

		glBufferData(GL_ARRAY_BUFFER, newPlane.size() * sizeof(NewVertex), newPlane.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(NewVertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(NewVertex), (void*)offsetof(NewVertex, x1));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(NewVertex), (void*)offsetof(NewVertex, u));
	}

	GLuint screenVAO, screenVBO;
	{
		glGenVertexArrays(1, &screenVAO);
		glBindVertexArray(screenVAO);
		glGenBuffers(1, &screenVBO);
		glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
		glBindVertexArray(screenVAO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(full_screens), full_screens, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertexture), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertexture), (void*)offsetof(Vertexture, s));

	}
	
	///====================FRAMEBUFFEROBJECTS=============================

	//generate pristine framebuffer
	GLuint pristineFbo;
	{
		glGenFramebuffers(1, &pristineFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, pristineFbo);
	}

	//generate pristine texture
	GLuint pristineTex;
	{
		glGenTextures(1, &pristineTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, pristineTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	}

	//add pristine texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pristineTex, 0);

	//generate depth texture
	GLuint depthTex;
	{
		glGenTextures(1, &depthTex);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	//attach depth texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);

	// check if framebuffer is complete
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Cannot setup framebuffer (incomplete): " << status << ".\n";
		return -1;
	}

	//generate two FBs for blurring
	GLuint blur[2];
	glGenFramebuffers(2, blur);

	//generate two textures for blurring
	GLuint blurTex[2];
	glGenTextures(2, blurTex);

	//set active texture
	glActiveTexture(GL_TEXTURE4);

	//bind both framebuffer and texture, then attach to FB
	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, blur[i]);
		glBindTexture(GL_TEXTURE_2D, blurTex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTex[i], 0);

		// check again if framebuffer is complete
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Cannot setup framebuffer (incomplete): " << status << ".\n";
			return -1;
		}
	}

	// generate and bind framebuffer to store area under the pool
	GLuint refractFbo;
	{
		glGenFramebuffers(1, &refractFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, refractFbo);
	}

	// generate refraction texture
	GLuint refractTex;
	{
		glGenTextures(1, &refractTex);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, refractTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// add refractTex to refractFbo
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, refractTex, 0);

	///loading programs 

	//skybox
	GLuint sbox = loadCubemap(faces);

	GLuint dudvmap;
	loadTexture(&dudvmap, 6, "textures/dudv.jpg");

	GLuint pooltex;
	loadTexture(&pooltex, 7, "textures/bathroom_tiles.jpg");
	
	//SB program
	auto skyboxprogram = loadProgram("shaders/skybox.vsh", "shaders/skybox.fsh");
	auto s_cube = glGetUniformLocation(skyboxprogram, "skybox");
	auto s_view = glGetUniformLocation(skyboxprogram, "view");
	auto s_proj = glGetUniformLocation(skyboxprogram, "projection");

	glUseProgram(skyboxprogram);
	glUniform1i(s_cube, 0);

	//render programs
	GLuint sphereprogram = loadProgram("shaders/reflect.vsh", "shaders/reflect.fsh");

	GLuint u_cube, u_model, u_view, u_proj, u_eyepos, u_dudv, u_pooltex, u_time, u_style;
	{
		u_cube = glGetUniformLocation(sphereprogram, "skybox");
		u_model = glGetUniformLocation(sphereprogram, "model");
		u_view = glGetUniformLocation(sphereprogram, "view");
		u_proj = glGetUniformLocation(sphereprogram, "projection");
		u_eyepos = glGetUniformLocation(sphereprogram, "eye_pos");
		u_dudv = glGetUniformLocation(sphereprogram, "dudv");
		u_pooltex = glGetUniformLocation(sphereprogram, "pooltex");
		u_time = glGetUniformLocation(sphereprogram, "time");
		u_style = glGetUniformLocation(sphereprogram, "style");
	}

	//blur program
	GLuint frameprogram = loadProgram("shaders/frame.vsh", "shaders/frame.fsh");
	auto f_frametex = glGetUniformLocation(frameprogram, "frametex"); //source texture for blurring
	
	glUseProgram(frameprogram);
	glUniform1i(f_frametex, 3);

	//combine program
	GLuint combineprogram = loadProgram("shaders/frame.vsh", "shaders/combine.fsh");
	GLuint c_pristineTex, c_blurTex, c_depthTex, c_focus, c_selection;
	{
		c_pristineTex = glGetUniformLocation(combineprogram, "pristineTex");
		c_blurTex = glGetUniformLocation(combineprogram, "blurTex");
		c_depthTex = glGetUniformLocation(combineprogram, "depthTex");
		c_focus = glGetUniformLocation(combineprogram, "focus");
		c_selection = glGetUniformLocation(combineprogram, "selection");
	}

	glUseProgram(combineprogram);
	
	// set up Uniform1i's
	glUniform1i(c_pristineTex, 3);
	glUniform1i(c_blurTex, 4);
	glUniform1i(c_depthTex, 5);

	GLuint poolprogram = loadProgram("shaders/plain.vsh", "shaders/plain.fsh");
	GLuint p_model, p_view, p_proj, p_pool_tex, p_clipping_plane;
	{
		p_model = glGetUniformLocation(poolprogram, "model");
		p_view = glGetUniformLocation(poolprogram, "view");
		p_proj = glGetUniformLocation(poolprogram, "projection");
		p_pool_tex = glGetUniformLocation(poolprogram, "poolTexture");
		p_clipping_plane = glGetUniformLocation(poolprogram, "clipping_plane");
	}
	
	glUseProgram(poolprogram);
	glUniform1i(p_pool_tex, 7);

	/// render loop
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		// configuring matrices
		glm::mat4 view, proj;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		proj = glm::perspective(glm::radians(45.f), (float)(SCR_WIDTH / SCR_HEIGHT), .1f, 100.f);

		// render the pool to a framebuffer bound to a refractTex
		{
			glBindFramebuffer(GL_FRAMEBUFFER, refractFbo);
			glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// pool
			{
				glFrontFace(GL_CW);
				glm::mat4 model = glm::mat4(1.f);
				glUseProgram(poolprogram);
				glUniformMatrix4fv(p_model, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(p_view, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(p_proj, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform4fv(p_clipping_plane, 1, glm::value_ptr(glm::vec4(0, -1, 0, .2)));

				glBindVertexArray(cubeTexVAO);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, texcube.size());
				glFrontFace(GL_CCW);
			}
		}

		//bind pristineFbo and render
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pristineFbo);
			glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// water plane
			{
				glm::mat4 model = glm::mat4(1.f);
				// model = glm::scale(model, glm::vec3(1.f, 0.8f, 1.f));
				// model = glm::rotate(model, 90.f * PI / 180.f, glm::vec3(1.f, 0.f, 0.f));
				glUseProgram(sphereprogram);
				glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(u_proj, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform3fv(u_eyepos, 1, (GLfloat*)& cameraPos);
				glUniform1i(u_style, 1);

				glUniform1f(u_time, glfwGetTime());

				glUniform1i(p_pool_tex, 8);
				glBindTexture(GL_TEXTURE_CUBE_MAP, sbox);
				glBindVertexArray(newPlaneVAO);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, newPlane.size());
			}

			// pool
			{
				glFrontFace(GL_CW);
				glm::mat4 model = glm::mat4(1.f);
				glUseProgram(poolprogram);
				glUniformMatrix4fv(p_model, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(p_view, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(p_proj, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform4fv(p_clipping_plane, 1, glm::value_ptr(glm::vec4(0, 0, 0, 0)));

				glBindVertexArray(cubeTexVAO);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, texcube.size());
				glFrontFace(GL_CCW);
			}

			//skybox
			{
				glDepthFunc(GL_LEQUAL);
				glUseProgram(skyboxprogram);
				view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));
				glUniformMatrix4fv(s_view, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(s_proj, 1, GL_FALSE, glm::value_ptr(proj));

				glBindVertexArray(skyboxVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glDepthFunc(GL_LESS);
			}
		}

		//perform blurring
		{

			glBindFramebuffer(GL_FRAMEBUFFER, blur[0]);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pristineTex);

			glDisable(GL_DEPTH_TEST);
			glBindVertexArray(screenVAO);

			glUseProgram(frameprogram);
			glBindVertexArray(screenVAO);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			for (int i = 1; i < BLUR_PASSES; i++) {
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, blur[i % 2]);
				glBindTexture(GL_TEXTURE_2D, blurTex[(i + 1) % 2]);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}

		//combine
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, pristineTex);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, blurTex[(BLUR_PASSES + 1) % 2]);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, depthTex);

			glClear(GL_COLOR_BUFFER_BIT);

			glBindVertexArray(screenVAO);
			glUseProgram(combineprogram);
			glUniform1i(c_selection, selection);
			glUniform1f(c_focus, sin(0.75 * glfwGetTime()) + 0.5);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}


		glfwSwapBuffers(window);
		glfwPollEvents();

		// update timers for the camera
		GLfloat currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();

	return 0;
}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		selection = 1; //just added this for selecting through stuff, go check out combine.fsh
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		selection = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		selection = 3;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		selection = 4;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		exit(0);
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// called whenever the mouse moves
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		// this is to prevent the camera from snapping out the first time the perFragment gains focus
		mouse_lastX = xpos;
		mouse_lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - mouse_lastX;
	GLfloat yoffset = mouse_lastY - ypos; // reversed since y-coordinates range from bottom to top
	mouse_lastX = xpos;
	mouse_lastY = ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	pitch += yoffset;
	yaw += xoffset;

	// constraints on pitch so the camera doesn't suddenly flip the image vertically 
	pitch = pitch > 89.f ? 89.f : pitch;
	pitch = pitch < -89.f ? -89.f : pitch;

	glm::vec3 front;
	front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	front.y = sin(glm::radians(pitch));
	front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	cameraFront = glm::normalize(front);

	cameraPos = -cameraFront;
}