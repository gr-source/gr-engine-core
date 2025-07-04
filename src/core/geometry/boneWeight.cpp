#include "gr-base/core/geometry/boneWeight.hpp"

BoneWeight::BoneWeight() {
    for (int i=0;i<4;++i) {
        m_weight[i] = 0.0f;
        m_index[i] = -1;
    }
}

template <>
void BoneWeight::insert(int idx, float value) {
    m_weight[idx >= 4 ? 0 : idx] = value;
}

template <>
void BoneWeight::insert(int idx, int value) {
    m_index[idx >= 4 ? 0 : idx] = value;
}
