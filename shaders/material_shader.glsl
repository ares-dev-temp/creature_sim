#version 410 core

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;
in vec3 normal;
in vec3 fragPos;

uniform sampler2D tex_1;
uniform sampler2D tex_2;

uniform vec3 color;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform Material material;

void main(){
    //ambient
    vec3 ambient = lightColor * material.ambient;

    vec3 norm = normalize( normal );
    vec3 lightDir = normalize(lightPos - fragPos);

    //diffuse
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = lightColor * (diff * material.diffuse);

    //specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specular = material.specular * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0f);
}
