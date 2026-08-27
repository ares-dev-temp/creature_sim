#include "mesh.h"
#include <glad/glad.h>


Mesh::Mesh( int vertexCount, float *vertexData, unsigned int *triangles, int vertex_size, int triangle_size ){
    //set up vertex data and buffers and configure vertex attributes
    this->vertices = vertexData;
    this->triangles = triangles;
    this->vertexCount = vertexCount;
    this->triangleCount = triangle_size;

    glGenVertexArrays(1, &(this->VAO));
    glGenBuffers(1, &(this->VBO));
    glGenBuffers(1, &(this->EBO));

    //bind the Vertex Array Object first, then bind and set vertex buffers, and them configure vertex attributes
    glBindVertexArray(this->VAO);

    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_size, vertexData, GL_STATIC_DRAW);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * triangle_size, triangles, GL_STATIC_DRAW );

    // 1. then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //set the texture coord attribute pointer
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) );
    glEnableVertexAttribArray(1);
}

Mesh::~Mesh(){
    glDeleteVertexArrays(1, &(this->VAO));
    glDeleteBuffers(1, &(this->VBO));
    glDeleteBuffers(1, &(this->EBO));
}
