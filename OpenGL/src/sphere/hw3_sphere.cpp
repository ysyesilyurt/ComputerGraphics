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
 * are VetexBufferObjects. Hence we will keep 1 VAO (and then be done with it) handle all our tasks using our VBOs */
GLuint vertexArrayObject;
// VBOs -- we keep one for vertexAttributes and one for indices
GLuint terrainVertexAttribBuffer, terrainIndexBuffer;

/* CPU backed Containers for our vertex data */
std::vector<int> indices;
std::vector<Vertex> vertices;

/* Sphere Related variables */
// TODO: center (0,0,0)
const int horizontal_split_count = 250;
const int vertical_split_count = 125;
const int radius = 350;

/* Texture variables */
int textureWidth, textureHeight, heightTextureWidth, heightTextureHeight;

/* Window variables */
static GLFWwindow * window = nullptr;
int windowX = 1000;
int windowY = 1000;

/* Geometry variables */
glm::vec3 pos, gaze, up;
glm::mat4 MVP, M_model, M_view, M_projection;
glm::vec3 light_pos;
float camSpeed = 0.0;
float heightFactor = 0.0;
// TODO: Initial values of the pitch and yaw
float pitch = 45.0;
float yaw = 90.0;
float fovy = 45.0;
float aspectRatio = 1.0;
float near = 0.1;
float far = 1000.0;

/* Uniform variables */ // TODO: Later alter the names of these and in below
int MVP_location, height_location, tex_w_location, tex_h_location, cam_pos_location,
heightmap_location, texture_location, light_pos_location;

/* Key flags */
bool heightFactor_increase = false;
bool heightFactor_decrease = false;
bool pitch_increase = false;
bool pitch_decrease = false;
bool yaw_increase = false;
bool yaw_decrease = false;
bool stop_cam = false;
bool camSpeed_increase = false;
bool camSpeed_decrease = false;
bool move_map_left = false;
bool move_map_right = false;
bool rollback_to_initial = false;
bool move_light_left = false;
bool move_light_right = false;
bool move_light_up = false;
bool move_light_down = false;
bool light_y_increase = false;
bool light_y_decrease = false;

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
	float x, y, z, xy;                              // vertex position
	float u, v;                                     // vertex texCoord
	float alpha, beta;
	for (int i = 0; i <= vertical_split_count; i++) {
		beta = M_PI / 2 - (i * M_PI / vertical_split_count); // starting from pi/2 to -pi/2
		xy = radius * cosf(beta);             // r * cos(u)
		z = radius * sinf(beta);              // r * sin(u)
		for (int j = 0; j <= horizontal_split_count; j++) {
			alpha = j * 2 * M_PI / horizontal_split_count;  // starting from 0 to 2pi

			// vertex position (cx + x, cy + y, cz + z)
			x = xy * cosf(alpha);             // r * cos(u) * cos(v)
			y = xy * sinf(alpha);             // r * cos(u) * sin(v)

			// vertex tex coord (s, t) range between [0, 1]
			u = (float)j / horizontal_split_count;
			v = (float)i / vertical_split_count;

			Vertex vertex;
			vertex.position = glm::vec3(x, y, z);
			vertex.normal = glm::vec3(x/radius, y/radius, z/radius); // remember n = p-c/r
			vertex.normal = glm::normalize(vertex.normal);
//			vertex.normal = glm::vec3(0.0);
			vertex.tex_coord = glm::vec2(u, v);

			vertices.push_back(vertex);
		}
	}
}

void initializeIndices() {
	/* Initialize indices per pixel, be careful about the winding order! */
	int k1, k2;
	for(int i = 0; i < vertical_split_count; ++i)	{
		k1 = i * (horizontal_split_count + 1);     // beginning of current stack
		k2 = k1 + horizontal_split_count + 1;      // beginning of next stack

		for(int j = 0; j < horizontal_split_count; ++j, ++k1, ++k2) {
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if(i != 0) {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if(i != (horizontal_split_count - 1)) {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(0)); // TODO: convert to (void *)(0) if applicable
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offsetof(Vertex, normal))); // TODO: (void*)(offsetof(Vertex, normal))
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offsetof(Vertex, tex_coord))); // TODO: (void*)(offsetof(Vertex, tex_coord))

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void setupGeometry() {

	/* Initialize Cam vectors first */
	pitch = 0.0;
	pos = glm::vec3(0, 600, -1000); // also make pitch = 0.0
	gaze = glm::vec3(0.0, -1.0, 0.0);
	up = glm::vec3(0.0, 0.0, 1.0);
//	pos = glm::vec3(0, 600, 0);
//	gaze = glm::vec3(0.0, -1.0, 0.0);
//	up = glm::vec3(0.0, 0.0, 1.0);

	/* Now Set MVP */
	M_model = glm::rotate(M_model, (float) glm::radians(-60.0), glm::vec3(1, 0, 0)); // TODO: GET A More precise orientation
	M_view = glm::lookAt(pos, pos + gaze, up); // Will be updated during flying
	M_projection = glm::perspective(fovy, aspectRatio, near, far); // Will be updated during flying
	MVP = M_projection * M_view * M_model;

	/* Set initial Light Position */
	light_pos = glm::vec3(0, 2500, 0); // TODO: Fix the highlight!
}

void setUniforms() {
	/* Set the uniform variables so that our shaders can access them */ // TODO: later change the names
	MVP_location = glGetUniformLocation(idProgramShader, "MVP");
	glUniformMatrix4fv(MVP_location, 1, GL_FALSE, glm::value_ptr(MVP));

	height_location = glGetUniformLocation(idProgramShader, "heightFactor");
	glUniform1f(height_location, heightFactor);

	tex_w_location = glGetUniformLocation(idProgramShader, "textureWidth");
	glUniform1i(tex_w_location, textureWidth);

	tex_h_location = glGetUniformLocation(idProgramShader, "textureHeight");
	glUniform1i(tex_h_location, textureHeight);

	cam_pos_location = glGetUniformLocation(idProgramShader, "cameraPos");
	glUniform3fv(cam_pos_location, 1, glm::value_ptr(pos));

	light_pos_location = glGetUniformLocation(idProgramShader, "lightPos");
	glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));

	heightmap_location = glGetUniformLocation(idProgramShader, "heightMapTexture");
	glUniform1i(heightmap_location, 0);

	texture_location = glGetUniformLocation(idProgramShader, "rgbTexture");
	glUniform1i(texture_location, 1);
}

void render() {
	/* First clear all buffers */
	glClearColor(0,0,0,1);
	glClearDepth(1.0); // TODO: DONT KNOW if WE NEED?
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
	M_projection = glm::perspective(fovy, aspectRatio, near, far); // Will be updated during flying
	MVP = M_projection * M_view * M_model;

	// Do not forget the update the uniforms of geometry too
	glUniformMatrix4fv(MVP_location, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform3fv(cam_pos_location, 1, glm::value_ptr(pos));
	glUniform1f(height_location, heightFactor);

	// Finally draw the scene using indices..
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}

void toggleScreens() {
	static bool isFullscreen = false;

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

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

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
			glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
			break;
		case 1: // right
			light_pos.x -= 5;
			glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
			break;
		case 2: // up
			light_pos.z += 5;
			glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
			break;
		case 3: // down
			light_pos.z -= 5;
			glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
			break;
		case 4: // increase height
			light_pos.y += 5;
			glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
			break;
		case 5: // decrease height
			light_pos.y -= 5;
			glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
			break;
		default:
			std::cout << "Problem Encountered during Light Pos Update!\n";
			break;
	}
}

void updateScene() {
	if (heightFactor_increase)
		heightFactor += 0.5;
	if (heightFactor_decrease)
		heightFactor -= 0.5;

	if (move_map_left) {
//		move_map_angle =  TODO refactor
		M_model = glm::rotate(M_model, (float) glm::radians(0.5), glm::vec3(0, 0, 1));
		light_pos = glm::rotate(light_pos, (float) glm::radians(-0.5), glm::vec3(0, 0, 1));
		glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
	}
	else if (move_map_right) {
		M_model = glm::rotate(M_model, (float) glm::radians(-0.5), glm::vec3(0, 0, 1));
		light_pos = glm::rotate(light_pos, (float) glm::radians(+0.5), glm::vec3(0, 0, 1));
		glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
	}

	// tODO: fix dis using q/e angle
	if (rollback_to_initial) {
//		the plane will be placed to the initial position with initial configurations of the camera and speed of 0
		pos = glm::vec3(0, 600, -1000); // also make pitch = 0.0
		light_pos = glm::vec3(0, 2500, 0); // TODO: Fix the highlight!
		glUniform3fv(light_pos_location, 1, glm::value_ptr(light_pos));
		camSpeed = 0;
//		pitch = 45.0;
		pitch = 0.0;
		yaw = 90.0;
		heightFactor = 0.0;
	}

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
}

static void resizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

static void errorCallback(int error, const char * description) {
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char * argv[]) {

	if (argc != 3) {
		printf("Please run the executable as ./hw3_sphere <height_map.jpg> <texture_map.jpg>\n");
		exit(-1);
	}

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		exit(-1);
	}

//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // BEWARE: IMPORTANT!
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

//	 TODO: GUY'S..
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);


	window = glfwCreateWindow(windowX, windowY, "CENG477 - HW3", nullptr, nullptr);

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

	// TODO: polygon mode? wireframe?

	/* Enable DEPTH-TEST */
	glEnable(GL_DEPTH_TEST);

	/* ?? Enable STENCIL-TEST */
	/* ?? Enable LIGHT0 ? */

	while (!glfwWindowShouldClose(window)) {
		render();
		updateScene();
		glfwSwapBuffers(window);
		glfwPollEvents(); // TODO: what does dis do?
	}
	cleanUp();
	return 0;
}