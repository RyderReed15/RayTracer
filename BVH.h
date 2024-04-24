#pragma once

#ifndef BVH_H
#define BVH_H

#include "Shape.h"
#include <Eigen/Dense>

__declspec(align(32)) struct BVHNode {
    float3 vMin, vMax;
    uint32_t leftFirst, primCount;
    bool isLeaf() { return primCount > 0; }
};

class BVH {
public:
    BVH(std::vector<Shape*>& vShapes);
    ~BVH();

    void IntersectBVH(ray_t& ray, uint32_t uIndex, float min = 0, float max = 10e12);
private:
    void UpdateNodeBounds(uint32_t uIndex);
    void Subdivide(uint32_t uIndex);
    bool IntersectSlab(ray_t& ray, float3 vMin, float3 vMax);

    BVHNode* m_aBVHNodes;
    uint32_t* m_aIndices;
    std::vector<Shape*> m_vShapes;
    uint32_t m_uNodeCount;
};

void _mm_store3_ps(float3& f, const __m128& m); 


#endif // !BVH_H