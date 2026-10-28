#version 410 core

layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aColor;
//layout (location = 2) in vec2 aTexCoords;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 normal;
out vec3 fragPos;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main(){
    TexCoord = vec2(aTexCoords.x, aTexCoords.y);
    fragPos = vec3( model * vec4(aPos, 1.0f) );
    normal = mat3( transpose( inverse(model) ) ) * aNormal;
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0f);
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}

