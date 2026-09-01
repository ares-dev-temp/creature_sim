#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

#include "libs/stb_image.h"
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "rendering/mesh.h"
#include "rendering/camera.h"

using json = nlohmann::json;

int screenWidth = 854, screenHeight = 480;

//time between current frame and last frame
float deltaTime = 0.0f;
//time of last frame
float lastFrame = 0.0f;

float lastX = 427.0f, lastY = 240.0f;

bool firstMouse = true;
float yaw = -90.0f, pitch = 0.0f;

Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f), (float)screenWidth/(float)screenHeight };

void processInput( GLFWwindow *window ){
    if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        glfwSetWindowShouldClose(window, true);
    
    float cameraSpeed = 5.0f * deltaTime;
    
    if( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
        camera.position += camera.forward * cameraSpeed;

    if( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
        camera.position += -camera.forward * cameraSpeed;

    if( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS ){
        glm::vec3 dir = glm::normalize( glm::cross( camera.forward, camera.up ) );
        camera.position += -dir * cameraSpeed;
    }

    if( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS ){
        glm::vec3 dir = glm::normalize( glm::cross( camera.forward, camera.up ) );
        camera.position += dir * cameraSpeed;
    }
}

void mouse_callback( GLFWwindow *window, double xpos, double ypos ){
    if( firstMouse ){
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if( pitch > 89.0f )
        pitch = 89.0f;
    if( pitch < -89.0f )
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos( glm::radians( yaw ) ) * cos( glm::radians( pitch ) );
    direction.y = sin( glm::radians( pitch ) );
    direction.z = sin( glm::radians( yaw ) ) * cos( glm::radians( pitch ) );
    camera.forward = glm::normalize( direction );
}

float rand_f( float min_x, float max_x ){
    float val = (rand() % 1000) / 1000.0f;
    //return (val - min_x) / (max_x - min_x);
    return val * (max_x - min_x) + min_x;
}

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
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwSetCursorPosCallback(window, mouse_callback);

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

    //Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f), (float)screenWidth/(float)screenHeight };

    //Create Shaders
    const char *vertex_shader_file_path = "shaders/vertex_shader.glsl";
    const char *fragment_shader_file_path = "shaders/fragment_shader.glsl";

    Shader myShader{ vertex_shader_file_path, fragment_shader_file_path };
    printf( "compiled shaders\n" );

    //Load texture
    Texture textureA{"imgs/container.jpg", GL_RGB};
    Texture textureB{"imgs/awesomeface.png", GL_RGBA, 1};

    myShader.use();
    //glUniform1i( glGetUniformLocation(myShader.ID, "tex_1"), 0 );
    //glUniform1i( glGetUniformLocation(myShader.ID, "tex_2"), 1 );
    //myShader.set_texture( "text_1", 0 );
    //myShader.set_texture( "tex_2", 1 );
    myShader.set_vector3( "color", glm::vec3(0.0f, 0.0f, 1.0f) );
    myShader.set_vector3( "lightColor", glm::vec3(1.0f, 1.0f, 1.0f) );
    myShader.set_vector3( "lightPos", glm::vec3(0.0f, 5.0f, 0.0f) );
    
    std::ifstream f("cube_data.json");
    json data = json::parse(f);

    //std::cout << typeid(data["vertex_data_2"]).name() << std::endl;
    //std::cout << typeid(data["indices"]).name() << std::endl;

    std::vector<float> vertex_data = data["vertex_data_2"].get<std::vector<float>>();
    std::vector<unsigned int> indice_data = data["indices"].get<std::vector<unsigned int>>();

    //for( int i = 0; i < vertex_data.size(); i+=3 ){
    //    vertex_data[i+2] = vertex_data[i+2] + 0.0;
    //}
    
    float *vertices = vertex_data.data();
    unsigned int *triangles = indice_data.data();

    Mesh mesh{ 8, vertices, triangles, (int)vertex_data.size(), (int)indice_data.size() };

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));

    glEnable( GL_DEPTH_TEST );
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    int fps = 0;
    float timePassed = 0.0f;

    glm::vec3 positions[10];

    for( int i = 0; i < 10; i++ ){
        positions[i] = glm::vec3( rand_f(-6.0f, 6.0f), rand_f(-1.2f, 1.2f), rand_f(-8.0f, 0.0f) );
        //std::cout << positions[i].x << "\n" << std::endl;
        //std::cout << (rand() % 1000) / 1000.0f << std::endl;
        //std::cout << positions[i].z << std::endl;
    }

    //positions[0] = glm::vec3( 0.0f, 0.0f, 0.0f );

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //process inputs
        processInput(window);

        glClearColor(0.2f, 0.6f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureA.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureB.ID);
        
        // ..:: Drawing code (in render loop) :: ..
        myShader.use();

        camera.update_viewMatrix( );

        int viewLoc = glGetUniformLocation( myShader.ID, "view" );
        glUniformMatrix4fv( viewLoc, 1, GL_FALSE, glm::value_ptr(camera.viewMatrix) );

        int projLoc = glGetUniformLocation( myShader.ID, "projection" );
        glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( camera.perspectiveMatrix ) );

        for( int i = 0; i < 10; i++ ){
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, positions[i]);
            float angle = glm::radians(10.0f) * (i+1.0) * glfwGetTime();
            modelMatrix = glm::rotate( modelMatrix, angle, glm::vec3(1.0f, 0.3f, 0.5f) );

            int modelLoc = glGetUniformLocation( myShader.ID, "model" );
            glUniformMatrix4fv( modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix) );

            glBindVertexArray(mesh.VAO);
            glDrawElements( GL_TRIANGLES, mesh.triangleCount, GL_UNSIGNED_INT, 0 );
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        if( timePassed < 1.0f ){
            timePassed += deltaTime;
            fps++;
        }else{
            printf( "FPS: %d\n", fps );
            timePassed = 0.0f;
            fps = 0;
        }
    }

    glfwTerminate();
    return 0;
}
