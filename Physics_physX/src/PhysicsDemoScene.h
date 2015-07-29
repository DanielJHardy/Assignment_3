#ifndef _PHYSICSDEMOSCENE_H_
#define	_PHYSICSDEMOSCENE_H_

#include "Application.h"
#include "Camera.h"

class PhysicsDemoScene : public Application
{
public:
	virtual bool startup();
	virtual void shutdown();
	virtual bool update();
	virtual void draw();

public:		
	FlyCamera m_camera;

	glm::vec2 m_screen_size;

	mat4 projection2D;


};


#endif // !_PHYSICSDEMOSCENE_H_
