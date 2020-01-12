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
//uniform sampler2D rgbTexture; TODO
uniform int textureWidth; // TODO remove later from openGl too
uniform int textureHeight;

// Output to Fragment Shader
out vec2 textureCoordinate; // For texture-color
out vec3 vertexNormal; // For Lighting computation
out vec3 ToLightVector; // Vector from Vertex to Light;
out vec3 ToCameraVector; // Vector from Vertex to Camera;

float get_height(in vec2 xy) {
    vec4 value = texture(heightMapTexture, xy); // TODO: using rgbTexture..
    float height = value.r;
    return height * heightFactor; // TODO: MAKE DIRECTION OF THE HEIGHT AS THE DIRECTION OF NORMAL!
}

void main() {
    textureCoordinate = tex_coord;
    vertexNormal = normal;
    vec3 heightOffset = vertexNormal * get_height(textureCoordinate); // calculating how much movement needs to be done TODO: yavuz sanity check later..
    vec3 calculated_pos = vec3(position.x + heightOffset.x, position.y + heightOffset.y, position.z + heightOffset.z);

    ToCameraVector = normalize(cameraPos - calculated_pos);
    ToLightVector = normalize(lightPos - calculated_pos);
    gl_Position = MVP * vec4(calculated_pos.xyz, 1.0f);
}
