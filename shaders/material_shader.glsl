#version 410 core

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirectionalLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight{
    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

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

#define POINTLIGHT_COUNT 4

uniform DirectionalLight dirLight;
uniform PointLight pointLights[POINTLIGHT_COUNT];
uniform SpotLight spotLight;

vec3 CalculateDirectionLight( DirectionalLight light, vec3 normal, vec3 viewDir );
vec3 CalculatePointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalculateSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

void main(){
    vec3 norm = normalize( normal );
    vec3 viewDir = normalize(viewPos - fragPos);

    //phase 1: Directional Lighting
    vec3 lighting = CalculateDirectionLight( dirLight, norm, viewDir );
    //phase 2: Point Lighting
    for( int i = 0; i < POINTLIGHT_COUNT; i++ )
        lighting += CalculatePointLight( pointLights[i], norm, fragPos, viewDir );
    //phase 3: Spot Lighting
    lighting += CalculateSpotLight(spotLight, norm, fragPos, viewDir);
    
    FragColor = vec4( lighting * color, 1.0f );
}

vec3 CalculateDirectionLight( DirectionalLight light, vec3 normal, vec3 viewDir ){
    vec3 lightDir = normalize( -light.direction );
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    //combine results
    vec3 ambient = light.ambient * material.ambient; //* material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    return ambient + diffuse + specular;
}

vec3 CalculatePointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir ){
    vec3 lightDir = normalize(light.position - fragPos);
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    // attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + 
                                light.linear * distance + 
                                light.quadratic * pow(distance, 2.0f) );
    //combine results
    vec3 ambient = light.ambient * material.ambient; //* material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 CalculateSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir ){
    vec3 lightDir = normalize(light.position - fragPos);
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);

    // attenuation
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + 
                                light.linear * distance + 
                                light.quadratic * pow(distance, 2.0f) );

    //spot lighting
    float theta = dot( lightDir, normalize(-light.direction) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( (theta - light.outerCutOff) / epsilon, 0.0f, 1.0f );

    //combine results
    vec3 ambient = light.ambient * material.ambient; //* material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    ambient *= intensity * attenuation;
    diffuse *= intensity * attenuation;
    specular *= intensity * attenuation;

    return ambient + diffuse + specular;
}
