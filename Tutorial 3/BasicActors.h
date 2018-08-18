#pragma once

#include "PhysicsEngine.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	///Sphere class
	class Sphere : public DynamicActor
	{
	public:
		//a sphere with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m
		// - denisty: 1kg/m^3
		Sphere(const PxTransform& pose=PxTransform(PxIdentity), PxReal radius=1.f, PxReal density=1.f) 
			: DynamicActor(pose)
		{ 
			CreateShape(PxSphereGeometry(radius), density);
		}
	};

	///Box class
	class Box : public DynamicActor
	{
	public:
		//a Box with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m x 1m x 1m
		// - denisty: 1kg/m^3
		Box(const PxTransform& pose=PxTransform(PxIdentity), PxVec3 dimensions=PxVec3(.5f,.5f,.5f), PxReal density=1.f) 
			: DynamicActor(pose)
		{ 
			CreateShape(PxBoxGeometry(dimensions), density);
		}
	};

	class Capsule : public DynamicActor
	{
	public:
		Capsule(const PxTransform& pose=PxTransform(PxIdentity), PxVec2 dimensions=PxVec2(1.f,1.f), PxReal density=1.f) 
			: DynamicActor(pose)
		{
			CreateShape(PxCapsuleGeometry(dimensions.x, dimensions.y), density);
		}
	};

	class Cloth : public Actor
	{
		PxClothMeshDesc mesh_desc;

	public:
		//constructor
		Cloth(PxTransform pose = PxTransform(PxIdentity), const PxVec2& size = PxVec2(1.f, 1.f), PxU32 width = 1, PxU32 height = 1, bool fix_top = true)
		{
			//prepare vertices
			PxReal w_step = size.x / width;
			PxReal h_step = size.y / height;

			PxClothParticle* vertices = new PxClothParticle[(width + 1)*(height + 1) * 4];
			PxU32* quads = new PxU32[width*height * 4];

			for (PxU32 j = 0; j < (height + 1); j++)
			{
				for (PxU32 i = 0; i < (width + 1); i++)
				{
					PxU32 offset = i + j*(width + 1);
					vertices[offset].pos = PxVec3(w_step*i, 0.f, h_step*j);
					if (fix_top && (j == 0)) //fix the top row of vertices
						vertices[offset].invWeight = 0.f;
					else
						vertices[offset].invWeight = 1.f;
				}

				for (PxU32 j = 0; j < height; j++)
				{
					for (PxU32 i = 0; i < width; i++)
					{
						PxU32 offset = (i + j*width) * 4;
						quads[offset + 0] = (i + 0) + (j + 0)*(width + 1);
						quads[offset + 1] = (i + 1) + (j + 0)*(width + 1);
						quads[offset + 2] = (i + 1) + (j + 1)*(width + 1);
						quads[offset + 3] = (i + 0) + (j + 1)*(width + 1);
					}
				}
			}

			//init cloth mesh description
			mesh_desc.points.data = vertices;
			mesh_desc.points.count = (width + 1)*(height + 1);
			mesh_desc.points.stride = sizeof(PxClothParticle);

			mesh_desc.invMasses.data = &vertices->invWeight;
			mesh_desc.invMasses.count = (width + 1)*(height + 1);
			mesh_desc.invMasses.stride = sizeof(PxClothParticle);

			mesh_desc.quads.data = quads;
			mesh_desc.quads.count = width*height;
			mesh_desc.quads.stride = sizeof(PxU32) * 4;

			//create cloth fabric (cooking)
			PxClothFabric* fabric = PxClothFabricCreate(*GetPhysics(), mesh_desc, PxVec3(0, -1, 0));

			//create cloth
			actor = (PxActor*)GetPhysics()->createCloth(pose, *fabric, vertices, PxClothFlags());
			//collisions with the scene objects

			colors.push_back(default_color);
			actor->userData = new UserData(&colors.back(), &mesh_desc);
		}

		~Cloth()
		{
			delete (UserData*)actor->userData;
		}
	};

	class Club : public DynamicActor
	{
		Club();
		//~CompoundObject();
	public:
		Club(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(1.f, 0.35f, 0.3f), 1.f);
			CreateShape(PxBoxGeometry(0.175f, 4.f, 0.175f), 1.f);
			CreateShape(PxBoxGeometry(0.225f, 1.f, 0.225f), 1.f);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0.6f, 0.3f, 0.f)));
			GetShape(1)->setLocalPose(PxTransform(PxVec3(0.f, 4.6f, 0.f)));
			GetShape(2)->setLocalPose(PxTransform(PxVec3(0.f, 9.6f, 0.f)));
		}
	};

	class Course : public StaticActor
	{
		Course();

	public:
		Course(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			// Stretch
			CreateShape(PxBoxGeometry(5.f, 0.1f, 10.f), 1.f);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 35.f)));

			// Hole left
			CreateShape(PxBoxGeometry(2.125f, 0.1f, 5.f), 1.f);
			GetShape(1)->setLocalPose(PxTransform(PxVec3(-2.875f, 0.f, 50.f)));
			// Hole right
			CreateShape(PxBoxGeometry(2.125f, 0.1f, 5.f), 1.f);
			GetShape(2)->setLocalPose(PxTransform(PxVec3(2.875f, 0.f, 50.f)));
			// Hole front
			CreateShape(PxBoxGeometry(0.75f, 0.1f, 2.125f), 1.f);
			GetShape(3)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 47.125f)));
			// Hole back
			CreateShape(PxBoxGeometry(0.75f, 0.1f, 2.125f), 1.f);
			GetShape(4)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 52.875f)));
		}
	};

	class CourseMiddle : public StaticActor
	{
		CourseMiddle();

	public:
		CourseMiddle(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			// Stretch
			CreateShape(PxBoxGeometry(5.f, 0.1f, 10.f), 1.f);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 15.f)));
		}
	};

	class Barriers : public StaticActor
	{
		Barriers();

	public:
		Barriers(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			// Back barrier
			CreateShape(PxBoxGeometry(5.f, 0.5f, 0.1f), 1.f);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0.f, 0.5f, 55.f)));

			// Left barrier
			CreateShape(PxBoxGeometry(0.1f, 0.5f, 25.f), 1.f);
			GetShape(1)->setLocalPose(PxTransform(PxVec3(5.f, 0.5f, 30.f)));

			// Right barrier
			CreateShape(PxBoxGeometry(0.1f, 0.5f, 25.f), 1.f);
			GetShape(2)->setLocalPose(PxTransform(PxVec3(-5.f, 0.5f, 30.f)));
		}
	};

	class Windmill : public StaticActor
	{
		Windmill();

	public:
		Windmill(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			// Top 1
			CreateShape(PxBoxGeometry(5.0f, 1.0f, 4.0f), 1.f);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0.0f, 2.f, 30.f)));

			// Top 2
			CreateShape(PxBoxGeometry(4.75f, 1.0f, 3.75f), 1.f);
			GetShape(1)->setLocalPose(PxTransform(PxVec3(0.0f, 4.f, 30.f)));

			// Top 3
			CreateShape(PxBoxGeometry(4.5f, 1.0f, 3.5f), 1.f);
			GetShape(2)->setLocalPose(PxTransform(PxVec3(0.0f, 6.f, 30.f)));

			// Top 4
			CreateShape(PxBoxGeometry(4.f, 2.0f, 3.f), 1.f);
			GetShape(3)->setLocalPose(PxTransform(PxVec3(0.0f, 9.f, 30.f)));

			// Left wall
			CreateShape(PxBoxGeometry(2.25f, 0.5f, 4.0f), 1.f);
			GetShape(4)->setLocalPose(PxTransform(PxVec3(2.75f, 0.5f, 30.0f)));

			// Right wall
			CreateShape(PxBoxGeometry(2.25f, 0.5f, 4.0f), 1.f);
			GetShape(5)->setLocalPose(PxTransform(PxVec3(-2.75f, 0.5f, 30.0f)));
		}
	};

	class Sails : public DynamicActor
	{
		Sails();

	public:
		Sails(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: DynamicActor(pose)
		{
			// Sail 1
			CreateShape(PxBoxGeometry(4.f, 1.1f, 0.1f), 1.f);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(5.f, 9.25f, 25.5f)));

			// Sail 2
			CreateShape(PxBoxGeometry(4.f, 1.1f, 0.1f), 1.f);
			GetShape(1)->setLocalPose(PxTransform(PxVec3(-5.f, 9.25f, 25.5f)));

			// Sail 3
			CreateShape(PxBoxGeometry(1.1f, 4.f, 0.1f), 1.f);
			GetShape(2)->setLocalPose(PxTransform(PxVec3(0.0f, 14.25f, 25.5f)));

			// Sail 4
			CreateShape(PxBoxGeometry(1.1f, 4.f, 0.1f), 1.f);
			GetShape(3)->setLocalPose(PxTransform(PxVec3(0.0f, 4.25f, 25.5f)));
		}
	};

	class TeeBox : public StaticActor
	{
		TeeBox();

	public:
		TeeBox(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f)
			: StaticActor(pose)
		{
			CreateShape(PxBoxGeometry(5.f, 0.1f, 5.f), 1.f);
			GetShape(0)->setLocalPose(PxTransform(PxVec3(0.f, 0.f, 0.f)));
		}
	};

	///Revolute Joint
	class RevoluteJoint : public Joint
	{
	public:
		RevoluteJoint(Actor* actor0, const PxTransform& localFrame0, Actor* actor1, const PxTransform& localFrame1)
		{
			PxRigidActor* px_actor0 = 0;
			if (actor0)
				px_actor0 = (PxRigidActor*)actor0->Get();

			joint = PxRevoluteJointCreate(*GetPhysics(), px_actor0, localFrame0, (PxRigidActor*)actor1->Get(), localFrame1);
			joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION,true);
		}

		void DriveVelocity(PxReal value)
		{
			//wake up the attached actors
			PxRigidDynamic *actor_0, *actor_1;
			((PxRevoluteJoint*)joint)->getActors((PxRigidActor*&)actor_0, (PxRigidActor*&)actor_1);
			if (actor_0)
			{
				if (actor_0->isSleeping())
					actor_0->wakeUp();
			}
			if (actor_1)
			{
				if (actor_1->isSleeping())
					actor_1->wakeUp();
			}
			((PxRevoluteJoint*)joint)->setDriveVelocity(value);
			((PxRevoluteJoint*)joint)->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
		}

		PxReal DriveVelocity()
		{
			return ((PxRevoluteJoint*)joint)->getDriveVelocity();
		}

		void SetLimits(PxReal lower, PxReal upper)
		{
			((PxRevoluteJoint*)joint)->setLimit(PxJointAngularLimitPair(lower, upper));
			((PxRevoluteJoint*)joint)->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);
		}
	};
}