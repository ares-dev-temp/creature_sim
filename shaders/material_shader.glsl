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

in vec2 TexCoord;
in vec3 normal;
in vec3 fragPos;
in vec4 fragPosLightSpace;

uniform sampler2D tex_1;
uniform sampler2D tex_2;
uniform sampler2D shadowMap;

uniform vec3 color;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform Material material;

#define POINTLIGHT_COUNT 4

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[POINTLIGHT_COUNT];
uniform SpotLight spotLight;

float ShadowCalculations( vec4 fragPosLightSpace, vec3 normal, vec3 lightDir );

vec3 CalculateDirectionLight( DirectionalLight light, vec3 normal, vec3 viewDir );
vec3 CalculatePointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow );
vec3 CalculateSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow );

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float near_plane = 1.0f;
    float far_plane = 7.5f;
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main(){
    vec3 norm = normalize( normal );
    vec3 viewDir = normalize(viewPos - fragPos);

    //calculate shadows
    //float shadow = ShadowCalculations( fragPosLightSpace );

    //phase 1: Directional Lighting
    vec3 lighting = CalculateDirectionLight( directionalLight, norm, viewDir );
    //phase 2: Point Lighting
    //for( int i = 0; i < POINTLIGHT_COUNT; i++ )
    //    lighting += CalculatePointLight( pointLights[i], norm, fragPos, viewDir, shadow );
    //phase 3: Spot Lighting
    //lighting += CalculateSpotLight(spotLight, norm, fragPos, viewDir, shadow);
    
    //vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
    vec3 new_color = color * lighting;

    //apply gamma correction
    float gamma = 2.2;
    new_color = pow( new_color, vec3(1.0/gamma) );

    FragColor = vec4( new_color, 1.0f );

    //float depthValue = texture(shadowMap, TexCoord).r;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    //FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}

float ShadowCalculations( vec4 fragPosLightSpace, vec3 normal, vec3 lightDir ){
    //perform perspective divides
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    //transform to [0, 1] range
    projCoords = projCoords * 0.5f + 0.5f;
    //get closest depth value from lights perspective (using [0, 1] range fragPosLight as coords)
    //float closestDepth = texture( shadowMap, projCoords.xy ).r;
    //get depth of current fragment from lights perspective
    float currentDepth = projCoords.z;
    //shadow bias to prevent shadow acne artifacts
    float bias = max(0.05f * (1.0f - dot(normal, lightDir)), 0.005f);
    //check whether current frag pos is in shadow
    //float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    float shadow = 0.0f;

    //outside of lights far plane
    if( projCoords.z > 1.0f )
        return 0.0f;

    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);
    for( int x = -1; x <= 1; ++x ){
        for( int y = -1; y <= 1; ++y ){
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;
    
    return shadow;
}

vec3 CalculateDirectionLight( DirectionalLight light, vec3 normal, vec3 viewDir ){
    vec3 lightDir = normalize( -light.direction );
    vec3 halfwayDir = normalize( lightDir + viewDir );
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), material.shininess);
    //combine results
    vec3 ambient = light.ambient * material.ambient; //* material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;

    //calculate shadows
    float shadow = ShadowCalculations( fragPosLightSpace, normal, lightDir );
    
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    return lighting;//ambient + diffuse + specular;
}

vec3 CalculatePointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow ){
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize( lightDir + viewDir );
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), material.shininess);
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

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    return lighting; //ambient + diffuse + specular;
}

vec3 CalculateSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow ){
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize( lightDir + viewDir );
    //diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    //specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), material.shininess);

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

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    return lighting; //ambient + diffuse + specular;
}
