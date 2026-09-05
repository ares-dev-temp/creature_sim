#pragma once

class Mesh{
    public:
        Mesh( int vertexCount, float *vertexData, unsigned int *triangles, int vertex_size, int triangle_size );
        ~Mesh();

        void draw();

        unsigned int VBO, VAO, EBO;
        int vertexCount;
        int triangleCount;
        float *vertices;
        unsigned int *triangles;
};