#pragma once

class BoneWeight {
public:
    BoneWeight();

    template <typename T>
    void insert(int idx, T value);
    
    float m_weight[4];
    int m_index[4];
};

