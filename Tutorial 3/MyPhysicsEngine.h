#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;
	
	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;

		MySimulationEventCallback() : trigger(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) 
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;
						trigger = true;
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
						trigger = false;
					}
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs) 
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;
				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	///Custom scene class
	class MyScene : public Scene
	{
		MySimulationEventCallback* my_callback;
		Sphere* ball, *ballCopy;
		Club* club, *clubCopy;
		Box* clubRot, *clubRotCopy;
		Course* course;
		CourseMiddle* courseMiddle;
		TeeBox* teeBox;
		Barriers* barriers;
		RevoluteJoint* clubJoint, *clubJointCopy;
		Windmill* windmill;
		Sails* sails;
		Box* sailRot;
		RevoluteJoint* sailJoint;
		Cloth* flag, *flagCopy;
		Capsule* flagPole;
		PxMaterial* concrete, *asphalt;

		bool win = false;
		PxTransform ballInitTransform, clubInitTransform, clubRotInitTransform, sailsInitTransform, flagPoleInitTransform;
		
	public:
		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene() {};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit() 
		{
			SetVisualisation();			

			GetMaterial()->setDynamicFriction(.2f);

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);

			//							Static		Sliding
			// Rubber on Dry Concrete	0.6 - 0.85	0.6 - 0.85
			// Rubber on Dry Asphalt	0.5 - 0.8	0.5	- 0.8
			// https://www.engineeringtoolbox.com/friction-coefficients-d_778.html
			// http://www.engineershandbook.com/Tables/frictioncoefficients.htm
			// http://www.roymech.co.uk/Useful_Tables/Tribology/co_of_frict.htm
			concrete = GetPhysics()->createMaterial(0.6f, 0.6f, 0.4f); // static friction, dynamic friction, restitution
			asphalt = GetPhysics()->createMaterial(0.5f, 0.5f, 0.7f);

			for (int i = 0; i < 1; i++)//	DUPLICATE ENTIRE COURSE
			{
				course = new Course(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(0.f, 0.f, 0.f))));
				course->Material(concrete);
				course->Color(PxVec3(0.5f, 0.5f, 0.5f));

				courseMiddle = new CourseMiddle(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(0.f, 0.f, 0.f))));
				courseMiddle->Material(asphalt);
				courseMiddle->Color(PxVec3(0.f, 0.f, 0.f));

				teeBox = new TeeBox(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(0.f, 0.f, 0.f))));
				teeBox->Color(PxVec3(1.f, 0.75f, 0.75f));

				barriers = new Barriers(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(0.f, 0.f, 0.f))));
				barriers->Color(PxVec3(0.75f, 0.75f, 1.f));

				windmill = new Windmill(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(0.f, 0.f, 0.f))));
				windmill->Color(PxVec3(0.75, 0.5f, 0.5f));

				club = new Club(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(1.f, 0.f, 0.f))));
				club->Color(PxVec3(0.f, 0.f, 1.f));
				clubInitTransform = ((PxRigidBody*)club->Get())->getGlobalPose();

				clubRot = new Box(PxTransform(PxVec3(0.f, 20.f, 0.f)));
				clubRot->SetKinematic(true);
				((PxRigidBody*)clubRot->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				clubRot->Color(PxVec3(1.f, 1.f, 1.f));
				clubRotInitTransform = ((PxRigidBody*)clubRot->Get())->getGlobalPose();

				clubJoint = new RevoluteJoint(
					clubRot,
					PxTransform(PxVec3(0.f, -10.f, 0.f),
						PxQuat(PxPi / 2, PxVec3(1.f, 0.f, 0.f))),
					club,
					PxTransform(PxVec3(0.f, 9.5f, 0.f)));

				clubJoint->SetLimits(-PxPi / 2 - PxPi / 4, PxPi / 2 - (2 * PxPi) / 3);

				sails = new Sails(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(0.f, 0.f, 0.f))));
				sails->Color(PxVec3(1.f, 0.9f, 0.9f));
				((PxRigidBody*)sails->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

				sailRot = new Box(PxTransform(PxVec3(0.f, 9.25f, 25.5f)));
				sailRot->SetKinematic(true);
				sailRot->Color(PxVec3(0.75, 0.5f, 0.5f));

				sailJoint = new RevoluteJoint(
					sailRot,
					PxTransform(PxVec3(0.f, 0.f, 0.f),
						PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))),
					sails,
					PxTransform(PxVec3(0.f, 9.25f, 25.5f),
						PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))));
				sailJoint->DriveVelocity(1.f);

				ball = new Sphere(PxTransform(PxVec3(0.f, 0.1f, 1.f)), 0.35f);
				((PxRigidBody*)ball->Get())->setGlobalPose(PxTransform(PxVec3(0.f, 0.1f, 0.8f)));
				ball->Color(PxVec3(1.0f, 1.f, 1.f));
				ball->Material(concrete);
				((PxRigidDynamic*)ball->Get())->setLinearDamping(0.1f);
				ballInitTransform = ((PxRigidBody*)ball->Get())->getGlobalPose();

				flag = new Cloth(PxTransform(PxVec3(0.f, 10.f, 50.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))), PxVec2(2.f, 2.f), 20, 20, true);
				flag->Color(PxVec3(1.f, 0.f, 0.f));
				((PxCloth*)flag->Get())->setExternalAcceleration(PxVec3(-10.0f, 5.0f, 0.0f));
				((PxCloth*)flag->Get())->setGlobalPose(PxTransform(PxVec3(0.f, 10.f, 50.f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))));

				flagPole = new Capsule(PxTransform(PxIdentity), PxVec2(0.05f, 5.95f));
				flagPole->Color(PxVec3(0.4f, 0.2f, 0.2f));
				((PxRigidBody*)flagPole->Get())->setGlobalPose(PxTransform(PxVec3(0.f, 6.1f, 50.f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))));
				((PxRigidBody*)flagPole->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				((PxRigidBody*)flagPole->Get())->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);

				Add(course);
				Add(courseMiddle);
				Add(teeBox);
				Add(barriers);
				Add(club);
				Add(clubRot);
				Add(ball);
				Add(windmill);
				Add(sails);
				Add(sailRot);
				Add(flag);
				Add(flagPole);

				if (i == 0)
				{
					ballInitTransform = ((PxRigidBody*)ball->Get())->getGlobalPose();
					clubInitTransform = ((PxRigidBody*)club->Get())->getGlobalPose();
					sailsInitTransform = ((PxRigidBody*)sails->Get())->getGlobalPose();
					flagPoleInitTransform = ((PxRigidBody*)flagPole->Get())->getGlobalPose();
				}
			}

			//		DUPLICATE OBJECTS
			int dupe_number = 0;
			PxReal dupe_offset_x = 0.f, dupe_offset_y = 1.5f, dupe_offset_z = -0.5f;
			bool dupe_ball = 0;
			bool dupe_club = 0;
			bool dupe_flag = 0;

			for (int i = 0; i < dupe_number; i++)
			{
				//	BALL
				if (dupe_ball)
				{
					ballCopy = new Sphere(PxTransform(PxVec3(0.f, 0.1f, 1.f)), 0.35f);
					((PxRigidBody*)ballCopy->Get())->setGlobalPose(PxTransform(PxVec3(
						0.f + (dupe_offset_x * (i + 1)),
						0.1f + (dupe_offset_y * (i + 1)),
						0.8f + (dupe_offset_z * (i + 1)))));
					ballCopy->Color(PxVec3(1.0f, 1.f, 1.f));
					ballCopy->Material(concrete);
					((PxRigidDynamic*)ballCopy->Get())->setLinearDamping(0.1f);
					Add(ballCopy);
				}

				//	CLUB
				if (dupe_club)
				{
					clubCopy = new Club(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(0.f, PxVec3(1.f, 0.f, 0.f))));
					clubCopy->Color(PxVec3(0.f, 0.f, 1.f));
					clubRotCopy = new Box(PxTransform(PxVec3(0.f, 20.f, 0.f)));
					clubRotCopy->SetKinematic(true);
					((PxRigidBody*)clubRotCopy->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
					clubRotCopy->Color(PxVec3(1.f, 1.f, 1.f));
					clubJointCopy = new RevoluteJoint(
						clubRotCopy,
						PxTransform(PxVec3(
							0.f + (dupe_offset_x * (i + 1)),
							-10.f + (dupe_offset_y * (i + 1)),
							0.f + (dupe_offset_z * (i + 1))),
							PxQuat(PxPi / 2, PxVec3(1.f, 0.f, 0.f))),
						clubCopy,
						PxTransform(PxVec3(0.f, 9.5f, 0.f)));
					clubJointCopy->SetLimits(-PxPi / 2 - PxPi / 4, PxPi / 2 - (2 * PxPi) / 3);
					Add(clubCopy);
					Add(clubRotCopy);
				}

				//	FLAG
				if (dupe_flag)
				{
					flagCopy = new Cloth(PxTransform(PxVec3(0.f, 10.f, 50.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))), PxVec2(2.f, 2.f), 20, 20, true);
					flagCopy->Color(PxVec3(1.f, 0.f, 0.f));
					((PxCloth*)flagCopy->Get())->setExternalAcceleration(PxVec3(-10.0f, 5.0f, 0.0f));
					((PxCloth*)flagCopy->Get())->setGlobalPose(PxTransform(PxVec3(
						0.f + (dupe_offset_x * (i + 1)),
						10.f + (dupe_offset_y * (i + 1)),
						50.f + (dupe_offset_z * (i + 1))),
						PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))));
					Add(flagCopy);
				}
			}
		}

		void simulationTesting()
		{
			PxVec3 pBall = ((PxRigidBody*)ball->Get())->getGlobalPose().p;
			PxVec3 pClub = ((PxRigidBody*)club->Get())->getGlobalPose().p;
			PxVec3 pSails = ((PxRigidBody*)sails->Get())->getGlobalPose().p;
			PxVec3 pFlagPole = ((PxRigidBody*)flagPole->Get())->getGlobalPose().p;

			cout << "Ball displacement\t\t" << (pBall - ballInitTransform.p).magnitude() << endl;
			cout << "Club displacement\t\t\t" << (pClub - clubInitTransform.p).magnitude() << endl;
			cout << "Windmill sail(s) displacement\t\t\t" << (pSails - sailsInitTransform.p).magnitude() << endl;
			cout << "Flagpole displacement\t\t\t\t\t" << (pFlagPole - flagPoleInitTransform.p).magnitude() << endl;

			cout << endl;
		}

		//Custom udpate function
		virtual void CustomUpdate()
		{
			checkWinState();
		}

		void swingClub(PxReal strength)
		{
			((PxRigidBody*)club->Get())->addForce(PxVec3(0, 0, 1)*strength);
		}

		void translateClub(PxVec3 t)
		{
			swingClub(-0.01f);

			PxVec3 p = ((PxRigidBody*)clubRot->Get())->getGlobalPose().p;
			p += t;
			((PxRigidBody*)clubRot->Get())->setGlobalPose(PxTransform(p));
		}

		void checkWinState()
		{
			PxVec3 p = ((PxRigidBody*)ball->Get())->getGlobalPose().p;
			PxReal xMin = -1.0f, xMax = 1.0f, yMax = -0.5f, zMin = 49.0f, zMax = 51.0f;
			if (win == false && p.y < yMax && p.x > xMin && p.x < xMax && p.z > zMin && p.z < zMax)
			{
				win = true;
				std::cout << "YOU WIN!" << std::endl;
			}
		}

		void resetGame()
		{
			((PxRigidBody*)ball->Get())->setGlobalPose(ballInitTransform);
			((PxRigidBody*)ball->Get())->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
			((PxRigidBody*)ball->Get())->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));

			((PxRigidBody*)club->Get())->setGlobalPose(clubInitTransform);
			((PxRigidBody*)club->Get())->setLinearVelocity(PxVec3(0.f, 0.f, 0.f));
			((PxRigidBody*)club->Get())->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));

			((PxRigidBody*)clubRot->Get())->setGlobalPose(clubRotInitTransform);
		}
	};
}
