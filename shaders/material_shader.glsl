#version 410 core

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light{
    vec3 position;
    vec3 direction;

    //vec3 ambient;
    //vec3 diffuse;
    //vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
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
uniform vec3 viewPos;

uniform Material material;
uniform Light light;

void main(){
    //ambient
    vec3 ambient = lightColor * material.ambient;

    vec3 norm = normalize( normal );
    vec3 lightDir = normalize(light.position - fragPos);
    //vec3 lightDir = normalize(-light.direction);      //for directional lighhs

    //diffuse
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = lightColor * (diff * material.diffuse);

    //for spotlights
    float theta = dot( lightDir, normalize(-light.direction) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( (theta - light.outerCutOff) / epsilon, 0.0f, 1.0f );

    //if( theta > light.cutOff ){
    //for points lights
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + 
                                (light.linear * distance) + 
                                (light.quadratic * pow(distance, 2.0f)) );

    //specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specular = material.specular * spec * lightColor;

    
    //for point lights
    //ambient *= attenuation;
    //diffuse *= attenuation;
    //specular *= attenuation;

    //for spotlights
    diffuse *= intensity;
    specular *= intensity;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0f);
    //}else{
    //    FragColor = vec4( ambient * color, 1.0f );
    //}
}
