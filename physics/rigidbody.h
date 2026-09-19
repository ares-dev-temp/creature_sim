#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace JPH;

class Rigidbody{
    public:
        Body *body;
        BodyCreationSettings body_settings;
        RVec3 position;
        Quat rotation;
        RVec3 scale;

        Rigidbody(RVec3 position, Quat rotation, RVec3 scale, EMotionType motionType, ObjectLayer layer);

        //void set_active( BodyInterface &body_interface, EActivation activation );
};