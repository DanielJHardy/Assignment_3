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
	m_camera = FlyCamera(width / height, 10.0f);
	m_camera.setLookAt(vec3(10, 10, 10), vec3(0), vec3(0,1,0));
	m_camera.sensitivity = 3;

	//setup 2D projection
	projection2D = glm::ortho(-m_screen_size.x / 2.0f, m_screen_size.x / 2.0f, -m_screen_size.y / 2.0f, m_screen_size.y / 2.0f);



	glfwSetTime(0.0);
	return true;
}

void PhysicsDemoScene::shutdown()
{

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