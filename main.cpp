#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

//my classes
#include "libs/stb_image.h"
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "rendering/mesh.h"
#include "rendering/camera.h"
#include "entities/entity.h"
#include "physics/physics_manager.h"
#include "physics/rigidbody.h"

using json = nlohmann::json;

int screenWidth = 854, screenHeight = 480;

//time between current frame and last frame
float deltaTime = 0.0f;
//time of last frame
float lastFrame = 0.0f;

float lastX = 427.0f, lastY = 240.0f;

bool firstMouse = true;
float yaw = -90.0f, pitch = 0.0f;

Camera camera{ glm::vec3(0.0f, 10.0f, 20.0f), (float)screenWidth/(float)screenHeight };

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

unsigned int loadCubemap( std::vector<std::string> faces ){
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for( unsigned int i = 0; i < faces.size(); i++ ){
        unsigned char *data = stbi_load( faces[i].c_str(), &width, &height, &nrChannels, 0 );
        if(data){
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
            0, GL_RGB, GL_UNSIGNED_BYTE, data );
            stbi_image_free( data );
        }
        else{
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
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

    glfwWindowHint( GLFW_SAMPLES, 4 );

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

    //Create Shaders
    const char *vertex_shader_file_path = "shaders/vertex_shader.glsl";
    const char *fragment_shader_file_path = "shaders/fragment_shader.glsl";

    Shader myShader{ vertex_shader_file_path, fragment_shader_file_path };
    printf( "compiled shaders\n" );

    const char *vertex_light_shader_file_path = "shaders/light_vertex_shader.glsl";
    const char *fragment_light_shader_file_path = "shaders/light_fragment_shader.glsl";
    Shader lightShader{ vertex_light_shader_file_path, fragment_light_shader_file_path };

    const char *fragment_material_shader_file_path = "shaders/material_shader.glsl";
    Shader materialShader{ vertex_shader_file_path, fragment_material_shader_file_path };

    //Load texture
    Texture textureA{"imgs/container.jpg", GL_RGB};
    Texture textureB{"imgs/awesomeface.png", GL_RGBA, 1};

    //glm::vec3 lightPos = glm::vec3( 0.0f, 10.0f, 0.0f );

    myShader.use();
    myShader.set_vector3( "color", glm::vec3(0.0f, 0.0f, 1.0f) );
    myShader.set_vector3( "lightColor", glm::vec3(1.0f, 1.0f, 1.0f) );
    //myShader.set_vector3( "lightPos", lightPos );
    
    std::ifstream f("cube_data.json");
    json data = json::parse(f);

    //std::cout << typeid(data["vertex_data_2"]).name() << std::endl;
    //std::cout << typeid(data["indices"]).name() << std::endl;

    std::vector<float> vertex_data = data["vertex_data_2"].get<std::vector<float>>();
    std::vector<unsigned int> indice_data = data["indices"].get<std::vector<unsigned int>>();
    
    float *vertices = vertex_data.data();
    unsigned int *triangles = indice_data.data();

    Mesh mesh{ 8, vertices, triangles, (int)vertex_data.size(), (int)indice_data.size() };

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));

    lightShader.use();
    //lightShader.set_vector3( "objectColor", glm::vec3(1.0f, 0.5f, 0.31f) );
    lightShader.set_vector3( "lightColor", glm::vec3(1.0f, 1.0f, 1.0f) );


    Mesh lightMesh{ 8, vertices, triangles, (int)vertex_data.size(), (int)indice_data.size() };

    materialShader.use();
    materialShader.set_vector3( "color", glm::vec3(0.0f, 0.0f, 1.0f) );
    materialShader.set_vector3( "lightColor", glm::vec3(1.0f, 1.0f, 1.0f) );
    materialShader.set_vector3( "viewPos", camera.position );

    materialShader.set_vector3( "material.ambient", glm::vec3(1.0f, 0.5f, 0.31f) * 10.0f );
    materialShader.set_vector3( "material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f) );
    materialShader.set_vector3( "material.specular", glm::vec3(0.5f, 0.5f, 0.5f) );
    //materialShader.set_vector3( "material.ambient", glm::vec3(1.0f, 1.0f, 1.0f) );
    //materialShader.set_vector3( "material.diffuse", glm::vec3(1.0f, 1.0f, 1.0f) );
    //materialShader.set_vector3( "material.specular", glm::vec3(1.0f, 1.0f, 1.0f) );
    materialShader.set_float( "material.shininess", 4.0f );

    //directional light properties
    materialShader.set_vector3( "directionalLight.direction", glm::vec3( -0.2f, -1.0f, -0.3f ) );
    materialShader.set_vector3( "directionalLight.ambient", glm::vec3( 0.05f, 0.05f, 0.05f ) );
    materialShader.set_vector3( "directionalLight.diffuse", glm::vec3( 0.4f, 0.4f, 0.4f ) );
    materialShader.set_vector3( "directionalLight.specular", glm::vec3( 0.5f, 0.5f, 0.5f ) );

    //point light properties
    glm::vec3 pointLightPositions[] = {
        glm::vec3(-12.0f, 5.0f, -12.0f),
        glm::vec3(-12.0f, 5.0f, 12.0f),
        glm::vec3( 12.0f, 5.0f, -12.0f),
        glm::vec3( 12.0f, 5.0f, 12.0f)
    };

    for( int i = 0; i < 4; i++ ){
        std::string pointLight = "pointLights[" + std::to_string( i ) + "]";
        
        materialShader.set_vector3( (pointLight + ".position").c_str(), pointLightPositions[i] );
        materialShader.set_vector3( (pointLight + ".ambient").c_str(), glm::vec3( 0.05f, 0.05f, 0.05f ) );
        materialShader.set_vector3( (pointLight + ".diffuse").c_str(), glm::vec3( 0.8f, 0.8f, 0.8f ) );
        materialShader.set_vector3( (pointLight + ".specular").c_str(), glm::vec3( 1.0f, 1.0f, 1.0f ) );

        materialShader.set_float( (pointLight + ".contant").c_str(), 1.0f );
        materialShader.set_float( (pointLight + ".linear").c_str(), 0.09f );
        materialShader.set_float( (pointLight + ".quadratic").c_str(), 0.032f );
    }

    //spot light properties
    materialShader.set_vector3( "spotLight.position", glm::vec3(0.0f, 10.0f, 0.0f) );
    materialShader.set_vector3( "spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f) ); //camera.forward );
    materialShader.set_vector3( "spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f) );
    materialShader.set_vector3( "spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f) );
    materialShader.set_vector3( "spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f) );
    materialShader.set_float( "spotLight.constant", 1.0f );
    materialShader.set_float( "spotLight.linear", 0.045f );
    materialShader.set_float( "spotLight.quadratic", 0.0075f );
    materialShader.set_float( "spotLight.cutOff", glm::cos( glm::radians( 10.0f ) ) );
    materialShader.set_float( "spotLight.outerCutOff", glm::cos( glm::radians( 15.0f ) ) );

    //glm::vec3 positions[1];
    //for( int i = 0; i < 1; i++ )
    //    positions[i] = glm::vec3( rand_f(-8.0f, 8.0f), rand_f(-4.0f, 4.0f), rand_f(-8.0f, 8.0f) );
    //positions[0] = glm::vec3( 0.0f, -1.0f, 0.0f );

    //Create entities
    //Entity ground_cube{&mesh, &myShader};
    Entity ground_cube{&mesh, &materialShader};
    Entity spotlight{&mesh, &lightShader};

    Entity pointlights[] = {
        Entity{&mesh, &lightShader},
        Entity{&mesh, &lightShader},
        Entity{&mesh, &lightShader},
        Entity{&mesh, &lightShader}
    };

    for( int i = 0; i < 4; i++ )
        pointlights[i].set_position( pointLightPositions[i] );
    
    //cube.position = glm::vec3( rand_f(-8.0f, 8.0f), rand_f(-4.0f, 4.0f), rand_f(-8.0f, 8.0f) );
    ground_cube.set_position( glm::vec3( 0.0f, -1.0f, 0.0f ) );
    ground_cube.set_scale( glm::vec3(50.0f, 0.5f, 50.0f) );

    spotlight.set_position( glm::vec3( 0.0f, 10.0f, 0.0f ) );

    myShader.set_vector3( "lightPos", spotlight.position );

    //materialShader.set_vector3( "light.position", light.position );

    PhysicsManager physicsManager;
    std::cout << "Jolt Physics loaded!\n";

    TempAllocatorMalloc temp_allocator;
    JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

    physicsManager.setup( 1024, 8192, 8192 );

    int spawnCount = 500;
    std::vector<Entity> entities;

    for( int i = 0; i < spawnCount; i++ ){
        float pos_x = rand_f(-8.0f, 8.0f), pos_y = rand_f( 10.0f, 25.0f), pos_z = rand_f(-10.0f, 10.0f);
        //entities.push_back( Entity(&mesh, &myShader) );
        entities.push_back( Entity(&mesh, &materialShader) );
        entities[i].set_position( glm::vec3(pos_x, pos_y, pos_z) );

        //block physics settings
        Quat rotationX = Quat::sRotation(
            Vec3::sAxisX(),
            DegreesToRadians( rand_f(0.0f, 180.0f) )
        );

        Quat rotationY = Quat::sRotation(
            Vec3::sAxisY(),
            DegreesToRadians( rand_f(0.0f, 180.0f) )
        );

        Quat rotation = rotationX * rotationY;
        
        Rigidbody rigidbody( RVec3(pos_x, pos_y, pos_z), rotation, Vec3(1.0f, 1.0f, 1.0f) * 0.5f,  EMotionType::Dynamic, Layers::MOVING );
        rigidbody.body = physicsManager.create_body( &rigidbody.body_settings, EActivation::Activate );
        physicsManager.activate_body( rigidbody.body );
        physicsManager.add_body( rigidbody.body );
    }

    //ground box physic settings
    Rigidbody ground( RVec3(0.0, -1.0f, 0.0), Quat::sIdentity(), Vec3(50.0f, 0.5f, 50.0f) * 0.5f, EMotionType::Static, Layers::MOVING );
    ground.body = physicsManager.create_body( &ground.body_settings, EActivation::DontActivate );
    physicsManager.activate_body( ground.body );
    


    //create skybox
    std::vector<std::string> faces = std::vector<std::string> {
        "imgs/skybox/right.jpg",
        "imgs/skybox/left.jpg",
        "imgs/skybox/top.jpg",
        "imgs/skybox/bottom.jpg",
        "imgs/skybox/front.jpg",
        "imgs/skybox/back.jpg"
    };

    unsigned int cubemapTextureID = loadCubemap(faces);

    const char *skybox_vertex_shader_file_path = "shaders/cubemap_vertex_shader.glsl";
    const char *skybox_fragment_shader_file_path = "shaders/cubemap_fragment_shader.glsl";

    Shader skyboxShader{ skybox_vertex_shader_file_path, skybox_fragment_shader_file_path };

    unsigned int skyboxVBO, skyboxVAO, skyboxEBO;
    
    std::vector<float> cube_vertex;
    for( int i = 0; i < 6 * 4 * 3; i++ ){
        int idx = (i % 3) + (i / 3) * 8;
        cube_vertex.push_back( vertices[idx] * 2.0f );
    }

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);

    //bind the Vertex Array Object first, then bind and set vertex buffers, and them configure vertex attributes
    glBindVertexArray(skyboxVAO);

    // 0. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cube_vertex.size(), cube_vertex.data(), GL_STATIC_DRAW);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, skyboxEBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indice_data.size(), indice_data.data(), GL_STATIC_DRAW );

    // 1. then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    

    //draw skybox
    /*glDepthMask(GL_FALSE);
    skyboxShader.use();
    skyboxShader.set_matrix( "projection", camera.perspectiveMatrix );
    skyboxShader.set_matrix( "view", camera.viewMatrix );
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
    glDrawElements( GL_TRIANGLES, indice_data.size(), GL_UNSIGNED_INT, 0 );
    glDepthMask(GL_TRUE);*/

    skyboxShader.use();
    skyboxShader.set_texture("skybox", 0);

    const int cCollisionSteps = 1;

    glEnable( GL_MULTISAMPLE );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //glDepthMask( GL_FALSE );

    int fps = 0;
    float timePassed = 0.0f;

    float fixedDeltaTime = 1.0f / 60.0f;
    float accumalator = 0.0f;

    int physicsUpdates = 0;

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //process physics
        physicsManager.Update(deltaTime, cCollisionSteps, &temp_allocator, &job_system);

        //process inputs
        processInput(window);

        glClearColor(0.2f, 0.6f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        /*glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureA.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureB.ID);*/
        
        // ..:: Drawing code (in render loop) :: ..
        /*glDisable(GL_CULL_FACE);

        //draw skybox
        glDepthMask(GL_FALSE);
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(camera.viewMatrix));
        skyboxShader.set_matrix( "view", view );
        skyboxShader.set_matrix( "projection", camera.perspectiveMatrix );
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
        glDrawElements( GL_TRIANGLES, indice_data.size(), GL_UNSIGNED_INT, 0 );
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);

        glEnable(GL_CULL_FACE);*/


        materialShader.use();

        camera.update_viewMatrix( );

        //float rad = 0.5f * sin( glfwGetTime() + 1.0f ) + 0.5f;
        float x = sin(glfwGetTime()) * 8.0f;
        //float z = cos(glfwGetTime() + 1.5f) * 8.0f * rad;
        //light.set_position( glm::vec3( x, 2.0f + (sin(glfwGetTime()) + 1.0f ) * 4.0f, 0.0f ) );

        materialShader.set_vector3( "viewPos", camera.position );
        materialShader.set_matrix( "view", camera.viewMatrix );
        materialShader.set_matrix( "projection", camera.perspectiveMatrix );

        materialShader.set_vector3( "spotLight.position", glm::vec3(x, 10.0f, 0.0f) );


        /*materialShader.set_vector3( "color", glm::vec3(0.0f, 0.0f, 1.0f) );
        materialShader.set_vector3( "lightColor", glm::vec3(1.0f, 1.0f, 1.0f) );
        materialShader.set_vector3( "viewPos", camera.position );
        materialShader.set_vector3( "material.ambient", glm::vec3(1.0f, 0.5f, 0.31f) );
        materialShader.set_vector3( "material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f) );
        materialShader.set_vector3( "material.specular", glm::vec3(0.5f, 0.5f, 0.5f) );
        materialShader.set_float( "material.shininess", 32.0f );

        //directional light properties
        materialShader.set_vector3( "directionalLight.direction", glm::vec3( -0.2f, -1.0f, -0.3f ) );
        materialShader.set_vector3( "directionalLight.ambient", glm::vec3( 0.05f, 0.05f, 0.05f ) );
        materialShader.set_vector3( "directionalLight.diffuse", glm::vec3( 0.4f, 0.4f, 0.4f ) );
        materialShader.set_vector3( "directionalLight.specular", glm::vec3( 0.5f, 0.5f, 0.5f ) );

        //point light properties
        for( int i = 0; i < 4; i++ ){
            std::string pointLight = "pointLights[" + std::to_string( i ) + "]";
            
            materialShader.set_vector3( (pointLight + ".position").c_str(), pointLightPositions[i] );
            materialShader.set_vector3( (pointLight + ".ambient").c_str(), glm::vec3( 0.05f, 0.05f, 0.05f ) );
            materialShader.set_vector3( (pointLight + ".diffuse").c_str(), glm::vec3( 0.8f, 0.8f, 0.8f ) );
            materialShader.set_vector3( (pointLight + ".specular").c_str(), glm::vec3( 1.0f, 1.0f, 1.0f ) );

            materialShader.set_float( (pointLight + ".contant").c_str(), 1.0f );
            materialShader.set_float( (pointLight + ".linear").c_str(), 0.09f );
            materialShader.set_float( (pointLight + ".quadratic").c_str(), 0.032f );
        }

        //spot light properties
        materialShader.set_vector3( "spotLight.position", glm::vec3(0.0f, 10.0f, 0.0f) );
        materialShader.set_vector3( "spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f) ); //camera.forward );
        materialShader.set_vector3( "spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f) );
        materialShader.set_vector3( "spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f) );
        materialShader.set_vector3( "spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f) );
        materialShader.set_float( "spotLight.constant", 1.0f );
        materialShader.set_float( "spotLight.linear", 0.045f );
        materialShader.set_float( "spotLight.quadratic", 0.0075f );
        materialShader.set_float( "spotLight.cutOff", glm::cos( glm::radians( 10.0f ) ) );
        materialShader.set_float( "spotLight.outerCutOff", glm::cos( glm::radians( 15.0f ) ) );*/

        //materialShader.set_vector3( "spotLight.position", camera.position );
        //materialShader.set_vector3( "spotLight.direction", camera.forward );

        accumalator += deltaTime;

        //physics loop
        while( accumalator > fixedDeltaTime ){
            Body* body;
            RMat44 transform;

            //cube physics calculations
            for( int i = 0; i < spawnCount; i++ ){
                body = physicsManager.get_body(i);
                physicsManager.activate_body( body );
                transform = physicsManager.get_body_transform( i );
                
                //Commenting the lines below doesnt throw an error
                RVec3 position = transform.GetTranslation();
                Vec3 x_coord = transform.GetAxisX();
                Vec3 y_coord = transform.GetAxisY();
                Vec3 z_coord = transform.GetAxisZ();
                entities[i].set_position( glm::vec3( position.GetX(), position.GetY(), position.GetZ() ) );
                float rotation[9] = {x_coord.GetX(), y_coord.GetX(), z_coord.GetX(), x_coord.GetY(), y_coord.GetY(), z_coord.GetY(), x_coord.GetZ(), y_coord.GetZ(), z_coord.GetZ()};
                entities[i].set_rotation( rotation );

                //cube rendering
                //float angle = glm::radians(90.0f) * deltaTime;
                //cube.rotate( angle, glm::vec3(1.0f, 0.3f, 0.5f) );
            }

            physicsUpdates++;

            accumalator -= fixedDeltaTime;
        }

        //render cubes
        for( int i = 0; i < spawnCount; i++ )
            entities[i].render();

        //myShader.use();
        //myShader.set_vector3( "light.position", light.position );
        //myShader.set_matrix( "view", camera.viewMatrix );
        //myShader.set_matrix( "projection", camera.perspectiveMatrix );

        ground_cube.render();



        lightShader.use();
        lightShader.set_matrix( "view", camera.viewMatrix );
        lightShader.set_matrix( "projection", camera.perspectiveMatrix );

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);

        spotlight.set_position( glm::vec3( x, 10.0f, 0.0f ) );
        spotlight.render();

        //printf( "player position: (%f, %f, %f)\n", camera.position.x, camera.position.y, camera.position.z );

        for( int i = 0; i < 4; i++ )
            pointlights[i].render();

        glDisable(GL_CULL_FACE);

        //draw skybox
        glDepthFunc( GL_LEQUAL );
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(camera.viewMatrix));
        skyboxShader.set_matrix( "view", view );
        skyboxShader.set_matrix( "projection", camera.perspectiveMatrix );
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureID);
        glDrawElements( GL_TRIANGLES, indice_data.size(), GL_UNSIGNED_INT, 0 );
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);

        glfwSwapBuffers(window);
        glfwPollEvents();

        if( timePassed < 1.0f ){
            timePassed += deltaTime;
            fps++;
        }else{
            printf( "FPS: %d, Physic Updates: %d\n", fps, physicsUpdates );
            timePassed = 0.0f;
            fps = 0;
            physicsUpdates = 0;
        }
    }

    glfwTerminate();
    return 0;
}
