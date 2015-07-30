#ifndef _PHYSICSDEMOSCENE_H_
#define	_PHYSICSDEMOSCENE_H_

#include "Application.h"
#include "Camera.h"

#include <PxPhysicsAPI.h>
#include <PxScene.h>
#include <pvd/PxVisualDebugger.h>

#include <vector>

using namespace physx;

class PhysicsDemoScene : public Application
{
public:
	virtual bool startup();
	virtual void shutdown();
	virtual bool update();
	virtual void draw();

	//physics
	void setupPhysX();
	void updatePhysX(float dt);

	void setupVisualDebugger();

	//Widgets
	void addWidget(PxShape* shape, PxRigidActor* actor);
	void addBox(PxShape* pShape, PxRigidActor* actor);
	void addSphere(PxShape* pShape, PxRigidActor* actor);

	//tutorials
	void setupTutorial();


	//shoot
	void shootSphere();


public:		
	//graphics
	FlyCamera m_camera;
	mat4 projection2D;

	glm::vec2 m_screen_size;

	//physics
	PxFoundation* g_PhysicsFoundation;
	PxPhysics* g_Physics;
	PxScene* g_PhysicsScene;
	PxDefaultErrorCallback g_DefaultErrorCallback;
	PxDefaultAllocator g_DefaultAllocator;
	PxSimulationFilterShader g_DefaultFilterShader = PxDefaultSimulationFilterShader;
	PxMaterial* g_PhysicsMaterial;
	PxMaterial* g_boxMaterial;
	PxCooking* g_PhysicsCooker;

	std::vector<PxRigidActor*> g_PhysXActors;

	//input
	bool mouse1State_last = false;



};


#endif // !_PHYSICSDEMOSCENE_H_
