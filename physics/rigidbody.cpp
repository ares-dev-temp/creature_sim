#include "rigidbody.h"

#include <iostream>

Rigidbody::Rigidbody( RVec3 pos, Quat rot, RVec3 s, EMotionType motionType, ObjectLayer layer ){
    position = pos;
    rotation = rot;
    scale = s;
    BoxShapeSettings body_shape_settings(scale);
    body_shape_settings.mConvexRadius = 0.01;
    body_shape_settings.SetDensity(200.0);
    body_shape_settings.SetEmbedded();
    ShapeSettings::ShapeResult body_shape_result = body_shape_settings.Create();
    ShapeRefC body_shape = body_shape_result.Get();
    this->body_settings = BodyCreationSettings(body_shape, position, rotation, motionType, layer);
    body_settings.mMaxLinearVelocity = 10000.0;
    body_settings.mApplyGyroscopicForce = true;
    body_settings.mLinearDamping = 0.0;
    body_settings.mAngularDamping = 0.0;
    
    //body = body_interface.CreateBody(body_settings);
    //body->SetFriction(0.5);
    //body->SetRestitution(0.3f);
    //body_interface.AddBody(body->GetID(), EActivation::Activate);
}

/*void Rigidbody::set_active( BodyInterface &body_interface, EActivation activation ){
    body_interface.AddBody(body->GetID(), activation);
}*/
