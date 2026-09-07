#include "entity.h"

Entity::Entity( Mesh *m, Shader *s ){
    mesh = m;
    shader = s;
    modelMatrix = glm::mat4(1.0f);
}

void Entity::translate( glm::vec3 vector ){
    position += vector;
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::rotate( float angle, glm::vec3 axis ){
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    modelMatrix *= glm::rotate( rotationMatrix, angle, axis );
}

void Entity::set_position( glm::vec3 vector ){
    position = vector;
    modelMatrix[3][0] = position.x;
    modelMatrix[3][1] = position.y;
    modelMatrix[3][2] = position.z;
}

void Entity::set_rotation( float rotation[9] ){
    modelMatrix[0][0] = rotation[0];
    modelMatrix[1][0] = rotation[1];
    modelMatrix[2][0] = rotation[2];

    modelMatrix[0][1] = rotation[3];
    modelMatrix[1][1] = rotation[4];
    modelMatrix[2][1] = rotation[5];

    modelMatrix[0][2] = rotation[6];
    modelMatrix[1][2] = rotation[7];
    modelMatrix[2][2] = rotation[8];
}

void Entity::set_scale( glm::vec3 vector ){
    scale = vector;
    modelMatrix = glm::scale( modelMatrix, vector );
}

void Entity::render(){
    shader->set_matrix( "model", modelMatrix );
    mesh->draw();
}