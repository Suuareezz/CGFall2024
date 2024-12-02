#version 120

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

varying vec3 fragNormal;
varying vec3 fragPosition;
varying vec2 fragTexCoord;

void main() {
    // Normal mapping
    vec3 normal = normalize(fragNormal);
    vec3 normalMap = texture2D(normalMap, fragTexCoord).rgb * 2.0 - 1.0;
    normal = normalize(normal + normalMap);

    // Diffuse lighting
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * texture2D(diffuseMap, fragTexCoord).rgb;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * vec3(0.5);

    // Final color
    gl_FragColor = vec4(diffuse + specular, 1.0);
}