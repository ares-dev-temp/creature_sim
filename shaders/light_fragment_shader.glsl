#version 410 core

struct Light{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}

out vec4 FragColor;

uniform vec3 lightColor;

uniform Light light;

void main(){
    FragColor = vec4( lightColor, 1.0f );
}
