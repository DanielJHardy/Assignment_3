#include "PhysicsDemoScene.h"

#include "gl_core_4_4.h"
#include <GLFW/glfw3.h>
#include "Gizmos.h"

#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

//constants
const vec4 white(1);
const vec4 black(0, 0, 0, 1);

//prototypes
void DrawGizmoGrid(int a_size);

//custom allocator
class myAllocator : public PxAllocatorCallback
{
public:
	virtual ~myAllocator() {}
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line)
	{
		void* pointer = _aligned_malloc(size, 16);
		return pointer;
	}

	virtual void deallocate(void* ptr)
	{
		_aligned_free(ptr);
	}

private:

};


bool PhysicsDemoScene::startup()
{
	if (Application::startup() == false)
		return false;

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	//init Gizmos
	Gizmos::create();

	//get screen width and height
	int width, height;
	glfwGetWindowSize(m_window, &width, &height);

	m_screen_size = glm::vec2(width, height);

	//setup camera
	m_camera = FlyCamera(m_screen_size.x / m_screen_size.y, 10.0f);
	m_camera.setLookAt(vec3(10, 10, 10), vec3(0), vec3(0,1,0));
	m_camera.sensitivity = 3;

	//setup 2D projection
	projection2D = glm::ortho(-m_screen_size.x / 2.0f, m_screen_size.x / 2.0f, -m_screen_size.y / 2.0f, m_screen_size.y / 2.0f);

	//setup PhysX
	setupPhysX();
	setupVisualDebugger();

	//tutorials
	setupTutorial();



	glfwSetTime(0.0);
	return true;
}

void PhysicsDemoScene::shutdown()
{
	g_PhysicsScene->release();
	g_Physics->release();
	g_PhysicsFoundation->release();

	Gizmos::destroy();
	Application::shutdown();
}

bool PhysicsDemoScene::update()
{
	if (Application::update() == false)
		return false;

	//check if window size has changed
	int screen_width, screen_height;
	glfwGetWindowSize(m_window, &screen_width, &screen_height);
	if (screen_width != m_screen_size.x || screen_height != m_screen_size.y)
	{
		//update view
		glViewport(0, 0, screen_width, screen_height);
		m_screen_size = glm::vec2(screen_width, screen_height);

		m_camera.proj = glm::perspective(glm::radians(60.0f), m_screen_size.x / m_screen_size.y, 0.1f, 1000.0f);

		//update 2D projection
		projection2D = glm::ortho(-m_screen_size.x / 2.0f, m_screen_size.x / 2.0f, -m_screen_size.y / 2.0f, m_screen_size.y / 2.0f);
	}

	Gizmos::clear();

	//get deltatime
	float dt = (float)glfwGetTime();
	glfwSetTime(0.0);

	//update camera
	m_camera.update(dt);

	//INPUT

	//if alt is down make holding LMB fire constantly
	if (glfwGetKey(m_window, GLFW_KEY_LEFT_ALT))
	{
		mouse1State_last = false;
	}

	//check if mouse was pressed down on this frame
	bool mouse1State = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_1);
	if (mouse1State && !mouse1State_last)
	{
		shootSphere();

	}
	mouse1State_last = mouse1State;



	//update PhysX
	updatePhysX(dt);



	return true;
}

void PhysicsDemoScene::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//draw grid
	DrawGizmoGrid(50);

	Gizmos::draw(m_camera.proj, m_camera.view);
	Gizmos::draw2D(projection2D);

	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void PhysicsDemoScene::setupPhysX()
{
	PxAllocatorCallback *myCallback = new myAllocator();
	g_PhysicsFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, *myCallback, g_DefaultErrorCallback);

	g_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_PhysicsFoundation, PxTolerancesScale());

	PxInitExtensions(*g_Physics);

	//create physics material
	g_PhysicsMaterial = g_Physics->createMaterial(0.5f, 0.5f,0.2f);

	//create physics scene	
	PxSceneDesc sceneDesc(g_Physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f,-10.0f,0.0f);
	sceneDesc.filterShader = &PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);

	g_PhysicsScene = g_Physics->createScene(sceneDesc);


}

void PhysicsDemoScene::updatePhysX(float dt)
{
	if (dt <= 0)
		return;

	g_PhysicsScene->simulate(dt);

	while (g_PhysicsScene->fetchResults() == false)
	{
		//dont need to do anything yet but have ot fetch results
	}

	//Add widgets to represent all the physX actors which are in the scene
	for (auto actor : g_PhysXActors)
	{
		PxU32 nShapes = actor->getNbShapes();
		PxShape** shapes = new PxShape*[nShapes];
		actor->getShapes(shapes, nShapes);

		//Render all the shapes in the physX actor
		while (nShapes--)
		{
			addWidget(shapes[nShapes], actor);
		}

		delete[] shapes;
	}
}


void PhysicsDemoScene::setupVisualDebugger()
{
	//check if PvdConnection manager is available on this platform
	if (g_Physics->getPvdConnectionManager() == nullptr)
		return;

	// setup connection parameters
	const char* pvd_host_ip =	"127.0.0.1";	// IP of the PC that is running PVD
	int port =					5425;			// TCP port to connect to, where PVD is listening
	unsigned int timeout =		100;			// Timeout in milliseconds to wait for PVD to respond, consoles and remote PCs need a higher timeout

	PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();

	// Now try to connect to PVD
	auto theConnection = PxVisualDebuggerExt::createConnection(g_Physics->getPvdConnectionManager(), pvd_host_ip, port, timeout, connectionFlags);


}

void PhysicsDemoScene::setupTutorial()
{
	//add a plane
	PxTransform pose = PxTransform(PxVec3(0.0f,0.0f,0.0f), PxQuat(PxHalfPi*1.0f, PxVec3(0.0f,0.0f,1.0f)));
	PxRigidStatic* plane = PxCreateStatic(*g_Physics, pose, PxPlaneGeometry(), *g_PhysicsMaterial);

	//add it to the physX scene
	g_PhysicsScene->addActor(*plane);

	//add a box
	float density = 100;
	PxBoxGeometry box(2, 2, 2);
	PxTransform transform(PxVec3(0,10,0));
	PxRigidDynamic* dynamicActor = PxCreateDynamic(*g_Physics, transform, box, *g_PhysicsMaterial, density);

	//add it to the physX scene
	g_PhysicsScene->addActor(*dynamicActor);

	g_PhysXActors.push_back(dynamicActor);

	//add a sphere
	PxSphereGeometry sphere(2);
	PxTransform transform2(PxVec3(0, 5, 0.2));
	PxRigidDynamic* dynamicActor2 = PxCreateDynamic(*g_Physics, transform2, sphere, *g_PhysicsMaterial, density);

	//add it to the PhysX scene
	g_PhysicsScene->addActor(*dynamicActor2);

	g_PhysXActors.push_back(dynamicActor2);


}

void PhysicsDemoScene::shootSphere()
{
	float density = 100;
	PxSphereGeometry projectile(0.4f);

	PxTransform transform(PxVec3(m_camera.world[3].x, m_camera.world[3].y - 1, m_camera.world[3].z));
	PxRigidDynamic* actor = PxCreateDynamic(*g_Physics, transform, projectile, *g_PhysicsMaterial, density);

	float muzzleSpeed = -100;

	//set intial velocity
	vec3 direction(m_camera.world[2]);
	PxVec3 velocity = PxVec3(direction.x, direction.y, direction.z) * muzzleSpeed;
	actor->setLinearVelocity(velocity, true);

	//add it to the PhysX scene
	g_PhysicsScene->addActor(*actor);
	g_PhysXActors.push_back(actor);
}

//Widgets

void PhysicsDemoScene::addWidget(PxShape* shape, PxRigidActor* actor)
{
	PxGeometryType::Enum type = shape->getGeometryType();

	switch (type)
	{

		case physx::PxGeometryType::eBOX:
			addBox(shape, actor);
			break;
		case physx::PxGeometryType::eSPHERE:
			addSphere(shape, actor);
			break;
		default:
			break;
	}
}

void PhysicsDemoScene::addBox(PxShape* pShape, PxRigidActor* actor)
{
	//get the geometry for this PhysX collision volume
	PxBoxGeometry geometry;
	float width = 1, height = 1, length = 1;
	bool status = pShape->getBoxGeometry(geometry);
	if (status)
	{
		width = geometry.halfExtents.x;
		height = geometry.halfExtents.y;
		length = geometry.halfExtents.z;
	}

	//get the transform for this PhysX collision volume
	PxMat44 m(PxShapeExt::getGlobalPose(*pShape, *actor));
	mat4 M(m.column0.x, m.column0.y, m.column0.z, m.column0.w,
		m.column1.x, m.column1.y, m.column1.z, m.column1.w,
		m.column2.x, m.column2.y, m.column2.z, m.column2.w,
		m.column3.x, m.column3.y, m.column3.z, m.column3.w);

	vec3 position;

	//get the position out of the transform
	position.x = m.getPosition().x;
	position.y = m.getPosition().y;
	position.z = m.getPosition().z;

	vec3 extents = vec3(width, height, length);
	vec4 colour = vec4(1,0,0,1);

	if (actor->getName() != nullptr && strcmp(actor->getName(), "Pickup1"))	 //pickups are green
		colour = vec4(0,1,0,1);

	//create our box gizmo
	Gizmos::addAABBFilled(position, extents, colour, &M);

}

void PhysicsDemoScene::addSphere(PxShape* pShape, PxRigidActor* actor)
{
	PxSphereGeometry geometry;
	float radius = 1;

	//get the geometry for this PhysX collision volume
	bool status = pShape->getSphereGeometry(geometry);
	if (status)
	{
		radius = geometry.radius;
	}

	//position
	PxTransform pose = actor->getGlobalPose();
	vec3 position = vec3(pose.p.x, pose.p.y, pose.p.z);

	//rotation
	glm::quat q = glm::quat(pose.q.w, pose.q.x, pose.q.y, pose.q.z);
	mat4 rotation = mat4(q);

	//colour
	vec4 colour = vec4(1, 0, 0, 1);

	if (actor->getName() != nullptr && strcmp(actor->getName(), "Pickup1"))	 //pickups are green
		colour = vec4(0, 1, 0, 1);


	//create Gizmo
	Gizmos::addSphereFilled(position, radius, 12, 12, colour, &rotation);
}

void DrawGizmoGrid(int a_size)
{
	int halfsize = a_size / 2;

	for (int i = 0; i <= a_size; ++i)
	{
		Gizmos::addLine(vec3(-halfsize + i, -0.01, -halfsize), vec3(-halfsize + i, -0.01, halfsize),
			i == halfsize ? white : black);
		Gizmos::addLine(vec3(-halfsize, -0.01, -halfsize + i), vec3(halfsize, -0.01, -halfsize + i),
			i == halfsize ? white : black);
	}
}


