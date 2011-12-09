/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
/* Desc: The Bullet physics engine wrapper
 * Author: Nate Koenig
 * Date: 11 June 2007
 * SVN: $Id: BulletPhysics.cc 7714 2009-05-23 18:08:49Z natepak $
 */

#include "BulletLink.hh"
#include "BulletPlaneShape.hh"
#include "BulletSphereShape.hh"
#include "BulletBoxShape.hh"
#include "BulletCylinderShape.hh"
#include "BulletTrimeshShape.hh"
#include "MapShape.hh"

#include "BulletHingeJoint.hh"
#include "BulletHinge2Joint.hh"
#include "BulletSliderJoint.hh"
#include "BulletBallJoint.hh"
#include "BulletUniversalJoint.hh"

#include "PhysicsFactory.hh"
#include "Mass.hh"
#include "common/Console.hh"
#include "common/Exception.hh"
#include "World.hh"
#include "math/Vector3.hh"
#include "Entity.hh"

#include "common/XMLConfig.hh"

#include "BulletPhysics.hh"

using namespace gazebo;
using namespace physics;

using namespace physics;

using namespace physics;


GZ_REGISTER_PHYSICS_ENGINE("bullet", BulletPhysics);

////////////////////////////////////////////////////////////////////////////////
// Constructor
BulletPhysics::BulletPhysics(World *world)
    : PhysicsEngine(world)
{
  this->collisionConfig = new btDefaultCollisionConfiguration();

  this->dispatcher = new btCollisionDispatcher(this->collisionConfig);

  this->broadPhase = new btDbvtBroadphase();

  // Create the dynamics solver
  this->solver = new btSequentialImpulseConstraintSolver;

  // Instantiate the world
  this->dynamicsWorld = new btDiscreteDynamicsWorld(this->dispatcher,
      this->broadPhase, this->solver, this->collisionConfig);

  common::Param::Begin(&this->parameters);
  common::Param::End();
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
BulletPhysics::~BulletPhysics()
{
  delete this->broadPhase;
  delete this->collisionConfig;
  delete this->dispatcher;
  delete this->solver;

  // TODO: Fix this line
  //delete this->dynamicsWorld;

  this->broadPhase = NULL;
  this->collisionConfig = NULL;
  this->dispatcher = NULL;
  this->solver = NULL;
  this->dynamicsWorld = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Load the Bullet engine
void BulletPhysics::Load(common::XMLConfigNode *node)
{
  common::XMLConfigNode *cnode = node->GetChild("bullet", "physics");
  if (cnode == NULL)
    gzthrow("Must define a <physics:ode> node in the XML file");

  this->stepTimeP->Load(cnode);

  this->gravityP->Load(cnode);
}

////////////////////////////////////////////////////////////////////////////////
// Save the Bullet engine
void BulletPhysics::Save(std::string &prefix, std::ostream &stream)
{
  stream << prefix << "<physics:bullet>\n";
  stream << prefix << "  " << *(this->gravityP) << "\n";
  stream << prefix << "</physics:bullet>\n";
}

////////////////////////////////////////////////////////////////////////////////
// Initialize the Bullet engine
void BulletPhysics::Init()
{
  math::Vector3 g = this->gravityP->GetValue();
  this->dynamicsWorld->setGravity(btmath::Vector3(g.x, g.y, g.z));
}

////////////////////////////////////////////////////////////////////////////////
/// Init the engine for threads. 
void BulletPhysics::InitForThread()
{
}

////////////////////////////////////////////////////////////////////////////////
// Update the Bullet collisions, create joints
void BulletPhysics::UpdateCollision()
{
}

////////////////////////////////////////////////////////////////////////////////
// Update the Bullet engine
void BulletPhysics::UpdatePhysics()
{
  /*common::Time time = Simulator::Instance()->GetRealTime() - this->lastUpdateTime;
  int steps = (int) round( (time / **this->stepTimeP).Double() );

  steps = std::max(steps,1);

  //time = 0.000001;
  //steps = 1;
  //this->dynamicsWorld->stepSimulation(time,  steps, (**this->stepTimeP));
  this->dynamicsWorld->stepSimulation((**this->stepTimeP).Double());

  this->lastUpdatecommon::Time = Simulator::Instance()->GetRealTime();
  */
}


////////////////////////////////////////////////////////////////////////////////
// Finilize the Bullet engine
void BulletPhysics::Fini()
{
}


////////////////////////////////////////////////////////////////////////////////
// Remove an entity from the physics engine
void BulletPhysics::RemoveEntity(Entity *entity)
{
}

////////////////////////////////////////////////////////////////////////////////
// Add an entity to the world
void BulletPhysics::AddEntity(Entity *entity)
{
  BulletLink *body = dynamic_cast<BulletLink*>(entity);

  this->dynamicsWorld->addRigidLink(body->GetBulletLink());
}

////////////////////////////////////////////////////////////////////////////////
// Create a new body
Link *BulletPhysics::CreateLink(Entity *parent)
{
  BulletLink *body = new BulletLink(parent);

  return body;
}

////////////////////////////////////////////////////////////////////////////////
/// Create a new collision
Collision *BulletPhysics::CreateCollision(std::string type, Link *parent)
{
  BulletCollision *collision = NULL;
  Shape *shape = NULL;
  BulletLink *body = NULL;

  body = dynamic_cast<BulletLink*>(parent);

  if (body == NULL)
    gzthrow("CreateCollision requires an BulletLink as a parent");

  collision = new BulletCollision(parent);

  if (type == "sphere")
    shape = new BulletSphereShape(collision);
  if (type == "box")
    shape = new BulletBoxShape(collision);
  if (type == "cylinder")
    shape = new BulletCylinderShape(collision);
  if (type == "plane")
    shape = new BulletPlaneShape(collision);
  if (type == "trimesh")
    shape = new BulletTrimeshShape(collision);
  if (type == "map")
    shape = new MapShape(collision);
  else 
    gzthrow("Unable to create a collision of type[" << type << "]");

  return collision;
}


////////////////////////////////////////////////////////////////////////////////
// Create a new joint
Joint *BulletPhysics::CreateJoint(std::string type)
{
  if (type == "slider")
    return new BulletSliderJoint(this->dynamicsWorld);
  if (type == "hinge")
    return new BulletHingeJoint(this->dynamicsWorld);
  if (type == "hinge2")
    return new BulletHinge2Joint(this->dynamicsWorld);
  if (type == "ball")
    return new BulletBallJoint(this->dynamicsWorld);
  if (type == "universal")
    return new BulletUniversalJoint(this->dynamicsWorld);
  else 
    gzthrow("Unable to create joint of type[" << type << "]");

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Convert an odeMass to Mass
void BulletPhysics::ConvertMass(Mass *mass, void *engineMass)
{
}

////////////////////////////////////////////////////////////////////////////////
/// Convert an gazebo Mass to a bullet Mass
void BulletPhysics::ConvertMass(void *engineMass, const Mass &mass)
{
}

////////////////////////////////////////////////////////////////////////////////
// Create an object to hold a set of ray collisions
/*PhysicsRaySensor *BulletPhysics::CreateRaySensor(Link *body)
{
  return NULL;
}*/

////////////////////////////////////////////////////////////////////////////////
math::Pose BulletPhysics::ConvertPose(btTransform bt)
{
  math::Pose pose;
  pose.pos.x = bt.getOrigin().getX();
  pose.pos.y = bt.getOrigin().getY();
  pose.pos.z = bt.getOrigin().getZ();

  pose.rot.u = bt.getRotation().getW();
  pose.rot.x = bt.getRotation().getX();
  pose.rot.y = bt.getRotation().getY();
  pose.rot.z = bt.getRotation().getZ();

  return pose;
}

////////////////////////////////////////////////////////////////////////////////
/// Convert a gazebo pose to a bullet transform
btTransform BulletPhysics::ConvertPose(const math::Pose pose)
{
  btTransform trans;

  trans.setOrigin( btmath::Vector3( pose.pos.x, pose.pos.y, pose.pos.z) );
  trans.setRotation( btQuaternion( pose.rot.x, pose.rot.y, 
                                   pose.rot.z, pose.rot.u) );
  return trans;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the gavity vector
void BulletPhysics::SetGravity(const gazebo::math::Vector3 &gravity)
{
  this->gravityP->SetValue(gravity);
  this->dynamicsWorld->setGravity(btmath::Vector3(gravity.x, gravity.y, gravity.z));
}