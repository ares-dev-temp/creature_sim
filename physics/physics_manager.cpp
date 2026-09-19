#include "physics_manager.h"

#include <iostream>
#include <fstream>
#include <string>

PhysicsManager::PhysicsManager(  ){
    JPH::RegisterDefaultAllocator();
    // 2. Create the Jolt factory
    JPH::Factory::sInstance = new JPH::Factory();
    // 3. Register all Jolt physics types
    JPH::RegisterTypes();
}

PhysicsManager::~PhysicsManager(){
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsManager::setup(const uint cMaxBodies, const uint cMaxBodyPairs, const uint cMaxContactConstraints){
    //TempAllocatorMalloc temp_allocator;
    //JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
    
    const uint cNumBodyMutexes = 0;

    physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface,
                      object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
    physics_system.SetGravity(Vec3(0, -9.8f, 0));
}

/*BodyInterface &PhysicsManager::GetBodyInterface(){
    //JPH::PhysicsSystem ph = *physics_system;
    //return body_interface;
    //std::cout << physics_system << "\n";
    //std::cout << (std::string)(physics_system->GetBodyInterface()) << "\n";
    return this->physics_system.GetBodyInterface();
}*/

void PhysicsManager::Update( float deltaTime, int cCollisionSteps, TempAllocatorMalloc *temp_allocator, JobSystemThreadPool *job_system ){
    physics_system.Update(deltaTime, cCollisionSteps, temp_allocator, job_system);
}

void PhysicsManager::add_body( Body *body ){
    bodies.push_back( body );
}

Body* PhysicsManager::create_body( BodyCreationSettings *body_settings, EActivation activation ){
    BodyInterface &body_interface2 = physics_system.GetBodyInterface();
    Body *body = body_interface2.CreateBody(*body_settings);
    body->SetFriction(0.5);
    body->SetRestitution(0.3f);
    body_interface2.AddBody(body->GetID(), activation);
    return body;
}

void PhysicsManager::activate_body( Body *body ){
    BodyInterface &body_interface = physics_system.GetBodyInterface();
    body_interface.ActivateBody(body->GetID());
}

Body *PhysicsManager::get_body( int idx ){
    return bodies[idx];
}

RMat44 PhysicsManager::get_body_transform( int idx ){
    BodyInterface &body_interface = physics_system.GetBodyInterface();
    return body_interface.GetWorldTransform(bodies[idx]->GetID());
}
