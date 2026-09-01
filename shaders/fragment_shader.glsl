#version 410 core

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

void main(){
    //FragColor = vec4(ourColor, 1.0);
    //FragColor = mix( texture(tex_1, TexCoord), texture(tex_2, TexCoord), 0.2 );

    //FragColor = vec4(color * lightColor, 1.0f);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize( normal );
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max( dot(norm, lightDir), 0.0f );
    vec3 diffuse = lightColor * diff;

    vec3 result = (ambient + diffuse) * color;
    FragColor = vec4( result, 1.0f );
} 