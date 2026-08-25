#version 410 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D tex_1;
uniform sampler2D tex_2;

void main(){
    //FragColor = vec4(ourColor, 1.0);
    FragColor = mix( texture(tex_1, TexCoord), texture(tex_2, TexCoord), 0.2 );
} 