#pragma once

#include <gr-math/gmath.hpp>

typedef struct DebugVertex
{
    Vector3 position;
    Color color;
} DebugVertex;

typedef struct DebugLine
{
    Vector3 start;
    Vector3 end;
    Color color;
} DebugLine;

typedef struct DebugPoint
{
    Vector3 position;
    Color color;
} DebugPoint;


