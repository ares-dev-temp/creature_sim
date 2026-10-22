#pragma once

#include <iostream>

#include "shader.h"

class Skybox{
    public:
        Skybox( Shader *s, std::string faces[6], float* vertices, unsigned int* triangles);

        Shader *shader;

        unsigned int cubemapTextureID;

        unsigned int skyboxVAO;

        unsigned int loadCubemap( std::string faces[6] );

        void draw( glm::mat4 viewMatrix, glm::mat4 perspectiveMatrix );
};
