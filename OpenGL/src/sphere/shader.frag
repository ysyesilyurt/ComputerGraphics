#version 330 core

// Output Color
out vec4 color;

// Texture-related data;
uniform sampler2D rgbTexture;

// Data from Vertex Shader
in vec2 textureCoordinate;
in vec3 vertexNormal;
in vec3 ToLightVector;
in vec3 ToCameraVector;

void main() {
  vec4 textureColor = texture(rgbTexture, textureCoordinate);

  vec4 k_a = vec4(0.25, 0.25, 0.25, 1.0); // reflectance coeff. for ambient
  vec4 k_d = vec4(1.0, 1.0, 1.0, 1.0); // reflectance coeff. for diffuse
  vec4 k_s = vec4(1.0, 1.0, 1.0, 1.0); // reflectance coeff. for specular
  vec4 i_a = vec4(0.3, 0.3, 0.3, 1.0); // light color for ambient
  vec4 i_d = vec4(1.0, 1.0, 1.0, 1.0); // light color for diffuse
  vec4 i_s = vec4(1.0, 1.0, 1.0, 1.0); // light color for specular
  int exponent = 100; // specular exponent

  vec3 ambient = (k_a * i_a).xyz;
  float cosTheta = max(dot(vertexNormal, ToLightVector), 0.0f);
  vec3 diffuse = cosTheta * (i_d * k_d).xyz;
  vec3 reflected = reflect(-normalize(ToLightVector - 0.5), vertexNormal);
  float cosAlpha = pow(max(dot(reflected, normalize(ToCameraVector + 5)), 0.0f), exponent);
  vec3 specular = cosAlpha * (i_s * k_s).xyz;

  vec3 computedSurfaceColor = ambient + diffuse + specular;
  color = vec4(clamp(textureColor.xyz * computedSurfaceColor, 0.0, 1.0), 1.0);
}
