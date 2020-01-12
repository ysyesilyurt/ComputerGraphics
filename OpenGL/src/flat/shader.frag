#version 330 core

// Output Color
out vec4 color;

// Texture-related data;
uniform sampler2D rgbTexture;

// Data from Vertex Shader
in vec2 textureCoordinate;
in vec3 vertexNormal; // For Lighting computation
in vec3 ToLightVector; // Vector from Vertex to Light;
in vec3 ToCameraVector; // Vector from Vertex to Camera;

void main() {

  // Assignment Constants below
  // get the texture color
  vec4 textureColor = texture(rgbTexture, textureCoordinate);

  // apply Phong shading by using the following parameters
  vec4 ka = vec4(0.25, 0.25, 0.25, 1.0); // reflectance coeff. for ambient
  vec4 Ia = vec4(0.3, 0.3, 0.3, 1.0); // light color for ambient
  vec4 Id = vec4(1.0, 1.0, 1.0, 1.0); // light color for diffuse
  vec4 kd = vec4(1.0, 1.0, 1.0, 1.0); // reflectance coeff. for diffuse
  vec4 Is = vec4(1.0, 1.0, 1.0, 1.0); // light color for specular
  vec4 ks = vec4(1.0, 1.0, 1.0, 1.0); // reflectance coeff. for specular
  int exponent = 100; // specular exponent

  // compute ambient component
  vec3 ambient = (ka * Ia).xyz;
  ambient = vec3(0.0f, 0.0f, 0.0f);

  // compute diffuse component
  float cosTheta = max(dot(vertexNormal, ToLightVector), 0.0f);
  vec3 diffuse = cosTheta * (Id * kd).xyz;
  // diffuse = vec3(0.0f, 0.0f, 0.0f);

  // compute specular component
  vec3 reflected = reflect(-ToLightVector, vertexNormal);
  float cosAlpha = pow(max(dot(reflected, ToCameraVector), 0.0f), exponent);
  vec3 specular = cosAlpha * (Is * ks).xyz;
  // specular = vec3(0.0f, 0.0f, 0.0f);

  vec3 computedSurfaceColor = ambient + diffuse + specular;

  // compute the color using the following equation
  color = vec4(clamp(textureColor.xyz * computedSurfaceColor, 0.0, 1.0), 1.0);
}