#ifndef Z_ENT
#define Z_ENT

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <btBulletDynamicsCommon.h>

#include "textureManager.h"
#include "modelManager.h"

//Entity class used for all objects in game
class Entity
{
	GLuint modelIndex; //Model # from the model manager
	GLuint textureIndex; //Texture # from the texture manager
	glm::vec3 pos; //position of the obj
	glm::quat rot; //rotation of the obj
	glm::vec3 scale; //scale of the obj
	bool visible; //if the object is visible

	btCollisionShape *colShape; //collsion shape used in physics
	bool customCol; //is it using a collsion shape other than the one provided by the model manager?
	btRigidBody* rigidBody; //rigid body used in physics
	bool nonMoving; //is the physics body static

	ModelManager *modMan; //model manager
public:
	//constructor
	Entity(ModelManager *mod, GLuint modI, GLuint texI, glm::vec3 p, glm::quat r, btCollisionShape* col, bool customColShape, btScalar mass, btVector3 *interia);

	//Set the positon of the obj
	void setPosition(glm::vec3 newPos);
	//Get the position of the obj
	glm::vec3 getPosition()
	{ return pos; }

	//Set the rotation of the obj
	void setRotation(glm::quat newRot);
	//Get the rotation of the obj
	glm::quat getRotation()
	{ return rot; }

	//Set the scale of the obj
	void setScale(glm::vec3 newScale)
	{ scale = newScale; }
	//get the scale of the obj
	glm::vec3 getScale()
	{ return scale; }

	//set the collision shape of the obj
	void setColShape(btCollisionShape *shape)
	{ colShape = shape; }
	//get the collsion shape of the obj
	btCollisionShape* getColShape()
	{ return colShape; }

	//Get rigid body of obj
	btRigidBody* getRigidBody()
	{ return rigidBody; }

	//Get if obj using custom collision shape
	bool isCustomShape()
	{ return customCol; }

	//Set the restitution of the obj
	void setRestitution(float res)
	{ rigidBody->setRestitution(res); }

	//set the friction of the obj
	void setFriction(float fric)
	{ rigidBody->setFriction(fric); }

	//get the model index
	GLuint getModelIndex()
	{ return modelIndex; }

	//draw the obj
	bool draw(glm::mat4* proj, glm::mat4* view, bool drawOnlyVerts, GLuint *matID);
	bool draw(glm::mat4* proj, glm::mat4* view, bool drawOnlyVerts, GLuint *matID, GLuint overrideTex);
	//update the obj
	void update();

};

class EntityManager
{
	std::vector<Entity> allEntities; //Vector of all the entities
	TextureManager *texMan; //Texture manager
	ModelManager *modMan; //model manager
	btDynamicsWorld *dynamicsWorld; //dynamics world for physics
public:
	EntityManager(GLuint TextureID, GLuint matID, GLuint VMID, GLuint MMID, btDynamicsWorld *dyWorld)
	{
		modMan = new ModelManager(TextureID, matID, VMID, MMID);
		texMan = new TextureManager;
		dynamicsWorld = dyWorld;
	};
	~EntityManager();
	//get the model manager
	ModelManager* getModMan()
	{ return modMan; }
	//Create entity
	bool createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot);
	bool createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot, btCollisionShape* colShape);
	bool createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot, btCollisionShape* colShape, btScalar mass, btVector3 *interia);
	//draw all entities
	bool drawAll(glm::mat4* proj, glm::mat4* view);
	bool drawAll(glm::mat4* proj, glm::mat4* view, bool drawOnlyVerts, GLuint *matID);
	bool drawAll(glm::mat4* proj, glm::mat4* view, bool drawOnlyVerts, GLuint *matID, GLuint overrideTex);
	//update all entities
	void updateAll();
	//get a certain entity
	Entity* getEntity(unsigned int index)
	{ 
		if (index > allEntities.size()-1)
			throw "Index above allEntites size.";
		if (index < 0)
			throw "Index below 0.";

		return &allEntities.at(index);
	}
};

#endif