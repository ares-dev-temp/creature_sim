#include "skybox.h"

#include <glad/glad.h>
#include "../libs/stb_image.h"

Skybox::Skybox( Shader *s, std::string faces[6], float* vertices, unsigned int* triangles ){
    shader = s;

    unsigned int skyboxVBO, skyboxEBO;
    
    std::vector<float> cube_vertex;
    for( int i = 0; i < 6 * 4 * 3; i++ ){
        int idx = (i % 3) + (i / 3) * 8;
        //cube_vertex.push_back( vertices[idx] * 2.0f );
        cube_vertex.push_back( *(vertices + idx) * 2.0f );
        //std::cout << vertices[idx] << std::endl;
    }

    //for( int i = 0; i < 36; i++ )
    //    std::cout << triangles[i] << std::endl;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);

    //bind the Vertex Array Object first, then bind and set vertex buffers, and them configure vertex attributes
    glBindVertexArray(this->skyboxVAO);

    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cube_vertex.size(), cube_vertex.data(), GL_STATIC_DRAW);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, skyboxEBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 36, triangles, GL_STATIC_DRAW );

    // 1. then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    this->cubemapTextureID = loadCubemap(faces);

    //draw skybox
    shader->use();
    shader->set_texture("skybox", 0);
}

void Skybox::draw( glm::mat4 viewMatrix, glm::mat4 perspectiveMatrix ){
    glDepthFunc( GL_LEQUAL );
    shader->use();
    glm::mat4 view = glm::mat4(glm::mat3( viewMatrix ));
    shader->set_matrix( "view", view );
    shader->set_matrix( "projection", perspectiveMatrix );
    glBindVertexArray(this->skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubemapTextureID);
    glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );
    //glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

unsigned int Skybox::loadCubemap( std::string faces[6] ){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for( unsigned int i = 0; i < 6; i++ ){
        unsigned char *data = stbi_load( faces[i].c_str(), &width, &height, &nrChannels, 0 );
        if(data){
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
            0, GL_RGB, GL_UNSIGNED_BYTE, data );
            stbi_image_free( data );
        }
        else{
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
}