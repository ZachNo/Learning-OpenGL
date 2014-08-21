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

class Entity
{
	GLuint modelIndex;
	GLuint textureIndex;
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
	bool visible;

	btCollisionShape *colShape;
	bool customCol;
	btDefaultMotionState* motionState;
	btRigidBody* rigidBody;
	bool nonMoving;

	ModelManager *modMan;
public:
	Entity(ModelManager *mod, GLuint modI, GLuint texI, glm::vec3 p, glm::quat r, btCollisionShape* col, bool customColShape, btScalar mass, btVector3 *interia);
	void setPosition(glm::vec3 newPos);
	glm::vec3 getPosition()
	{ return pos; }
	void setRotation(glm::quat newRot);
	glm::quat getRotation()
	{ return rot; }
	void setScale(glm::vec3 newScale)
	{ scale = newScale; }
	glm::vec3 getScale()
	{ return scale; }
	void setColShape(btCollisionShape *shape)
	{ colShape = shape; }
	btCollisionShape* getColShape()
	{ return colShape; }
	btRigidBody* getRigidBody()
	{ return rigidBody; }
	bool isCustomShape()
	{ return customCol; }
	void setRestitution(float res)
	{ rigidBody->setRestitution(res); }
	void setFriction(float fric)
	{ rigidBody->setFriction(fric); }
	GLuint getModelIndex()
	{ return modelIndex; }
	bool draw(glm::mat4* proj, glm::mat4* view);
	void update();

};

class EntityManager
{
	std::vector<Entity> allEntities;
	TextureManager *texMan;
	ModelManager *modMan;
	btDynamicsWorld *dynamicsWorld;
public:
	EntityManager(GLuint TextureID, GLuint matID, GLuint VMID, GLuint MMID, btDynamicsWorld *dyWorld)
	{
		modMan = new ModelManager(TextureID, matID, VMID, MMID);
		texMan = new TextureManager;
		dynamicsWorld = dyWorld;
	};
	~EntityManager();
	ModelManager* getModMan()
	{ return modMan; }
	bool createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot);
	bool createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot, btCollisionShape* colShape);
	bool createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot, btCollisionShape* colShape, btScalar mass, btVector3 *interia);
	bool drawAll(glm::mat4* proj, glm::mat4* view);
	void updateAll();
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