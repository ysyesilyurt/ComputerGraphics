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
uniform int textureWidth;
uniform int textureHeight;
uniform int textureOffset;

// Output to Fragment Shader
out vec3 vertexNormal;
out vec3 ToLightVector;
out vec3 ToCameraVector;
out vec2 textureCoordinate;

// Global VS variables
float dx = 1.0f / textureWidth;
float dz = 1.0f / textureHeight;
float texOffset = textureOffset * dx;

float calcHeight(in vec2 xy) {
    vec4 value = texture(heightMapTexture, xy);
    return value.r * heightFactor;
}

vec2 calcCoords(in vec2 xy_offset) {
    return vec2(1.0f - ((position.x + xy_offset.x) * dx) + texOffset, 1.0f - (position.z + xy_offset.y) * dz);
}

vec3 calcNormal(in vec3 pos) {
    vec3 left = vec3(position.x - 1.0f, calcHeight(calcCoords(vec2(-1.0f, 0.0f))), position.z);
    vec3 right = vec3(position.x + 1.0f, calcHeight(calcCoords(vec2(1.0f, 0.0f))), position.z);
    vec3 top = vec3(position.x , calcHeight(calcCoords(vec2(0.0f, -1.0f))), position.z - 1.0f);
    vec3 bottom = vec3(position.x , calcHeight(calcCoords(vec2(0.0f, 1.0f))), position.z + 1.0f);
    vec3 topright = vec3(position.x + 1.0f, calcHeight(calcCoords(vec2(1.0f, 1.0f))), position.z - 1.0f);
    vec3 bottomleft = vec3(position.x - 1.0f, calcHeight(calcCoords(vec2(-1.0f, -1.0f))), position.z + 1.0f);
    vec3 norm = vec3(0.0f, 0.0f, 0.0f);
    norm += cross(right - pos, topright - pos);
    norm += cross(topright - pos, top - pos);
    norm += cross(top - pos, left - pos);
    norm += cross(left - pos, bottomleft - pos);
    norm += cross(bottomleft - pos, bottom - pos);
    norm += cross(bottom - pos, right - pos);
    norm = normalize(norm);
    return norm;
}

void main() {
    textureCoordinate = tex_coord;
    textureCoordinate.x += texOffset;
    vec3 pos = vec3(position.x, calcHeight(textureCoordinate), position.z);
    ToCameraVector = normalize(cameraPos - pos);
    ToLightVector = normalize(lightPos - pos);
    vertexNormal = calcNormal(pos);
    gl_Position = MVP * vec4(pos.xyz, 1.0f);
}
