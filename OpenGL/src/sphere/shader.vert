#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

// Data from CPU 
uniform mat4 MVP; // ModelViewProjection Matrix
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform float heightFactor;

// Texture-related data
uniform sampler2D heightMapTexture;
uniform sampler2D rgbTexture;
uniform int textureWidth;
uniform int textureHeight;

// Output to Fragment Shader
out vec2 textureCoordinate; // For texture-color
out vec3 vertexNormal; // For Lighting computation
out vec3 ToLightVector; // Vector from Vertex to Light;
out vec3 ToCameraVector; // Vector from Vertex to Camera;

float get_height(in vec2 xy) {
    vec4 value = texture(heightMapTexture, xy); // TODO: using rgbTexture..
    float height = value.r;
    return height * heightFactor;
}

vec2 get_coordinates(in vec2 xy_offset) {
    float dx = 1.0f / textureWidth;
    float dz = 1.0f / textureHeight;

    vec2 coord = vec2(1.0f - (position.x + xy_offset.x) * dx, 1.0f - (position.z + xy_offset.y) * dz);
    return coord;
}

vec3 calculate_normal() {
    vec3 left =       vec3(position.x - 1.0f , get_height(get_coordinates(vec2(-1.0f, 0.0f))), position.z);
    vec3 right =      vec3(position.x + 1.0f, get_height(get_coordinates(vec2(1.0f, 0.0f)) ), position.z);
    vec3 top =        vec3(position.x , get_height(get_coordinates(vec2(0.0f, -1.0f))), position.z - 1.0f);
    vec3 bottom =     vec3(position.x , get_height(get_coordinates(vec2(0.0f, 1.0f)) ), position.z + 1.0f);
    vec3 topright =   vec3(position.x + 1.0f, get_height(get_coordinates(vec2(1.0f, 1.0f)) ), position.z - 1.0f);
    vec3 bottomleft = vec3(position.x - 1.0f, get_height(get_coordinates(vec2(-1.0f, -1.0f))), position.z + 1.0f);

    vec3 pos = vec3(position.x, get_height(textureCoordinate), position.z);
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
//    float dx = 1.0 / textureWidth;
//    float dz = 1.0 / textureHeight;
    textureCoordinate = tex_coord;
//    textureCoordinate.x += 1000000 * dx;
    vec3 calculated_pos = position;

    ToCameraVector = normalize(cameraPos - calculated_pos);
    ToLightVector = normalize(lightPos - calculated_pos);
    vertexNormal = normal;

    gl_Position = MVP * vec4(calculated_pos.xyz, 1.0f);
}
