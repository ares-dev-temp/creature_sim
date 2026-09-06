#pragma once

#include <glm/glm.hpp>
#include "../rendering/mesh.h"
#include "../rendering/shader.h"

class Entity{
    public:
        glm::vec3 position, rotation, scale;
        Mesh *mesh;
        Shader *shader;
        glm::mat4 modelMatrix;

        Entity(Mesh *mesh, Shader *shader);
        //~Entity();

        void translate( glm::vec3 vector );

        void rotate( float angle, glm::vec3 axis );

        void set_position( glm::vec3 vector );

        void set_scale( glm::vec3 vector );

        void render();

};