#include "PhysicsDemoScene.h"

int main()
{
	PhysicsDemoScene app;

	//if startup fails end program
	if (app.startup() == false)
		return -1;

	//update and draw
	while (app.update() == true)
	{
		app.draw();
	}

	//shutdown
	app.shutdown();

	return 0;
}