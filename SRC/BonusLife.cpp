#include <stdlib.h>
#include "GameUtil.h"
#include "BonusLife.h"
#include "BoundingShape.h"
#include "Asteroid.h"

BonusLife::BonusLife(void) : GameObject("BonusLife")
{
    mAngle = rand() % 360;
    mPosition.x = rand() / 2;
    mPosition.y = rand() / 2;
    mPosition.z = 0.0;

}

BonusLife::~BonusLife(void)
{
}

bool BonusLife::CollisionTest(shared_ptr<GameObject> o)
{
    // Only allow collision with the spaceship
    if (o->GetType() != GameObjectType("Spaceship")) return false;
    if (mBoundingShape.get() == NULL) return false;
    if (o->GetBoundingShape().get() == NULL) return false;
    return mBoundingShape->CollisionTest(o->GetBoundingShape());
}


void BonusLife::OnCollision(const GameObjectList& objects)
{
    mWorld->FlagForRemoval(GetThisPtr());
}