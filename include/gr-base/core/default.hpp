#pragma once

#include <stdint.h>

#define GID_INVALID  (GameObjectHandle)-1

#define GID_INDEX_MASK      0x00000000FFFFFFFF
#define GID_VERSION_MASK    0xFFFFFFFF00000000
#define GID_VERSION_SHIFT   32

#define CreateHandle(i, v) \
    static_cast<uint64_t>(i) | (static_cast<uint64_t>(v) << GID_VERSION_SHIFT)

#define GetHandleID(i) \
    static_cast<uint32_t>(i & GID_INDEX_MASK)

#define GetHandleVersion(i) \
    static_cast<uint32_t>(i >> GID_VERSION_SHIFT)

typedef uint64_t GameObjectHandle;

class GameObject;
class Scene;


