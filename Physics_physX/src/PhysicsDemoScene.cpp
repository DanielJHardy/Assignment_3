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
	m_camera = FlyCamera((float)(width / height), 10.0f);
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
	g_PhysicsMaterial = g_Physics->createMaterial(0.5f, 0.5f,0.5f);

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
	float density = 10;
	PxBoxGeometry box(2, 2, 2);
	PxTransform transform(PxVec3(0,5,0));
	PxRigidDynamic* dynamicActor = PxCreateDynamic(*g_Physics, transform, box, *g_PhysicsMaterial, density);

	//add it to the physX scene
	g_PhysicsScene->addActor(*dynamicActor);


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

//tutorials

