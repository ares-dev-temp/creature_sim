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

void Entity::set_scale( glm::vec3 vector ){
    scale = vector;
    modelMatrix = glm::scale( modelMatrix, vector );
}

void Entity::render(){
    shader->set_matrix( "model", modelMatrix );
    mesh->draw();
}