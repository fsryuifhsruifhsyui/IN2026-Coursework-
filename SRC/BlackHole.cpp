#include <stdlib.h>
#include "GameUtil.h"
#include "BlackHole.h"
#include "BoundingShape.h"
#include "Asteroid.h"

BlackHole::BlackHole(void) : GameObject("BlackHole")
{
    mAngle = rand() % 360;
    mPosition.x = rand() / 2;
    mPosition.y = rand() / 2;
    mPosition.z = 0.0;

}

BlackHole::~BlackHole(void)
{
}

bool BlackHole::CollisionTest(shared_ptr<GameObject> o)
{
    if (o->GetType() == GameObjectType("Spaceship") || o->GetType() == GameObjectType("Bullet")) return false;
    if (o->GetType() != GameObjectType("Asteroid")) return false;
    if (!mBoundingShape || !o->GetBoundingShape()) return false;
    return mBoundingShape->CollisionTest(o->GetBoundingShape());
}


void BlackHole::OnCollision(const GameObjectList& objects)
{
    for (auto obj : objects)
    {
        if (obj->GetType() == GameObjectType("Asteroid"))
        {
            mWorld->FlagForRemoval(obj); 
        }
    }
}
