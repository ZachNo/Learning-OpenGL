#include <btBulletDynamicsCommon.h>

#include "entity.h"

//Draw the model through the model manager
bool Entity::draw(glm::mat4* proj, glm::mat4* view)
{
	if (visible)
	{
		if (!modMan->draw(modelIndex, textureIndex, pos, rot, scale, proj, view))
			return 0;
	}
	return 1;
}

bool EntityManager::createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot)
{
	GLuint modelIndex = modMan->newModel(modelFile);
	GLuint textureIndex = texMan->importTexture(textureFile);
	Entity newEnt(modMan, modelIndex, textureIndex, pos, rot, modMan->getColShape(modelIndex), 0, 0, &btVector3(0, 0, 0));
	dynamicsWorld->addRigidBody(newEnt.getRigidBody());
	allEntities.push_back(newEnt);
	return 1;
}

bool EntityManager::createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot, btCollisionShape* colShape)
{
	GLuint modelIndex = modMan->newModel(modelFile);
	GLuint textureIndex = texMan->importTexture(textureFile);
	Entity newEnt(modMan, modelIndex, textureIndex, pos, rot, colShape, 1, 0, &btVector3(0, 0, 0));
	dynamicsWorld->addRigidBody(newEnt.getRigidBody());
	allEntities.push_back(newEnt);
	return 1;
}

bool EntityManager::createEntity(std::string modelFile, std::string textureFile, glm::vec3 pos, glm::quat rot, btCollisionShape* colShape, btScalar mass, btVector3 *interia)
{
	GLuint modelIndex = modMan->newModel(modelFile);
	GLuint textureIndex = texMan->importTexture(textureFile);
	Entity *newEnt;
	if (colShape == NULL)
		newEnt = new Entity(modMan, modelIndex, textureIndex, pos, rot, modMan->getColShape(modelIndex), 0, mass, interia);
	else
		newEnt = new Entity(modMan, modelIndex, textureIndex, pos, rot, colShape, 1, mass, interia);
	dynamicsWorld->addRigidBody(newEnt->getRigidBody());
	allEntities.push_back(*newEnt);
	delete newEnt;
	return 1;
}

//Draw all entities
bool EntityManager::drawAll(glm::mat4* proj, glm::mat4* view)
{
	if (allEntities.size() < 1)
		return 1;
	for (unsigned int i = 0; i < allEntities.size(); i++)
	{
		if (!allEntities.at(i).draw(proj, view))
			return 0;
	}
	return 1;
}

//Entity constructor
Entity::Entity(ModelManager *mod, GLuint modI, GLuint texI, glm::vec3 p, glm::quat r, btCollisionShape* col, bool customColShape, btScalar mass, btVector3 *interia)
{
	modMan = mod;
	modelIndex = modI;
	textureIndex = texI;
	pos = p;
	rot = r;
	scale = glm::vec3(1.0f, 1.0f, 1.0f);
	visible = 1;
	colShape = col;
	customCol = customColShape;
	//Set to nonmoving if no mass, saves update call
	if (mass == 0)
		nonMoving = 1;

	//Create bullet physics stuff
	motionState = new btDefaultMotionState(btTransform(btQuaternion(r.w, r.x, r.y, r.z), btVector3(p.x, p.y, p.z)));
	if (mass > 0)
		colShape->calculateLocalInertia(mass, *interia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, colShape, *interia);
	rigidBody = new btRigidBody(rigidBodyCI);
}

//EntityManager destructor
EntityManager::~EntityManager()
{
	//Loop through all entities and remove Rigid Bodies and such
	unsigned int size = allEntities.size();
	for (unsigned int i = 0; i < size; i++)
	{
		btRigidBody* rigid = allEntities.at(i).getRigidBody();
		dynamicsWorld->removeRigidBody(rigid);
		delete rigid->getMotionState();
		delete rigid;
		if (allEntities.at(i).isCustomShape() && allEntities.at(i).getColShape() != NULL)
		{
			//if using custom shape, remove shape from all objects using it and delete it
			btCollisionShape *shp = allEntities.at(i).getColShape();
			for (unsigned int i = 0; i < allEntities.size(); i++)
			{
				if (allEntities.at(i).getColShape() == shp)
					allEntities.at(i).setColShape(NULL);
			}
			delete shp;
		}
	}
	//Delete modelmanager and texturemanager
	delete modMan;
	delete texMan;
}

//Update all entities
void EntityManager::updateAll()
{
	for (unsigned int i = 0; i < allEntities.size(); i++)
		allEntities.at(i).update();
}

//Update model position/rotation to Bullet sim object
void Entity::update()
{
	if (nonMoving)
		return;
	btTransform trans;
	rigidBody->getMotionState()->getWorldTransform(trans);
	btVector3 btpos = trans.getOrigin();
	btQuaternion btrot = trans.getRotation();
	pos = glm::vec3(btpos.getX(), btpos.getY(), btpos.getZ());
	rot = glm::quat(btrot.getW(), btrot.getX(), btrot.getY(), btrot.getZ());
}

//Set position of model and of bullet object
void Entity::setPosition(glm::vec3 newPos)
{
	pos = newPos;
	btTransform trans;
	rigidBody->getMotionState()->getWorldTransform(trans);
	trans.setOrigin(btVector3(newPos.x, newPos.y, newPos.z));
	rigidBody->getMotionState()->setWorldTransform(trans);
}

//set rotation of model and bullet object
void Entity::setRotation(glm::quat newRot)
{
	rot = newRot;
	btTransform trans;
	rigidBody->getMotionState()->getWorldTransform(trans);
	trans.setRotation(btQuaternion(newRot.w, newRot.x, newRot.y, newRot.z));
	rigidBody->getMotionState()->setWorldTransform(trans);
}