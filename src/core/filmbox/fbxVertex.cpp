#include "gr-base/core/filmbox/fbxVertex.hpp"

FbxVertex::FbxVertex(const Vector3 & position, const Vector3 & normal, const Vector2 & texCoord, int index) : position(position), normal(normal), texCoord(texCoord), index(index) {
    for (auto i=0;i<4;++i) {
        weight[i] = 0.0f;
        boneIndex[i] = -1;
    }
}
