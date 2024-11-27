#pragma once

#include <gr-math/gmath.hpp>

#include <ostream>
#include <istream>

class GameObject;
class Entity;
class Stream;

class __attribute__((visibility("default"))) Component {
public:
    virtual void Init() {}

    virtual void Start() {}

    virtual void Update() {}

    virtual void Enable() {}

    virtual void Disable() {}

    virtual bool Serialize(Stream *streaming) { return false; }

    virtual bool Deserialize(Stream *streaming) { return false; }

    virtual void Destroy() {}

    GameObject *getGameObject() const;
    
    Entity* entity;
};


