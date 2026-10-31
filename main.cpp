#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
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
#include "rendering/skybox.h"

using json = nlohmann::json;

const unsigned int SCREEN_WIDTH = 854, SCREEN_HEIGHT = 480;

//time between current frame and last frame
float deltaTime = 0.0f;
//time of last frame
float lastFrame = 0.0f;

float lastX = 427.0f, lastY = 240.0f;

bool firstMouse = true;
float yaw = -90.0f, pitch = 0.0f;

int spawnCount = 200;
std::vector<Entity> entities;

json material_data;
//point light properties
glm::vec3 pointLightPositions[] = {
    glm::vec3(-8.0f, 5.0f, -8.0f)//,
    //glm::vec3(-12.0f, 5.0f, 12.0f),
    //glm::vec3( 12.0f, 5.0f, -12.0f),
    //glm::vec3( 12.0f, 5.0f, 12.0f)
};

Camera camera{ glm::vec3(0.0f, 10.0f, 20.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT };

void RenderScene( Shader *shader, Entity *entity, glm::mat4 lightSpaceMatrix );
void processInput( GLFWwindow *window );
void mouse_callback( GLFWwindow *window, double xpos, double ypos );

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

float rand_f( float min_x, float max_x ){
    float val = (rand() % 1000) / 1000.0f;
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

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
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

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_MULTISAMPLE );
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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
    //Texture textureA{"imgs/container.jpg", GL_RGB};
    //Texture textureB{"imgs/awesomeface.png", GL_RGBA, 1};

    myShader.use();
    myShader.set_vector3( "color", glm::vec3(0.0f, 0.0f, 1.0f) );
    myShader.set_vector3( "lightColor", glm::vec3(1.0f, 1.0f, 1.0f) );
    //myShader.set_vector3( "lightPos", lightPos );
    
    std::ifstream f("cube_data.json");
    json data = json::parse(f);

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

    std::ifstream fm("materials_data.json");
    material_data = json::parse(fm);

    materialShader.use();
    materialShader.get_material( material_data, pointLightPositions );
    materialShader.set_vector3( "directionalLight.direction", glm::vec3( -0.2f, -1.0f, -0.3f ) );
    materialShader.set_vector3( "spotLight.position", glm::vec3(0.0f, 10.0f, 0.0f) );
    materialShader.set_vector3( "spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f) );

    //Create entities
    //Entity ground_cube{&mesh, &myShader};
    Entity ground_cube{&mesh, &materialShader};
    Entity spotlight{&mesh, &lightShader};

    /*Entity pointlights[] = {
        Entity{&mesh, &lightShader},
        Entity{&mesh, &lightShader},
        Entity{&mesh, &lightShader},
        Entity{&mesh, &lightShader}
    };*/

    std::vector<Entity> pointLights;

    //Entity pointLights[ pointLightPositions->length() ];

    printf( "point light count %d\n", pointLightPositions->length() );

    for( int i = 0; i < pointLightPositions->length()/3; i++ ){
        Entity pointLight = Entity{&mesh, &lightShader};
        pointLight.set_position( pointLightPositions[i] );
        pointLights.push_back( pointLight );
    }
    
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

    //int spawnCount = 200;
    //std::vector<Entity> entities;

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

    //Create skybox
    std::string faces[6] = {
        "imgs/skybox/right.jpg",
        "imgs/skybox/left.jpg",
        "imgs/skybox/top.jpg",
        "imgs/skybox/bottom.jpg",
        "imgs/skybox/front.jpg",
        "imgs/skybox/back.jpg"
    };

    const char *skybox_vertex_shader_file_path = "shaders/cubemap_vertex_shader.glsl";
    const char *skybox_fragment_shader_file_path = "shaders/cubemap_fragment_shader.glsl";
    Shader skyboxShader{ skybox_vertex_shader_file_path, skybox_fragment_shader_file_path };
    Skybox skybox( &skyboxShader, faces, vertices, triangles );

    const int cCollisionSteps = 2;

    //frame buffer object
    unsigned int depthMapFBO;
    glGenFramebuffers( 1, &depthMapFBO );

    //const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    //create depth map texture
    unsigned int depthMap;
    glGenTextures( 1, &depthMap );
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    //attach depth texture as FBOs depth buffer
    glBindFramebuffer( GL_FRAMEBUFFER, depthMapFBO );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0 );
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow framebuffer is incomplete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //create depth map shaders
    const char *depth_vertex_shader_file_path = "shaders/depthShader.glsl";
    const char *empty_fragment_shader_file_path = "shaders/emptyFragmentShader.glsl";
    Shader depthShader{ depth_vertex_shader_file_path, empty_fragment_shader_file_path };
    

    //create depth map shaders
    const char *cubeDepth_vertex_shader_file_path = "shaders/cubeDepthShader.glsl";
    const char *cubeDepth_fragment_shader_file_path = "shaders/depthFragmentShader.glsl";
    const char *cubeDepth_geometry_shader_file_path = "shaders/depthGeometryShader.glsl";
    Shader cubeDepthShader{ cubeDepth_vertex_shader_file_path, cubeDepth_fragment_shader_file_path, cubeDepth_geometry_shader_file_path };

    //create cube map for point light shadows
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);

    glBindTexture( GL_TEXTURE_CUBE_MAP, depthCubemap );
    for( unsigned int i = 0; i < 6; i++ )
        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );

    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

    materialShader.use();
    //materialShader.set_texture("shadowMap", 0);
    materialShader.set_texture("shadowCubeMap", 1);

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

        camera.update_viewMatrix( );

        //process physics
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

        glClearColor(0.2f, 0.6f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float x = sin(glfwGetTime()) * 8.0f;

        //glm::vec3 pointLighPos = glm::vec3( -12.0f, 10.0f, 12.0f );
        glm::vec3 pointLighPos = pointLights[0].position;

        //glm::vec3 lightPos = glm::vec3(x, 10.0f, 0.0f);
        glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
        glm::vec3 lightTarget(0.0f);
        glm::vec3 lightPos = lightTarget - lightDirection * 15.0f;

        //render depth of scene to texture
        float near_plane = 1.0f, far_plane = 40.0f;
        glm::mat4 lightProjection = glm::ortho( -20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane );
        
        // note that if you use a perspective projection matrix you'll have to change the light position 
        // as the current light position isn't enough to reflect the whole scene
        //glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);

        glm::mat4 lightView = glm::lookAt( lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f) );
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        //render scene from lights point of view
        //depthShader.use();
        //depthShader.set_matrix( "lightSpaceMatrix", lightSpaceMatrix );
        
        //1. first render to depth map
        /*glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer( GL_FRAMEBUFFER, depthMapFBO );
        glClear(GL_DEPTH_BUFFER_BIT);
        //glCullFace(GL_FRONT);
        RenderScene(&depthShader, &ground_cube, lightSpaceMatrix);
        //glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);*/


        float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
        float near = 1.0f, far = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back( shadowProj * glm::lookAt( pointLighPos, pointLighPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) ) );
        shadowTransforms.push_back( shadowProj * glm::lookAt( pointLighPos, pointLighPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) ) );
        shadowTransforms.push_back( shadowProj * glm::lookAt( pointLighPos, pointLighPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) ) );
        shadowTransforms.push_back( shadowProj * glm::lookAt( pointLighPos, pointLighPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) ) );
        shadowTransforms.push_back( shadowProj * glm::lookAt( pointLighPos, pointLighPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f) ) );
        shadowTransforms.push_back( shadowProj * glm::lookAt( pointLighPos, pointLighPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f) ) );


        // render depth cube map for point light shadows
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        //configureshaderandmatrices();
        //renderscene();
        cubeDepthShader.use();
        cubeDepthShader.set_float( "far_plane", far );
        for (unsigned int i = 0; i < 6; ++i){
            std::string shadowMatrices = "shadowMatrices[" + std::to_string( i ) + "]";
            cubeDepthShader.set_matrix(shadowMatrices.c_str(), shadowTransforms[i]);
        }
        materialShader.set_float( "far_plane", far );
        cubeDepthShader.set_vector3( "lightPos", pointLighPos );
        RenderScene(&cubeDepthShader, &ground_cube, lightSpaceMatrix);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //reset viewport
        glViewport(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind textures on corresponding texture units
        /*glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureA.ID);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureB.ID);*/

        //glActiveTexture( GL_TEXTURE0 );
        //glBindTexture(GL_TEXTURE_2D, depthMap);

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        
        // ..:: Drawing code (in render loop) :: ..

        materialShader.use();
        materialShader.get_material( material_data, pointLightPositions );

        //camera.update_viewMatrix( );

        materialShader.set_float( "far_plane", far );

        //float rad = 0.5f * sin( glfwGetTime() + 1.0f ) + 0.5f;
        //float x = sin(glfwGetTime()) * 8.0f;
        //float z = cos(glfwGetTime() + 1.5f) * 8.0f * rad;
        //light.set_position( glm::vec3( x, 2.0f + (sin(glfwGetTime()) + 1.0f ) * 4.0f, 0.0f ) );

        //materialShader.get_material(material_data);

        //materialShader.set_vector3( "directionalLight.direction", glm::vec3( -0.2f, -1.0f, -0.3f ) );
        materialShader.set_vector3( "directionalLight.direction", lightDirection );
        materialShader.set_matrix( "lightSpaceMatrix", lightSpaceMatrix );

        materialShader.set_vector3( "viewPos", camera.position );
        materialShader.set_matrix( "view", camera.viewMatrix );
        materialShader.set_matrix( "projection", camera.perspectiveMatrix );

        materialShader.set_vector3( "spotLight.position", glm::vec3(x, 10.0f, 0.0f) );

        //render cubes
        for( int i = 0; i < spawnCount; i++ )
            entities[i].render();

        //myShader.use();
        //myShader.set_vector3( "light.position", light.position );
        //myShader.set_matrix( "view", camera.viewMatrix );
        //myShader.set_matrix( "projection", camera.perspectiveMatrix );

        //render ground
        ground_cube.render();

        lightShader.use();
        lightShader.set_matrix( "view", camera.viewMatrix );
        lightShader.set_matrix( "projection", camera.perspectiveMatrix );

        spotlight.set_position( glm::vec3( x, 10.0f, 0.0f ) );
        spotlight.render();

        //printf( "player position: (%f, %f, %f)\n", camera.position.x, camera.position.y, camera.position.z );

        for( int i = 0; i < pointLights.size(); i++ )
            pointLights[i].render();

        glDisable(GL_CULL_FACE);
        skybox.draw( camera.viewMatrix, camera.perspectiveMatrix );
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

void RenderScene( Shader *shader, Entity *entity, glm::mat4 lightSpaceMatrix ){
    //shader->use();
    //shader->set_matrix( "lightSpaceMatrix", lightSpaceMatrix );
    
    //render cubes
    for( int i = 0; i < spawnCount; i++ ){
        shader->set_matrix( "model", entities[i].modelMatrix );
        //entities[i].render();
        entities[i].mesh->draw();
    }

    //render ground
    shader->set_matrix( "model", entity->modelMatrix );
    //entity->render();
    entity->mesh->draw();
}

void processInput( GLFWwindow *window ){
    if( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        glfwSetWindowShouldClose(window, true);
    
    float cameraSpeed = 15.0f * deltaTime;
    
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
