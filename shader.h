#pragma once

#include <string>

class Shader{
    public:
        unsigned int ID;
        std::string vertexSource;
        std::string fragmentSource;

        Shader(const char *vertexPath, const char *fragmentPath);
        ~Shader();
        
        std::string read_file( const char* file_name );

        void compile_shader();

        //activates the shader
        void use();
};
