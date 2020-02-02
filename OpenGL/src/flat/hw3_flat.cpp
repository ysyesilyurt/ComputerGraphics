#include <GL/glew.h>
#include <glm/ext.hpp>
#include <vector>
#include "helper.h"

/* Shader and Texture Ids */
GLuint idProgramShader;
GLuint idFragmentShader;
GLuint idVertexShader;
GLuint idJpegTexture;
GLuint idJpegHeightmap;

/* Think of our data as a company, VertexArrayObject is the boss and it manages several staff members which
 * are VertexBufferObjects. Hence we will keep 1 VAO (and then be done with it) handle all our tasks using our VBOs */
GLuint vertexArrayObject;
// VBOs -- we keep one for vertexAttributes and one for indices
GLuint terrainVertexAttribBuffer, terrainIndexBuffer;

/* CPU backed Containers for our vertex data */
std::vector<int> indices;
std::vector<Vertex> vertices;

/* Texture variables */
int textureWidth, textureHeight, heightTextureWidth, heightTextureHeight, textureOffset = 0;

/* Window variables */
static GLFWwindow * window = nullptr;
int windowX = 1000;
int windowY = 1000;

/* Geometry variables */
glm::vec3 pos, gaze, up, light_pos;
glm::mat4 MVP, M_model, M_view, M_projection;
float camSpeed = 0.0, heightFactor = 10.0, pitch = 45.0, yaw = 90.0, fovy = 45.0, aspectRatio = 1.0,
near = 0.1, far = 1000.0;

/* Uniform variable locations */
int MVP_location, heightFactor_location, textureWidth_location, textureHeight_location, cameraPos_location,
heightmap_location, texture_location, lightPos_location, textureOffset_location;

/* Key flags */
bool heightFactor_increase = false, heightFactor_decrease = false, pitch_increase = false, pitch_decrease = false,
yaw_increase = false, yaw_decrease = false, stop_cam = false, camSpeed_increase = false, camSpeed_decrease = false,
move_map_left = false, move_map_right = false, rollback_to_initial = false, move_light_left = false,
move_light_right = false, move_light_up = false, move_light_down = false, light_y_increase = false,
light_y_decrease = false, isFullscreen = false;

void cleanUp() {
	// Disable vertex arrays at the end
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// Cleanup VBOs and shader
	glDeleteBuffers(1, &terrainVertexAttribBuffer);
	glDeleteBuffers(1, &terrainIndexBuffer);
	glDeleteProgram(idProgramShader);
	glDeleteVertexArrays(1, &vertexArrayObject);

	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(window);
	glfwTerminate();
}

void initializeVertices() {
	for (int z = 0; z < textureHeight + 1; z++) {
		for (int x = 0; x < textureWidth + 1; x++) {
			Vertex vertex;
			vertex.position = glm::vec3(x, 0.0, z); // y-coords will come from the vert shader
			vertex.normal = glm::vec3(0.0); // will be calculated in vert shader
			vertex.tex_coord = glm::vec2(1.0 - (x * 1.0 / textureWidth), 1.0 - (z * 1.0 / textureHeight));
			vertices.push_back(vertex);
		}
	}
}

void initializeIndices() {
	/* Initialize indices per pixel, be careful about the winding order! */
	for (int i = textureWidth + 2; i < (textureWidth + 1) * (textureHeight + 1); i++) {
		/* Each Pixel consists of 2 triangles */
		if (i % (textureWidth + 1) == 0) {
			/* Time to switch rows.. */
			continue;
		}
		/* Provide indices for the first triangle with a correct winding order that suits RH-rule */
		indices.push_back(i - (textureWidth + 2));
		indices.push_back(i - 1);
		indices.push_back(i - (textureWidth + 1));

		/* Provide indices for the second triangle with a correct winding order that suits RH-rule */
		indices.push_back(i - (textureWidth + 1));
		indices.push_back(i - 1);
		indices.push_back(i);
	}
}

void initBuffers() {
	/* Init VAO */
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	/* Init VBOs */
	glGenBuffers(1, &terrainVertexAttribBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, terrainVertexAttribBuffer);

	/* Construct Terrain primitive data on CPU memory and pass it onto GPU memory */
	initializeVertices();
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &terrainIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainIndexBuffer);

	/* Construct Terrain primitives index data on CPU memory and pass it onto GPU memory */
	initializeIndices();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

	/* glVertexAttribPointer(array_index, #_of_coods_per_Vertex, type, need_normalization?,
	 * Byte_offset_between_consecutive_vertices, offset_of_fields_in_vertex_structure) */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) offsetof(Vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (offsetof(Vertex, tex_coord)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void setupGeometry() {
	/* Initialize Cam vectors first */
	pos = glm::vec3(textureWidth / 2.0, textureWidth / 10.0, -textureWidth / 4.0);
	gaze = glm::vec3(0.0, 0.0, 1.0);
	up = glm::vec3(0.0, 1.0, 0.0);

	/* Now Set MVP */
	M_model = glm::mat4(1.0);
	M_view = glm::lookAt(pos, pos + gaze, up);
	M_projection = glm::perspective(fovy, aspectRatio, near, far);
	MVP = M_projection * M_view * M_model;

	/* Set initial Light Position */
	light_pos = glm::vec3(textureWidth / 2.0, 100, textureHeight / 2.0);
}

void setUniforms() {
	/* Set the uniform variables so that our shaders can access them */
	MVP_location = glGetUniformLocation(idProgramShader, "MVP");
	glUniformMatrix4fv(MVP_location, 1, GL_FALSE, glm::value_ptr(MVP));

	heightFactor_location = glGetUniformLocation(idProgramShader, "heightFactor");
	glUniform1f(heightFactor_location, heightFactor);

	textureWidth_location = glGetUniformLocation(idProgramShader, "textureWidth");
	glUniform1i(textureWidth_location, textureWidth);

	textureHeight_location = glGetUniformLocation(idProgramShader, "textureHeight");
	glUniform1i(textureHeight_location, textureHeight);

	textureOffset_location = glGetUniformLocation(idProgramShader, "textureOffset");
	glUniform1i(textureOffset_location, textureOffset);

	cameraPos_location = glGetUniformLocation(idProgramShader, "cameraPos");
	glUniform3fv(cameraPos_location, 1, glm::value_ptr(pos));

	lightPos_location = glGetUniformLocation(idProgramShader, "lightPos");
	glUniform3fv(lightPos_location, 1, glm::value_ptr(light_pos));

	heightmap_location = glGetUniformLocation(idProgramShader, "heightMapTexture");
	glUniform1i(heightmap_location, 0);

	texture_location = glGetUniformLocation(idProgramShader, "rgbTexture");
	glUniform1i(texture_location, 1);
}

void render() {
	/* First clear all buffers */
	glClearColor(0,0,0,1);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Now render the frame */
	// First update the gaze acc. to new values of yaw and pitch
	gaze.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	gaze.y = sin(glm::radians(pitch - 45.0));
	gaze.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	gaze = glm::normalize(gaze); // do not forget the normalize the gaze

	// Then update the position of the camera
	pos += camSpeed * gaze;
	M_view = glm::lookAt(pos, pos + gaze, up); // gluLookAt(eye, center, up)
	M_projection = glm::perspective(fovy, aspectRatio, near, far);
	MVP = M_projection * M_view * M_model;

	// Do not forget the update the uniforms of geometry too
	glUniformMatrix4fv(MVP_location, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform3fv(cameraPos_location, 1, glm::value_ptr(pos));

	// Finally draw the scene using indices..
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}

void toggleScreens() {
	if (isFullscreen) {
		isFullscreen = false;
		glfwSetWindowMonitor(window, nullptr, windowX, windowY, windowX, windowY, 0);
		glViewport(0, 0, windowX, windowY);
	}
	else {
		isFullscreen = true;
		glfwGetWindowPos(window, &windowX, &windowY);
		glfwGetWindowSize(window, &windowX, &windowY);
		auto monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode * mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, 0);
		glViewport(0, 0, mode->width, mode->height);
	}
}

static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		heightFactor_increase = true;

	if (key == GLFW_KEY_R && action == GLFW_RELEASE)
		heightFactor_increase = false;

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		heightFactor_decrease = true;

	if (key == GLFW_KEY_F && action == GLFW_RELEASE)
		heightFactor_decrease = false;

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		move_map_left = true;

	if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		move_map_left = false;

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		move_map_right = true;

	if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		move_map_right = false;

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		move_light_left = true;

	if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		move_light_left = false;

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		move_light_right = true;

	if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		move_light_right = false;

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		move_light_up = true;

	if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
		move_light_up = false;

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		move_light_down = true;

	if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		move_light_down = false;

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		light_y_increase = true;

	if (key == GLFW_KEY_T && action == GLFW_RELEASE)
		light_y_increase = false;

	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		light_y_decrease = true;

	if (key == GLFW_KEY_G && action == GLFW_RELEASE)
		light_y_decrease = false;

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		pitch_increase = true;

	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		pitch_increase = false;

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		pitch_decrease = true;

	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		pitch_decrease = false;

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		yaw_decrease = true;

	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		yaw_decrease = false;

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		yaw_increase = true;

	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		yaw_increase = false;

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		camSpeed_increase = true;

	if (key == GLFW_KEY_Y && action == GLFW_RELEASE)
		camSpeed_increase = false;

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		camSpeed_decrease = true;

	if (key == GLFW_KEY_H && action == GLFW_RELEASE)
		camSpeed_decrease = false;

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		stop_cam = true;

	if (key == GLFW_KEY_X && action == GLFW_RELEASE)
		stop_cam = false;

	if (key == GLFW_KEY_I && action == GLFW_PRESS)
		rollback_to_initial = true;

	if (key == GLFW_KEY_I && action == GLFW_RELEASE)
		rollback_to_initial = false;

	if (key == GLFW_KEY_P && action == GLFW_RELEASE)
		toggleScreens();
}

void moveLight(int toWhere) {
	switch (toWhere) {
		case 0: // left
			light_pos.x += 5;
			break;
		case 1: // right
			light_pos.x -= 5;
			break;
		case 2: // up
			light_pos.z += 5;
			break;
		case 3: // down
			light_pos.z -= 5;
			break;
		case 4: // increase height
			light_pos.y += 5;
			break;
		case 5: // decrease height
			light_pos.y -= 5;
			break;
		default:
			std::cout << "Problem Encountered during light location update!\n";
			break;
	}
	glUniform3fv(lightPos_location, 1, glm::value_ptr(light_pos));
}

void updateScene() {
	if (heightFactor_increase) {
		heightFactor += 0.5;
		glUniform1f(heightFactor_location, heightFactor);
	}
	if (heightFactor_decrease) {
		heightFactor -= 0.5;
		glUniform1f(heightFactor_location, heightFactor);
	}
	if (move_map_left) {
		textureOffset -= 1;
		glUniform1i(textureOffset_location, textureOffset);
	}
	if (move_map_right){
		textureOffset += 1;
		glUniform1i(textureOffset_location, textureOffset);
	}
	if (rollback_to_initial) {
		pos = glm::vec3(textureWidth / 2.0, textureWidth / 10.0, -textureWidth / 4.0);
		light_pos = glm::vec3(textureWidth / 2.0, 100, textureHeight / 2.0);
		glUniform3fv(lightPos_location, 1, glm::value_ptr(light_pos));
		textureOffset = 0;
		glUniform1i(textureOffset_location, textureOffset);
		heightFactor = 10.0;
		glUniform1f(heightFactor_location, heightFactor);
		camSpeed = 0;
		pitch = 45.0;
		yaw = 90.0;
	}
	if (pitch_increase) {
		pitch += 0.05;
		if (pitch > 89.0)
			pitch = 89.0;
	}
	if (pitch_decrease) {
		pitch -= 0.05;
		if (pitch < 0.0)
			pitch = 0.0;
	}
	if (yaw_increase) {
		yaw += 0.05;
		if (yaw > 360.0)
			yaw -= 360.0;
	}
	if (yaw_decrease) {
		yaw -= 0.05;
		if (yaw < 0.0)
			yaw += 360.0;
	}
	if (camSpeed_increase)
		camSpeed += 0.01;
	if (camSpeed_decrease)
		camSpeed -= 0.01;
	if (stop_cam)
		camSpeed = 0.0;
	if (move_light_left)
		moveLight(0);
	if (move_light_right)
		moveLight(1);
	if (move_light_up)
		moveLight(2);
	if (move_light_down)
		moveLight(3);
	if (light_y_increase)
		moveLight(4);
	if (light_y_decrease)
		moveLight(5);
}

static void resizeCallback(GLFWwindow* win, int width, int height) {
	glViewport(0, 0, width, height);
}

static void errorCallback(int error, const char * description) {
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char * argv[]) {

	if (argc != 3) {
		printf("Please run the executable as ./hw3_flat <height_map.jpg> <texture_map.jpg>\n");
		exit(-1);
	}

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		exit(-1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	window = glfwCreateWindow(windowX, windowY, "CENG477 - HW3 - Flat Mapping", nullptr, nullptr);

	if (!window) {
		fprintf(stderr, "Could not create window, exitting..");
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	glViewport(0,0, windowX, windowY);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW, exitting..");
		glfwTerminate();
		exit(-1);
	}

	glfwSetFramebufferSizeCallback(window, resizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	initShaders();
	initTexture(argv[1], &heightTextureWidth, &heightTextureHeight, true);
	initTexture(argv[2], &textureWidth, &textureHeight, false);
	initBuffers();
	setupGeometry();
	glUseProgram(idProgramShader);
	setUniforms();

	/* Enable DEPTH-TEST */
	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window)) {
		render();
		updateScene();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanUp();
	return 0;
}