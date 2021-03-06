#include "controls.h"

const float MOUSESPEED = 0.005f;

void computeMatricesFromInputs(GLFWwindow* window, btDynamicsWorld* world, float* horzAng, float* vertAng, float fov, Entity* ent, glm::mat4* ViewMatrix, glm::mat4* ProjectionMatrix, bool mouseLock)
{
	static double lastTime = glfwGetTime();

	glm::vec3 orbitPos = ent->getPosition();

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (mouseLock)
	{
		// Reset mouse position for next frame
		glfwSetCursorPos(window, 1024 / 2, 768 / 2);

		// Compute new orientation
		*horzAng += MOUSESPEED * float(1024 / 2 - xpos);
		//*vertAng += MOUSESPEED * float(768 / 2 - ypos);
	}

	//Direction is straight forward
	glm::vec3 direction( sin(*horzAng), 0, cos(*horzAng) );

	// Right vector
	glm::vec3 right = glm::vec3(
		sin(*horzAng - 3.14f / 2.0f),
		0,
		cos(*horzAng - 3.14f / 2.0f)
		);

	//Controls for rolling the ball
	if (glfwGetKey(window, GLFW_KEY_W))
	{
		btVector3 btDir = btVector3(-direction.x, direction.y, -direction.z);
		ent->getRigidBody()->applyForce(btDir, btVector3(0, 5, 0));
	}
	if (glfwGetKey(window, GLFW_KEY_S))
	{
		btVector3 btDir = btVector3(direction.x, direction.y, direction.z);
		ent->getRigidBody()->applyForce(btDir, btVector3(0, 5, 0));
	}
	if (glfwGetKey(window, GLFW_KEY_A))
	{
		btVector3 btDir = btVector3(right.x, right.y, right.z);
		ent->getRigidBody()->applyForce(btDir, btVector3(0, 5, 0));
	}
	if (glfwGetKey(window, GLFW_KEY_D))
	{
		btVector3 btDir = btVector3(-right.x, right.y, -right.z);
		ent->getRigidBody()->applyForce(btDir, btVector3(0, 5, 0));
	}

	//Control for jumping
	if (glfwGetKey(window, GLFW_KEY_SPACE) && glfwGetTime() - lastTime > 2.5f)
	{
		//If time since last jump is more than 2.5 seconds

		btVector3 normal(0.0f, 0.0f, 0.0f);
		int numManifolds = world->getDispatcher()->getNumManifolds();
		for (int i = 0; i<numManifolds; i++)
		{
			btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
			btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
			btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());

			//If neither object is the ball, skip
			if (obA != ent->getRigidBody() && obB != ent->getRigidBody())
				continue;

			int numContacts = contactManifold->getNumContacts();
			for (int j = 0; j<numContacts; j++)
			{
				btManifoldPoint pt = contactManifold->getContactPoint(j);
				if (pt.getDistance() < 0.1f)
					normal = pt.m_normalWorldOnB;
			}
		}
		//If found a contact point between the ball and anything, jump away from the thing
		if (normal != btVector3(0.0f, 0.0f, 0.0f))
		{
			ent->getRigidBody()->applyImpulse(normal * 10, btVector3(0, 1, 0));
			lastTime = glfwGetTime();
		}
	}


	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	//Orbit camera offset
	glm::vec3 offset = glm::vec3(3 * sin(*horzAng), 0, 3 * cos(*horzAng));

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	*ProjectionMatrix = glm::perspective(fov, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	*ViewMatrix = glm::lookAt(
		orbitPos + offset + glm::vec3(0, 2, 0), // Camera is here
		orbitPos, // and looks here : at the same position, plus "direction"
		up // Head is up (set to 0,-1,0 to look upside-down)
		);
}