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

/*#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>*/
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace JPH;

namespace Layers
{
  static constexpr ObjectLayer MOVING = 0;
};

class ObjectLayerPairFilterImpl: public ObjectLayerPairFilter
{
  public:
    virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override {
      return true;
    }
};

namespace BroadPhaseLayers
{
  static constexpr BroadPhaseLayer MOVING(0);
};

class BPLayerInterfaceImpl final: public BroadPhaseLayerInterface
{
  public:
    virtual uint GetNumBroadPhaseLayers() const override {
      return 1;
    }

    virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override {
      return BroadPhaseLayers::MOVING;
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char *GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override {
      return "MOVING";
    }
#endif
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
  virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override {
    return true;
  }
};

// STL
#include <iostream>

#include "libs/stb_image.h"
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "rendering/mesh.h"
#include "rendering/camera.h"
#include "entities/entity.h"

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

    JPH::RegisterDefaultAllocator();

    // 2. Create the Jolt factory
    JPH::Factory::sInstance = new JPH::Factory();
    // 3. Register all Jolt physics types
    JPH::RegisterTypes();

    //Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f), (float)screenWidth/(float)screenHeight };

    //Create Shaders
    const char *vertex_shader_file_path = "shaders/vertex_shader.glsl";
    const char *fragment_shader_file_path = "shaders/fragment_shader.glsl";

    Shader myShader{ vertex_shader_file_path, fragment_shader_file_path };
    printf( "compiled shaders\n" );

    const char *vertex_light_shader_file_path = "shaders/light_vertex_shader.glsl";
    const char *fragment_light_shader_file_path = "shaders/light_fragment_shader.glsl";
    Shader lightShader{ vertex_light_shader_file_path, fragment_light_shader_file_path };

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

    //glm::vec3 positions[1];
    //for( int i = 0; i < 1; i++ )
    //    positions[i] = glm::vec3( rand_f(-8.0f, 8.0f), rand_f(-4.0f, 4.0f), rand_f(-8.0f, 8.0f) );
    //positions[0] = glm::vec3( 0.0f, -1.0f, 0.0f );

    //Create entities
    Entity ground_cube{&mesh, &myShader};
    Entity light{&mesh, &lightShader};
    
    //cube.position = glm::vec3( rand_f(-8.0f, 8.0f), rand_f(-4.0f, 4.0f), rand_f(-8.0f, 8.0f) );
    ground_cube.set_position( glm::vec3( 0.0f, -1.0f, 0.0f ) );
    ground_cube.set_scale( glm::vec3(50.0f, 0.5f, 50.0f) );

    light.set_position( glm::vec3( 0.0f, 10.0f, 0.0f ) );

    myShader.set_vector3( "lightPos", light.position );


    TempAllocatorMalloc temp_allocator;
    JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

    // 4. Create the physics system
    JPH::PhysicsSystem physics_system;
    std::cout << "Jolt Physics loaded!\n";

    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 8192;//1024;
    const uint cMaxContactConstraints = 8192;//1024;
    BPLayerInterfaceImpl broad_phase_layer_interface;
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    
    //float a = 1.0;
    //float b = 0.1;
    //float c = 0.5;

    physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface,
                      object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
    physics_system.SetGravity(Vec3(0, -9.8f, 0));

    BodyInterface &body_interface = physics_system.GetBodyInterface();

    int spawnCount = 1000;
    std::vector<Body*> bodies;
    std::vector<Entity> entities;

    //float cube_x = 0.0f, cube_y = 2.0f, cube_z = -5.0f;
    //Entity cube{&mesh, &myShader};
    //cube.set_position( glm::vec3(cube_x, cube_y, cube_z) );

    for( int i = 0; i < spawnCount; i++ ){
        float pos_x = rand_f(-8.0f, 8.0f);
        float pos_y = rand_f( 5.0f, 20.0f);
        float pos_z = rand_f(-10.0f, 10.0f);

        entities.push_back( Entity(&mesh, &myShader) );
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

        BoxShapeSettings body_shape_settings(Vec3(1.0f, 1.0f, 1.0f) * 0.5f);
        body_shape_settings.mConvexRadius = 0.01;
        body_shape_settings.SetDensity(200.0);
        body_shape_settings.SetEmbedded();
        ShapeSettings::ShapeResult body_shape_result = body_shape_settings.Create();
        ShapeRefC body_shape = body_shape_result.Get();
        BodyCreationSettings body_settings(body_shape, RVec3(pos_x, pos_y, pos_z), rotation, EMotionType::Dynamic, Layers::MOVING);
        body_settings.mMaxLinearVelocity = 10000.0;
        body_settings.mApplyGyroscopicForce = true;
        body_settings.mLinearDamping = 0.0;
        body_settings.mAngularDamping = 0.0;
        Body *body = body_interface.CreateBody(body_settings);
        body->SetFriction(0.5);
        body->SetRestitution(0.3f);
        body_interface.AddBody(body->GetID(), EActivation::Activate);

        body_interface.ActivateBody(body->GetID());

        bodies.push_back( body );
    }

    //ground box physic settings
    BoxShapeSettings ground_shape_settings(Vec3(50.0f, 0.5f, 50.0f) * 0.5f);
    ground_shape_settings.mConvexRadius = 0.01;
    ground_shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult ground_shape_result = ground_shape_settings.Create();
    ShapeRefC ground_shape = ground_shape_result.Get();
    BodyCreationSettings ground_settings(ground_shape, RVec3(0.0, -1.0f, 0.0), Quat::sIdentity(), EMotionType::Static, Layers::MOVING);
    Body *ground = body_interface.CreateBody(ground_settings);
    ground->SetFriction(0.5);
    body_interface.AddBody(ground->GetID(), EActivation::DontActivate);

    body_interface.ActivateBody(ground->GetID());

    const int cCollisionSteps = 1;

    glEnable( GL_DEPTH_TEST );
    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    int fps = 0;
    float timePassed = 0.0f;

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //process physics
        physics_system.Update(deltaTime, cCollisionSteps, &temp_allocator, &job_system);

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

        //float rad = 0.5f * sin( glfwGetTime() + 1.0f ) + 0.5f;
        float x = cos(glfwGetTime()) * 8.0f;
        //float z = cos(glfwGetTime() + 1.5f) * 8.0f * rad;
        light.set_position( glm::vec3( x, 2.0f + (sin(glfwGetTime()) + 1.0f ) * 4.0f, 0.0f ) );

        myShader.set_vector3( "lightPos", light.position );
        myShader.set_matrix( "view", camera.viewMatrix );
        myShader.set_matrix( "projection", camera.perspectiveMatrix );

        //cube physics
        for( int i = 0; i < spawnCount; i++ ){
            body_interface.ActivateBody(bodies[i]->GetID());
            RMat44 transform = body_interface.GetWorldTransform(bodies[i]->GetID());
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
            entities[i].render();
        }
        
        ground_cube.render();

        lightShader.use();
        lightShader.set_matrix( "view", camera.viewMatrix );
        lightShader.set_matrix( "projection", camera.perspectiveMatrix );

        light.render();

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

    // 6. Shut down Jolt
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;

    glfwTerminate();
    return 0;
}
