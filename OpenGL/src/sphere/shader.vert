#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

// Data from CPU 
uniform mat4 MVP;
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform float heightFactor;

// Texture-related data
uniform sampler2D heightMapTexture;
uniform int textureOffset;

// Output to Fragment Shader
out vec2 textureCoordinate;
out vec3 vertexNormal;
out vec3 ToLightVector;
out vec3 ToCameraVector;

float get_height(in vec2 xy) {
    vec4 value = texture(heightMapTexture, xy);
    float height = value.r;
    return height * heightFactor;
}

void main() {
    textureCoordinate = tex_coord;
    textureCoordinate.x += textureOffset * (1.0 / 250);
    vertexNormal = normal;
    vec3 heightOffset = vertexNormal * get_height(textureCoordinate);
    vec3 calculated_pos = position + heightOffset;
    ToCameraVector = normalize(cameraPos - position);
    ToLightVector = normalize(lightPos - position);
    gl_Position = MVP * vec4(calculated_pos.xyz, 1.0f);
}
