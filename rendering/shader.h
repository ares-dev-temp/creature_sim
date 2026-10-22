#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>

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

        void set_matrix( const char* matrixName, glm::mat4 matrix );

        void set_float( const char* floatName, float val );

        void get_material( nlohmann::json data, glm::vec3 pointLightPositions[] );

        //activates the shader
        void use();
};
