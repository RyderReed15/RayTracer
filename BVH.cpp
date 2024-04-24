
#include "BVH.h"
#include <iostream>

void _mm_store3_ps(float3& f, const __m128& m) {
    _mm_storel_pi((__m64*) & f, m); f.z = m.m128_f32[2];
}


BVH::BVH(std::vector<Shape*>& vShapes){
    m_aBVHNodes = new BVHNode[vShapes.size() * 2 - 1];
    m_aIndices = new uint32_t[vShapes.size()];
    m_vShapes = vShapes;
    m_uNodeCount = 1;

    for (int i = 0; i < vShapes.size(); i++) m_aIndices[i] = i;

    BVHNode& root = m_aBVHNodes[0];
    root.leftFirst = 0; root.primCount = vShapes.size();
    UpdateNodeBounds(0);
    Subdivide(0);
}

BVH::~BVH(){
    delete[] m_aBVHNodes;
    delete[] m_aIndices;
}

void BVH::IntersectBVH(ray_t& ray, uint32_t uIndex, float min, float max) {
    BVHNode& node = m_aBVHNodes[uIndex];
    if (!IntersectSlab(ray, node.vMin, node.vMax)) return;
    if (node.isLeaf())
    {
        for (uint32_t i = 0; i < node.primCount; i++) {
            m_vShapes[m_aIndices[node.leftFirst + i]]->Trace(ray, min, max);
        }
            
            //IntersectTri(ray, tri[triIdx[node.leftFirst + i]]);
    }
    else
    {
        IntersectBVH(ray, node.leftFirst, min, max);
        IntersectBVH(ray, node.leftFirst + 1, min, max);
    }
}



void BVH::UpdateNodeBounds(uint32_t uIndex){
    BVHNode& node = m_aBVHNodes[uIndex];

    __m128 min4 = _mm_set_ps1(INFINITY);
    __m128 max4 = _mm_set_ps1(-INFINITY);

    for (uint32_t first = node.leftFirst, i = 0; i < node.primCount; i++) {
        auto vMin = m_vShapes[m_aIndices[first + i]]->GetMin();
        auto vMax = m_vShapes[m_aIndices[first + i]]->GetMax();
        __m128 a4 = _mm_set_ps(0, vMin[2], vMin[1], vMin[0]);
        __m128 b4 = _mm_set_ps(0, vMax[2], vMax[1], vMax[0]);

        max4 = _mm_max_ps(max4, b4);
        min4 = _mm_min_ps(min4, a4);
    }

    _mm_store3_ps(node.vMax, max4);
    _mm_store3_ps(node.vMin, min4);
}

void BVH::Subdivide(uint32_t uIndex){
    BVHNode& node = m_aBVHNodes[uIndex];
    if (node.primCount <= 1) {
        return;
    }


    __m128 min4 = _mm_set_ps1(INFINITY);
    __m128 max4 = _mm_set_ps1(-INFINITY);

    //temporary hack to ensure a significantly large object does not mess up stuff
    for (unsigned int first = node.leftFirst, i = 0; i < node.primCount; i++) {

        auto center = m_vShapes[m_aIndices[first + i]]->GetCenter();
        __m128 c4 = _mm_set_ps(0, center[2], center[1], center[0]);

        max4 = _mm_max_ps(max4, c4);
        min4 = _mm_min_ps(min4, c4);
    }

    float3 vExtent;
    _mm_store3_ps(vExtent, _mm_sub_ps(max4, min4));
    


    //Change to select axis of most variance
    int axis = 0;
    if (vExtent[1] > vExtent[0]) axis = 1;
    if (vExtent[2] > vExtent[axis]) axis = 2;

    float splitPos = min4.m128_f32[axis] + vExtent[axis] * .5f;

    int i = node.leftFirst;
    int j = i + node.primCount - 1;
    while (i <= j) {
        if (m_vShapes[m_aIndices[i]]->GetCenter()[axis] <= splitPos) i++;
        else {
            std::swap(m_aIndices[i], m_aIndices[j--]);
        }
    }
    
    int leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.primCount) return;

    int leftChildIdx = m_uNodeCount++;
    int rightChildIdx = m_uNodeCount++;
    m_aBVHNodes[leftChildIdx].leftFirst = node.leftFirst;
    m_aBVHNodes[leftChildIdx].primCount = leftCount;
    m_aBVHNodes[rightChildIdx].leftFirst = i;
    m_aBVHNodes[rightChildIdx].primCount = node.primCount - leftCount;
    node.leftFirst = leftChildIdx;
    node.primCount = 0;
    UpdateNodeBounds(leftChildIdx);
    UpdateNodeBounds(rightChildIdx);
    // recurse
    Subdivide(leftChildIdx);
    Subdivide(rightChildIdx);
}

bool BVH::IntersectSlab(ray_t& ray, float3 vMin, float3 vMax) {

    __m128 o4 = _mm_set_ps(0, ray.origin.z, ray.origin.y, ray.origin.x);
    __m128 d4 = _mm_rcp_ps(_mm_set_ps(0, ray.direction.z, ray.direction.y, ray.direction.x));
    __m128 t1 = _mm_mul_ps(_mm_sub_ps(_mm_set_ps(0, vMin.z, vMin.y, vMin.x), o4), d4);
    __m128 t2 = _mm_mul_ps(_mm_sub_ps(_mm_set_ps(0, vMax.z, vMax.y, vMax.x), o4), d4);
    __m128 vmax4 = _mm_max_ps(t1, t2), vmin4 = _mm_min_ps(t1, t2);
    float3 max; float3 min;
    _mm_store3_ps(max, vmax4);
    _mm_store3_ps(min, vmin4);

    float tmax = std::min(max[0], std::min(max[1], max[2]));
    float tmin = std::max(min[0], std::max(min[1], min[2]));


    return tmax >= tmin && tmin < ray.hit && tmax > 0;
}




