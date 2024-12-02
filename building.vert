#version 120

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

attribute vec3 position;
attribute vec3 normal;
attribute vec2 texCoord;

varying vec3 fragNormal;
varying vec3 fragPosition;
varying vec2 fragTexCoord;

void main() {
    fragPosition = vec3(modelViewMatrix * vec4(position, 1.0));
    fragNormal = normalMatrix * normal;
    fragTexCoord = texCoord;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}