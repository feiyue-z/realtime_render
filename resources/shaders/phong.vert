#version 330 core

layout(location = 0) in vec3 objectSpacePos;
layout(location = 1) in vec3 objectSpacrNormal;

out vec3 worldSpacePos;
out vec3 worldSpaceNormal;
out vec2 uvPos;

uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    // world space position
    worldSpacePos = vec3(modelMatrix * vec4(objectSpacePos, 1.0));

    // world space normal
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    worldSpaceNormal = normalize(normalMatrix * objectSpacrNormal);

    // clip space position
    gl_Position = projectionMatrix * viewMatrix * vec4(worldSpacePos, 1.0);
}
