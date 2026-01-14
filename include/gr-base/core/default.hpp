#pragma once

#include <stdint.h>

#define GID_INVALID  (GameObjectID)-1

#define GID_INDEX_MASK      0x00000000FFFFFFFF
#define GID_VERSION_MASK    0xFFFFFFFF00000000
#define GID_VERSION_SHIFT   32

#define CreateGameObjectID(i, v) \
    static_cast<uint64_t>(i) | (static_cast<uint64_t>(v) << GID_VERSION_SHIFT)

#define GetIDIndex(i) \
    static_cast<uint32_t>(i & GID_INDEX_MASK)

#define GetIDVersion(i) \
    static_cast<uint32_t>(i >> GID_VERSION_SHIFT)

typedef uint64_t GameObjectID;

class GameObject;
class Scene;


