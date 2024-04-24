#include "Shape.h"
#include <iostream>
bool phong = true;



Polygon::Polygon(std::vector<Eigen::Vector3f> vVertices, Material cMaterial) {
    m_vVertices = vVertices;
    m_cMaterial = cMaterial;


    m_vMin = vVertices[0];
    m_vMax = vVertices[0];
    for (auto& e : vVertices) {

        m_vMin = m_vMin.cwiseMin(e);
        m_vMax = m_vMax.cwiseMax(e);
    }

    m_vCenter = m_vMin + ((m_vMax - m_vMin) / 2.0);
    
}

Polygon::Polygon(std::vector<Eigen::Vector3f> vVertices, std::vector<Eigen::Vector3f> vNormals, Material cMaterial) {
    m_vVertices = vVertices;
    m_vNormals = vNormals;
    m_cMaterial = cMaterial;


    m_vMin = vVertices[0];
    m_vMax = vVertices[0];
    for (auto& e : vVertices) {

        m_vMin = m_vMin.cwiseMin(e);
        m_vMax = m_vMax.cwiseMax(e);
    }

    m_vCenter = m_vMin + ((m_vMax - m_vMin) / 2.0);

}

bool Polygon::Trace(ray_t& ray, float fMin, float fMax){
    auto n = (m_vVertices[1] - m_vVertices[0]).cross(m_vVertices[2] - m_vVertices[0]);
    //compute normal of polygon. NFF specifies first triangle will always match normal direction
    Eigen::Vector3f d = { ray.direction.x, ray.direction.y, ray.direction.z };
    Eigen::Vector3f e = { ray.origin.x, ray.origin.y, ray.origin.z };

    float tau = (m_vVertices[0] - e).dot(n) / d.dot(n);
    auto p = e + tau * d;

    int index0, index1 = 0;
    //Choose specific 2d plane to prevent simplifying to a line
    if (abs(n.z()) > abs(n.x()) && (abs(n.z()) > abs(n.y()))) {
        index0 = 0;
        index1 = 1;
    }
    else if (abs(n.y()) > abs(n.x())) {
        index0 = 0;
        index1 = 2;
    }
    else {
        index0 = 1;
        index1 = 2;
    }

    int intersections = 0;
    //Cast 2d ray out from point and count intersections with polygon
    for (int i = 0; i < m_vVertices.size(); i++) {
        Eigen::Vector3f a = m_vVertices[i];
        Eigen::Vector3f b = m_vVertices[(i + 1) % m_vVertices.size()];

        if (abs(a[index1] - b[index1]) > 10e-6) {
            float s = (p[index1] - b[index1]) / (a[index1] - b[index1]);

            if (s >= 0 && s <= 1) {
                float t2 = s * (a[index0] - b[index0]) + b[index0] - p[index0];
                if (t2 >= 0) {
                    intersections++;
                }
            }
        }
    }

    //Intersections of odd amount mean point is inside polygon and should be rendered
    if (intersections % 2 && tau < ray.hit && tau >= fMin && tau <= fMax) {

        ray.hit = tau;
        ray.material = m_cMaterial;
        ray.normal = GetNormal(e + ray.hit * d, true);
        return true;

    }
    return false;
}

Eigen::Vector3f Polygon::GetNormal(Eigen::Vector3f vPos, bool bPhong) const {
    if (bPhong) {
        if (m_vNormals.size()) {
            Eigen::Vector3f sum = { 0,0,0 };
            std::vector<float> weights(m_vVertices.size());
            for (int i = 0; i < m_vVertices.size(); i++) {
                //Calculate mean value coordinates
                int next = (i + 1) % m_vVertices.size();
                int prev = (((i - 1) == -1) ? m_vVertices.size() - 1 : (i - 1));

                float alpha1 = acos((m_vVertices[prev] - vPos).dot(m_vVertices[i] - vPos) / ((m_vVertices[prev] - vPos).norm() * (m_vVertices[i] - vPos).norm()));
                float alpha2 = acos((m_vVertices[i] - vPos).dot(m_vVertices[next] - vPos) / ((m_vVertices[i] - vPos).norm() * (m_vVertices[next] - vPos).norm()));

                weights[i] = (tan(-alpha1 * 0.5f) + tan(-alpha2 * 0.5f)) / -(m_vVertices[i] - vPos).norm();
            }
            
            for (int i = 0; i < m_vNormals.size(); i++) {
                sum += (weights[i]) * m_vNormals[i];
                //Adjust normals based on mean value coordinates

            }

            return -sum.normalized();
        }
    }
    return -(m_vVertices[1] - m_vVertices[0]).cross(m_vVertices[2] - m_vVertices[0]).normalized();
    
    
}

Sphere::Sphere(Eigen::Vector3f vCenter, float radius, Material cMaterial){

    m_vCenter = vCenter;
    m_fRadius = radius;
    m_cMaterial = cMaterial;

    m_vMin = { vCenter[0] - radius, vCenter[1] - radius, vCenter[2] - radius };
    m_vMax = { vCenter[0] + radius, vCenter[1] + radius, vCenter[2] + radius };
}

bool Sphere::Trace(ray_t& ray, float fMin, float fMax){
    Eigen::Vector3f d = { ray.direction.x, ray.direction.y, ray.direction.z };
    Eigen::Vector3f e = { ray.origin.x, ray.origin.y, ray.origin.z };
    Eigen::Vector3f c = m_vCenter;
    float r = m_fRadius;

    //Quadratic formula to solve enter and exit of sphere
    float ddotd = d.dot(d);
    float ddotec = d.dot(e - c);
    float root = sqrt(ddotec * ddotec - ddotd * ((e - c).dot(e - c) - r * r));
    float tau1 = (-ddotec + root) / ddotd;
    float tau2 = (-ddotec - root) / ddotd;

    if (tau1 < ray.hit && tau1 >= fMin && tau1 <= fMax) {

        //Hit triangle is closer to camera than previous, render it
        ray.hit = tau1;
        ray.material = m_cMaterial;
        ray.normal = GetNormal(e + ray.hit * d);
    }
    if (tau2 < ray.hit && tau2 >= fMin && tau2 <= fMax) {

        //Hit triangle is closer to camera than previous, render it
        ray.hit = tau2;
        ray.material = m_cMaterial;
        ray.normal = GetNormal(e + ray.hit * d);

    }
    return ray.hit == tau1 || ray.hit == tau2;
}



Eigen::Vector3f Sphere::GetNormal(Eigen::Vector3f vPos, bool bPhong) const{
    return (m_vCenter - vPos).normalized();
}



Eigen::Vector3f Shape::GetNormal(Eigen::Vector3f vPos, bool bPhong) const {
    return Eigen::Vector3f();
}

bool Shape::Trace(ray_t& ray, float fMin, float fMax){
    return false;
}

Material Shape::GetMaterial(void){
    return m_cMaterial;
}

Eigen::Vector3f Shape::GetCenter(void){
    return m_vCenter;
}

Eigen::Vector3f Shape::GetMin(void){
    return m_vMin;
}

Eigen::Vector3f Shape::GetMax(void){
    return m_vMax;
}

Triangle::Triangle(Eigen::Vector3f vVertOne, Eigen::Vector3f vVertTwo, Eigen::Vector3f vVertThree, Material cMaterial){
    m_vNormOne = { INFINITY, INFINITY, INFINITY };
    m_vVertOne = vVertOne;
    m_vVertTwo = vVertTwo;
    m_vVertThree = vVertThree;
    m_cMaterial = cMaterial;

    m_vCenter = (vVertOne + vVertTwo + vVertThree) / 3.0;

    m_vMin = vVertOne.cwiseMin(vVertTwo);
    m_vMin = m_vMin.cwiseMin(vVertThree);

    m_vMax = vVertOne.cwiseMax(vVertTwo);
    m_vMax = m_vMax.cwiseMax(vVertThree);

}

Triangle::Triangle(Eigen::Vector3f vVertOne, Eigen::Vector3f vVertTwo, Eigen::Vector3f vVertThree, Eigen::Vector3f vNormOne, Eigen::Vector3f vNormTwo, Eigen::Vector3f vNormThree, Material cMaterial) {
    m_vVertOne = vVertOne;
    m_vVertTwo = vVertTwo;
    m_vVertThree = vVertThree;

    m_vNormOne = vNormOne;
    m_vNormTwo = vNormTwo;
    m_vNormThree = vNormThree;

    m_cMaterial = cMaterial;

    m_vCenter = (vVertOne + vVertTwo + vVertThree) / 3.0;

    m_vMin = vVertOne.cwiseMin(vVertTwo);
    m_vMin = m_vMin.cwiseMin(vVertThree);

    m_vMax = vVertOne.cwiseMax(vVertTwo);
    m_vMax = m_vMax.cwiseMax(vVertThree);

}

bool Triangle::Trace(ray_t& ray, float fMin, float fMax) {
    const Eigen::Vector3f vEdgeOne = m_vVertTwo - m_vVertOne;
    const Eigen::Vector3f vEdgeTwo = m_vVertThree - m_vVertOne;
    const Eigen::Vector3f vDir = { ray.direction.x, ray.direction.y, ray.direction.z };

    const Eigen::Vector3f h = vDir.cross(vEdgeTwo);
    const float a = vEdgeOne.dot(h);

    if (-10e-6 < a && a < 10e-6) return false;

    

    const float f = 1.0 / a;
    const Eigen::Vector3f s = Eigen::Vector3f(ray.origin.x, ray.origin.y, ray.origin.z) - m_vVertOne;
    

    const float u = f * s.dot(h);

    if (u < -10e-4 || u > 1 + 10e-4) return false;

    const Eigen::Vector3f q = s.cross(vEdgeOne);
    const float v = f * vDir.dot(q);

    if (v < -10e-4 || u + v > 1 + 10e-4) return false;

    const float tau = f * vEdgeTwo.dot(q);

    

    if (tau < ray.hit && tau >= fMin && tau <= fMax) {
        //Hit triangle is closer to camera than previous, render it
        ray.hit = tau;
        ray.material = m_cMaterial;
        ray.normal = GetNormal({ 1 - (u + v), u, v }, true);
        return true;
    }
    return false;
}

Eigen::Vector3f Triangle::GetNormal(Eigen::Vector3f vPos, bool bPhong) const {

    if (bPhong) {
        if (m_vNormOne[0] != INFINITY) {
            //interpolate coordinates based on barycentric coords
            return -(vPos[0] * m_vNormOne + vPos[1] * m_vNormTwo + vPos[2] * m_vNormThree).normalized();
        }
    }
    return -(m_vVertTwo - m_vVertOne).cross(m_vVertThree - m_vVertOne).normalized();
}
