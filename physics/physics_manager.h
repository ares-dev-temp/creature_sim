#pragma once

#include <vector>

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
//#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace JPH;

namespace Layers{
    static constexpr ObjectLayer MOVING = 0;
};

class ObjectLayerPairFilterImpl: public ObjectLayerPairFilter{
    public:
        virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override {
            return true;
        }
};

namespace BroadPhaseLayers{
    static constexpr BroadPhaseLayer MOVING(0);
};

class BPLayerInterfaceImpl final: public BroadPhaseLayerInterface{
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

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter{
    public:
        virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override {
            return true;
    }
};

class PhysicsManager{
    public:
        JPH::PhysicsSystem physics_system;
        std::vector<Body*> bodies;

        BPLayerInterfaceImpl broad_phase_layer_interface;
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;

        PhysicsManager( );
        ~PhysicsManager();

        void setup( uint cMaxBodies, uint cMaxBodyPairs, uint cMaxContactConstraints );

        void add_body(Body *body);

        Body* create_body(BodyCreationSettings *body_settings,  EActivation activation);

        void activate_body( Body *body );

        Body* get_body( int idx );

        RMat44 get_body_transform( int idx );
        
        void Update( float deltaTime, int cCollisionSteps, TempAllocatorMalloc *temp_allocator, JobSystemThreadPool *job_system );
};
