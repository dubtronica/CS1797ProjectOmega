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

	GLuint spherevao, spherevbo;
	auto sphere = genSphere(0.1125f, 50);
	{
		glGenVertexArrays(1, &spherevao);
		glBindVertexArray(spherevao);
		glGenBuffers(1, &spherevbo);
		glBindBuffer(GL_ARRAY_BUFFER, spherevbo);
		glBindVertexArray(spherevao);

		glBufferData(GL_ARRAY_BUFFER, sphere.size() * sizeof(Vertex), sphere.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x1));
	}

	GLuint cubevao, cubevbo;
	auto cube = genCube(0.1125f, 50);
	{
		glGenVertexArrays(1, &cubevao);
		glBindVertexArray(cubevao);
		glGenBuffers(1, &cubevbo);
		glBindBuffer(GL_ARRAY_BUFFER, cubevbo);
		glBindVertexArray(cubevao);

		glBufferData(GL_ARRAY_BUFFER, cube.size() * sizeof(Vertex), cube.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x1));
	}

	GLuint vao, vbo;
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindVertexArray(vao);

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

	///loading programs 

	//skybox
	GLuint sbox = loadCubemap(faces);
	
	//SB program
	auto skyboxprogram = loadProgram("shaders/skybox.vsh", "shaders/skybox.fsh");
	auto s_cube = glGetUniformLocation(skyboxprogram, "skybox");
	auto s_view = glGetUniformLocation(skyboxprogram, "view");
	auto s_proj = glGetUniformLocation(skyboxprogram, "projection");

	//render programs
	GLuint sphereprogram = loadProgram("shaders/reflect.vsh", "shaders/reflect.fsh");
	GLuint sphereprogram2 = loadProgram("shaders/refract.vsh", "shaders/refract.fsh");

	GLuint u_cube, u_model, u_view, u_proj, u_eyepos, u_cube2, u_model2, u_view2, u_proj2, u_eyepos2;
	{
		u_cube = glGetUniformLocation(sphereprogram, "skybox");
		u_model = glGetUniformLocation(sphereprogram, "model");
		u_view = glGetUniformLocation(sphereprogram, "view");
		u_proj = glGetUniformLocation(sphereprogram, "projection");
		u_eyepos = glGetUniformLocation(sphereprogram, "eye_pos");

		u_cube2 = glGetUniformLocation(sphereprogram2, "skybox");
		u_model2 = glGetUniformLocation(sphereprogram2, "model");
		u_view2 = glGetUniformLocation(sphereprogram2, "view");
		u_proj2 = glGetUniformLocation(sphereprogram2, "projection");
		u_eyepos2 = glGetUniformLocation(sphereprogram2, "eye_pos");
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
	

	/// render loop
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		std::cout << selection << std::endl;

		//bind pristineFbo and render
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pristineFbo);
			glClearColor(0.f, 0.f, 0.f, 1.0f);
			glEnable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// configuring matrices
			glm::mat4 view, proj;
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
			proj = glm::perspective(glm::radians(45.f), (float)(SCR_WIDTH / SCR_HEIGHT), .1f, 100.f);

			//first sphere
			{
				glm::mat4 model = glm::mat4(1.f);
				glUseProgram(sphereprogram);
				glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(u_proj, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform3fv(u_eyepos, 1, (GLfloat*)&cameraPos);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, sbox);
				glBindVertexArray(spherevao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, sphere.size());
			}

			//second sphere
			{
				glm::mat4 model2 = glm::mat4(1.f);
				model2 = glm::translate(model2, glm::vec3(0.f, 0.2f, 0.f));
				model2 = glm::scale(model2, glm::vec3(0.5f, 0.5f, 0.5f));
				glUseProgram(sphereprogram2);
				glUniformMatrix4fv(u_model2, 1, GL_FALSE, glm::value_ptr(model2));
				glUniformMatrix4fv(u_view2, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(u_proj2, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform3fv(u_eyepos2, 1, (GLfloat*)&cameraPos);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, sbox);
				glBindVertexArray(spherevao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, sphere.size());
			}

			//third sphere
			{
				glm::mat4 model3 = glm::mat4(1.f);
				model3 = glm::translate(model3, glm::vec3(0.f, -0.2f, 0.f));
				model3 = glm::scale(model3, glm::vec3(0.5f, 0.5f, 0.5f));
				glUseProgram(sphereprogram2);
				glUniformMatrix4fv(u_model2, 1, GL_FALSE, glm::value_ptr(model3));
				glUniformMatrix4fv(u_view2, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(u_proj2, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform3fv(u_eyepos2, 1, (GLfloat*)&cameraPos);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, sbox);
				glBindVertexArray(spherevao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, sphere.size());
			}

			//first cube
			{
				glm::mat4 model4 = glm::mat4(1.f);
				model4 = glm::translate(model4, glm::vec3(-0.2f, 0.f, 0.f));
				model4 = glm::scale(model4, glm::vec3(0.5f, 0.5f, 0.5f));
				glUseProgram(sphereprogram);
				glUniformMatrix4fv(u_model2, 1, GL_FALSE, glm::value_ptr(model4));
				glUniformMatrix4fv(u_view2, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(u_proj2, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform3fv(u_eyepos2, 1, (GLfloat*)&cameraPos);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, sbox);
				glBindVertexArray(cubevao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, cube.size());
			}

			//second cube
			{
				glm::mat4 model5 = glm::mat4(1.f);
				model5 = glm::translate(model5, glm::vec3(0.2f, 0.f, 0.f));
				model5 = glm::scale(model5, glm::vec3(0.5f, 0.5f, 0.5f));
				glUseProgram(sphereprogram);
				glUniformMatrix4fv(u_model2, 1, GL_FALSE, glm::value_ptr(model5));
				glUniformMatrix4fv(u_view2, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(u_proj2, 1, GL_FALSE, glm::value_ptr(proj));
				glUniform3fv(u_eyepos2, 1, (GLfloat*)&cameraPos);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, sbox);
				glBindVertexArray(cubevao);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, cube.size());
			}


			//skybox
			{
				glDepthFunc(GL_LEQUAL);
				glUseProgram(skyboxprogram);
				view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)));
				glUniform1i(s_cube, sbox);
				glUniformMatrix4fv(s_view, 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(s_proj, 1, GL_FALSE, glm::value_ptr(proj));

				glBindVertexArray(skyboxVAO);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, sbox);
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
			glBindVertexArray(vao);

			glUseProgram(frameprogram);
			glBindVertexArray(vao);

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

			glBindVertexArray(vao);
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