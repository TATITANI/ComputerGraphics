#version 330 core
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 transform;
uniform mat4 modelTransform;

out vec3 normal;
out vec2 texCoord;
out vec3 position;

void main() {
  // 클립 스페이스 좌표(0~1)
  gl_Position = transform * vec4(aPos, 1.0); 
  // world 좌표로 변환
  // 점이 아닌 벡터에는 transpose(inverse를 해야 world 변환이 가능
  normal = (transpose(inverse(modelTransform)) * vec4(aNormal, 0.0)).xyz;
  texCoord = aTexCoord;
  position = (modelTransform * vec4(aPos, 1.0)).xyz;
}