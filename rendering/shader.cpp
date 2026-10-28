#include "shader.h"
#include <stdio.h>

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

using json = nlohmann::json;
//using namespace glm;

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

void Shader::set_float( const char* name, float val ){
    int loc = glGetUniformLocation( ID, name );
    glUniform1f( loc, val );
}

glm::vec3 convert( std::vector<float> vector ){
    //printf( "vector: (%f, %f, %f)\n", vector[0], vector[1], vector[2] );
    return glm::vec3( vector[0], vector[1], vector[2] );
}

void Shader::get_material( json data, glm::vec3 pointLightPositions[] ){
    set_vector3( "color", convert( data["cube_material"]["color"] ) );
    set_vector3( "lightColor", convert( data["cube_material"]["lightColor"] ) );
    //set_vector3( "viewPos", camera.position );

    set_vector3( "material.ambient", convert( data["cube_material"]["ambient"] ) );
    set_vector3( "material.diffuse", convert( data["cube_material"]["diffuse"] ) );
    set_vector3( "material.specular", convert( data["cube_material"]["specular"] ) );
    set_float( "material.shininess", data["cube_material"]["shininess"] );

    //directional light properties
    //set_vector3( "directionalLight.direction", glm::vec3( data["directionLight"][] ) );
    set_vector3( "directionalLight.ambient", convert( data["directionalLight"]["ambient"] ) );
    set_vector3( "directionalLight.diffuse", convert( data["directionalLight"]["diffuse"] ) );
    set_vector3( "directionalLight.specular", convert( data["directionalLight"]["specular"] ) );

    //point light properties
    for( int i = 0; i < 4; i++ ){
        std::string pointLight = "pointLights[" + std::to_string( i ) + "]";
        
        set_vector3( (pointLight + ".position").c_str(), pointLightPositions[i] );
        set_vector3( (pointLight + ".ambient").c_str(), convert( data["pointlight"]["ambient"] ) );
        set_vector3( (pointLight + ".diffuse").c_str(), convert( data["pointlight"]["diffuse"] ) );
        set_vector3( (pointLight + ".specular").c_str(), convert( data["pointlight"]["specular"] ) );

        set_float( (pointLight + ".contant").c_str(), data["pointlight"]["constant"] );
        set_float( (pointLight + ".linear").c_str(), data["pointlight"]["linear"] );
        set_float( (pointLight + ".quadratic").c_str(), data["pointlight"]["quadratic"] );
    }

    //spot light properties
    //set_vector3( "spotLight.position", glm::vec3( data["spotlight"][""] ) );
    //set_vector3( "spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f) ); //camera.forward );
    set_vector3( "spotLight.ambient", convert( data["spotlight"]["ambient"] ) );
    set_vector3( "spotLight.diffuse", convert( data["spotlight"]["diffuse"] ) );
    set_vector3( "spotLight.specular", convert( data["spotlight"]["specular"] ) );
    set_float( "spotLight.constant", data["spotlight"]["constant"] );
    set_float( "spotLight.linear", data["spotlight"]["linear"] );
    set_float( "spotLight.quadratic", data["spotlight"]["quadratic"] );
    set_float( "spotLight.cutOff", glm::cos( glm::radians( float(data["spotlight"]["cutOff"]) ) ) );
    set_float( "spotLight.outerCutOff", glm::cos( glm::radians( float( data["spotlight"]["outerCutOff"] ) ) ) );
}

void Shader::use(){
    glUseProgram(ID);
}
