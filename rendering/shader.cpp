#include "shader.h"
#include <stdio.h>

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader( const char *vertexPath, const char *fragmentPath ){
    vertexSource = read_file( vertexPath );
    fragmentSource = read_file( fragmentPath );
    compile_shader();
}

Shader::~Shader(){
    glDeleteProgram( ID );
}

std::string Shader::read_file( const char* file_name ){
    // 1. retrieve the vertex/fragment source code from filePath
    std::string shaderCode;
    std::ifstream shaderFile;
    // ensure ifstream objects can throw exceptions:
    shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open files
        shaderFile.open(file_name);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << shaderFile.rdbuf();
        // close file handlers
        shaderFile.close();
        // convert stream into string
        shaderCode = vShaderStream.str();
    }
    catch(std::ifstream::failure e){
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    return shaderCode;
}

void Shader::compile_shader(){
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    int success;
    char infoLog[512];

    //create and compile vertex shader
    const char *vertexSourcePtr = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &vertexSourcePtr, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf( "ERROR::SHADER::VERTEX::COMPILATION_FAILED: %s\n", infoLog );
    }

    //create and compile fragment shader
    const char *fragmentSourcePtr = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentSourcePtr, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf( "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED: %s\n", infoLog );
    }

    //create the Shader Program and link vertex and fragment shader
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        printf( "ERROR::SHADER::PROGRAM::LINKING_FAILED: %s\n", infoLog );
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::set_texture( const char* name, int id ){
    glUniform1i( glGetUniformLocation(ID, name), id );
}

void Shader::set_vector3( const char* name, glm::vec3 vector ){
    glUniform3fv( glGetUniformLocation( ID, name ), 1, &vector[0] );
}

void Shader::set_matrix( const char* name, glm::mat4 matrix ){
    int loc = glGetUniformLocation( ID, name );
    glUniformMatrix4fv( loc, 1, GL_FALSE, glm::value_ptr(matrix) );
}

void Shader::use(){
    glUseProgram(ID);
}
