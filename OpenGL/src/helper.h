#ifndef __HELPER__H__
#define __HELPER__H__

#include <iostream>
#include <string>
#include <fstream>
#include <jpeglib.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>


extern GLuint idProgramShader;
extern GLuint idFragmentShader;
extern GLuint idVertexShader;
extern GLuint idJpegTexture;

void initShaders();

GLuint initVertexShader(const std::string& filename);

GLuint initFragmentShader(const std::string& filename);

bool readDataFromFile(const std::string& fileName, std::string &data);

void initTexture(char *filename,int *w, int *h);

/* Represent a vertex using below struct */
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coord;
	/* We do not need an additional color field in here since we calculate
	 * the color of the vertices using Phong Shading in our Shaders */
};

#endif
