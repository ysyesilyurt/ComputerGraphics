#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

// Data from CPU 
uniform mat4 MVP; // ModelViewProjection Matrix
uniform vec3 cameraPosition;
uniform float heightFactor;

// Texture-related data
uniform sampler2D rgbTexture; // TODO: get a height texture and this and USE THEM ACCORDINGLY
uniform int textureWidth; // TODO: CHANGE BELOW acc. to names of these uniform vars
uniform int textureHeight;

// Output to Fragment Shader
out vec2 textureCoordinate; // For texture-color
out vec3 vertexNormal; // For Lighting computation
out vec3 ToLightVector; // Vector from Vertex to Light;
out vec3 ToCameraVector; // Vector from Vertex to Camera;

float get_height(in vec2 xy) {
    vec4 color = texture(rgbTexture, xy);
    float height = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b; // color.r
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
    textureCoordinate = tex_coord;
    vec3 calculated_pos = vec3(position.x, get_height(textureCoordinate), position.z);
    ToCameraVector = normalize(cameraPosition - calculated_pos);

    vec3 light_pos = vec3(textureWidth / 2.0f, textureWidth + textureHeight, textureHeight / 2.0f);
    ToLightVector = normalize(light_pos - calculated_pos);

    vertexNormal = calculate_normal();

    gl_Position = MVP * vec4(calculated_pos.xyz, 1.0f);
}
