#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader{
    public:
        unsigned int ID;
        std::string vertexSource;
        std::string fragmentSource;

        Shader(const char *vertexPath, const char *fragmentPath);
        ~Shader();
        
        std::string read_file( const char* file_name );

        void compile_shader();

        void set_texture( const char* textureName, int tex_id );

        void set_vector3( const char* vectorName, glm::vec3 vector );

        //activates the shader
        void use();
};
