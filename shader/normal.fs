#version 330 core

in vec2 texCoord;
in vec3 position;
in vec3 normal;
in vec3 tangent;

out vec4 fragColor;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform sampler2D diffuse;
uniform sampler2D normalMap; 

void main() {
    vec3 texColor = texture(diffuse, texCoord).xyz;
    vec3 texNorm = texture(normalMap, texCoord).xyz * 2.0 - 1.0;
    vec3 N = normalize(normal);
    vec3 T = normalize(vec3(tangent));
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    vec3 pixelNorm = normalize(TBN * texNorm) ;

    // 0~1인 rgb를 -1~-1로 변경
    // vec3 pixelNorm = normalize((texture(normalMap, texCoord).xyz * 2.0 - 1.0));
    vec3 ambient = texColor * 0.2;

    vec3 lightDir = normalize(lightPos - position);
    float diff = max(dot(pixelNorm, lightDir), 0.0);
    vec3 diffuse = diff * texColor * 0.8;

    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * vec3(0.5);

    fragColor = vec4(ambient + diffuse + specular, 1.0);
}