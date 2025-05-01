#pragma once
#ifndef __BONUSLIFE_H__
#define __BONUSLIFE_H__

#include "GameObject.h"

class BonusLife : public GameObject
{
public:
    BonusLife(void);
    ~BonusLife(void);

    bool CollisionTest(shared_ptr<GameObject> o);
    void OnCollision(const GameObjectList& objects);
};

#endif