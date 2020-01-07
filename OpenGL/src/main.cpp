#include <GL/glew.h>
#include <glm/ext.hpp>
#include <vector>
#include "helper.h"

// Shaders
GLuint idProgramShader;
GLuint idFragmentShader;
GLuint idVertexShader;
GLuint idJpegTexture;
GLuint idMVPMatrix;

/* Think of our data as a company, VertexArrayObject is the boss and it manages several staff members which
 * are VetexBufferObjects. Hence we will keep 1 VAO (and then be done with it) handle all our tasks using our VBOs */
GLuint vertexArrayObject;
// VBOs -- we keep one for vertexAttributes and one for indices
GLuint terrainVertexAttribBuffer, terrainIndexBuffer;

/* CPU backed Containers for our vertex data */
std::vector<int> indices;
std::vector<Vertex> vertices;

/* Texture variables */
int textureWidth, textureHeight, heightTextureWidth, heightTextureHeight;

/* Window variables */
static GLFWwindow * window = nullptr;
int windowX = 1000;
int windowY = 1000;

/* Geometry variables */
glm::vec3 pos, gaze, up;
glm::mat4 MVP, M_model, M_view, M_projection;
float camSpeed = 0.0f;
float heightFactor = 10.0f;
// TODO: Initial values of the pitch and yaw
float pitch = 45.0f;
float yaw = 90.0f;

/* Uniform variables */ // TODO: Later alter the names of these and in below
int MVP_location;
int height_location;
int tex_w_location ;
int tex_h_location;
int cam_pos_location;

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
	const float dx = 1.0f / textureWidth;
	const float dz = 1.0f / textureHeight;

	for (int z = 0; z < textureHeight + 1; z++) {
		for (int x = 0; x < textureWidth + 1; x++) {
			Vertex vertex;
			vertex.position = glm::vec3(x, 0.0f, z); // y-coords of the vertices will come the from vertex shader from the corresponding texture color(only R channel) on the heightmap image
			vertex.normal = glm::vec3(0.0f); // TODO::::: IF NOT NEEDED IN VERTEX SHADER etc. REMOVE FROM HERE AND EVERYWHERE..
			vertex.tex_coord = glm::vec2(1.0f - x * dx, 1.0f - z * dz); // fetch(p, q).(1 – dx).(1 – dy)

			vertices.push_back(std::move(vertex)); // TODO: check the highlight effects
		}
	}
}

void initializeIndices() {
	/* Initialize indices per pixel, be careful about the winding order! */
	int size = (textureWidth + 1) * (textureHeight + 1);
	for (int i = textureWidth + 2; i < size; i++) { // TODO: REFACTOR
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(0)); // TODO: convert to (void *)(0) if applicable
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offsetof(Vertex, normal))); // TODO: (void*)(offsetof(Vertex, normal))
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offsetof(Vertex, tex_coord))); // TODO: (void*)(offsetof(Vertex, tex_coord))

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
}

void setupGeometry() {

	/* Initialize Cam vectors first */
	pos = glm::vec3(textureWidth / 2.0f, textureWidth / 10.0f, -textureWidth / 4.0f);
	gaze = glm::vec3(0.0f, 0.0f, 1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f); // Warning: up vector?
//	glm::vec3 camera_cross = cross(camera_up, camera_gaze); // TODO: HEEEY! dont we need cross

	/* Now Set MVP */
	M_model = glm::mat4(1.0f); // Will not change again
	M_view = glm::lookAt(pos, pos + gaze, up); // Will be updated during flying TODO: pos + gaze * 0.1f ??
	// ... a perspective projection with an angle of 45 degrees with the aspect ratio of 1,
	// ... near and far plane will be 0.1 and 1000 respectively
	M_projection = glm::perspective(45.0f, 1.0f, 0.1f, 1000.0f); // Will be updated during flying

	MVP = M_projection * M_view * M_model;
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

	cam_pos_location = glGetUniformLocation(idProgramShader, "cameraPosition");
	glUniform3fv(cam_pos_location, 1, glm::value_ptr(pos));
}

void render() {

	/* First clear all buffers */
	glClearColor(0,0,0,1);
	glClearDepth(1.0f); // TODO: DONT KNOW WE NEED?
//	glClearColor(0.4f, 0.4f, 0.3f, 1.0f); // WARNING: WHY THOSE VALUES?
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Now render the frame */
	// First update the gaze acc. to new values of yaw and pitch
	gaze.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	gaze.y = sin(glm::radians(pitch - 45.0f));
	gaze.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	gaze = glm::normalize(gaze); // do not forget the normalize the gaze

	// Then update the position of the camera
	pos += camSpeed * gaze;
	M_view = glm::lookAt(pos, pos + gaze, up); // Will be updated during flying
	M_projection = glm::perspective(45.0f, 1.0f, 0.1f, 1000.0f); // TODO: Check if this statement is necessary or not since the value does not change?
	MVP = M_projection * M_view * M_model;

	// Do not forget the update the uniforms of geometry too
	glUniformMatrix4fv(MVP_location, 1, GL_FALSE, glm::value_ptr(MVP));
	glUniform3fv(cam_pos_location, 1, glm::value_ptr(pos));
	glUniform1f(height_location, heightFactor);

	// Finally draw the scene using indices..
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
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

//	glfwSetFramebufferSizeCallback(window, resizeCallback); FOR RESIZE CALLBACK
	initShaders();
//	initTexture(argv[1], &heightTextureWidth, &heightTextureHeight);
	initTexture(argv[1], &textureWidth, &textureHeight);

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
		glfwSwapBuffers(window);
		/* TODO: a key callback or a handleInputs function that is called continiously?
		 * 	- handle inputs
		 * 		- altering heightFactor R and F keys
		 * 		- moving the map using Q and E keys
		 * 		- moving the light source using arrow keys
		 * 		- altering height (y position) of the light source using T and G keys
		 * 		- altering pitch using W and S keys
		 * 		- altering yaw using A and D keys
		 * 		- altering camSpeed using Y and H keys
		 * 		- zeroing camSpeed using X key
		 * 		- converting back to initial conf using I key (initial cam pos, speed = 0 and etc.)
		 * 		- toggling to fullscreen using P key
		 */
		glfwPollEvents(); // TODO: what does dis do?
	}
	cleanUp();
	return 0;
}