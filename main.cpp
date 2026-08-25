#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

#include "libs/stb_image.h"
#include "shader.h"
#include "texture.h"
#include "mesh.h"

using json = nlohmann::json;

int screenWidth = 854, screenHeight = 480;

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Request core profile for modern OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on macOS

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 5. Load OpenGL functions with GLAD
    int version = gladLoadGL();
    if (version == 0){
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL version: "
              << glGetString(GL_VERSION)
              << '\n';

    //Create Shaders
    const char *vertex_shader_file_path = "shaders/vertex_shader.glsl";
    const char *fragment_shader_file_path = "shaders/fragment_shader.glsl";

    Shader myShader{ vertex_shader_file_path, fragment_shader_file_path };
    //myShader.compile_shader();
    printf( "compiled shaders\n" );

    //Load texture
    Texture textureA{"imgs/container.jpg", GL_RGB};
    Texture textureB{"imgs/awesomeface.png", GL_RGBA, 1};

    myShader.use();
    glUniform1i( glGetUniformLocation(myShader.ID, "tex_1"), 0 );
    glUniform1i( glGetUniformLocation(myShader.ID, "tex_2"), 1 );
    
    std::ifstream f("cube_data.json");
    json data = json::parse(f);

    //std::cout << typeid(data["vertex_data_2"]).name() << std::endl;
    //std::cout << typeid(data["indices"]).name() << std::endl;

    std::vector<float> vertex_data = data["vertex_data_2"].get<std::vector<float>>();
    std::vector<unsigned int> indice_data = data["indices"].get<std::vector<unsigned int>>();

    for( int i = 0; i < vertex_data.size(); i+=3 ){
        vertex_data[i+2] = vertex_data[i+2] + 0.0;
    }
    
    float *vertices = vertex_data.data();
    unsigned int *triangles = indice_data.data();

    Mesh mesh{ 8, vertices, triangles, (int)vertex_data.size(), (int)indice_data.size() };

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.6f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureA.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureB.ID);

        // ..:: Drawing code (in render loop) :: ..
        myShader.use();

        glBindVertexArray(mesh.VAO);
        glDrawElements( GL_TRIANGLES, mesh.triangleCount, GL_UNSIGNED_INT, 0 );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
