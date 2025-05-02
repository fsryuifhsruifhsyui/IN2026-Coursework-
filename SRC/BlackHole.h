#pragma once
#ifndef __BLACKHOLE_H__
#define __BLACKHOLE_H__

#include "GameObject.h"

class BlackHole : public GameObject
{
public:
    BlackHole(void);
    ~BlackHole(void);

    bool CollisionTest(shared_ptr<GameObject> o);
    void OnCollision(const GameObjectList& objects);
};

#endif