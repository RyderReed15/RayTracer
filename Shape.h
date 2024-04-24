#pragma once

#ifndef SHAPE_H
#define SHAPE_H

#include "Structs.h"
#include <vector>


class Shape {
public:


    virtual Eigen::Vector3f GetNormal(Eigen::Vector3f vPos, bool bPhong = false) const;
    virtual bool Trace(ray_t& ray, float fMin = 0, float fMax = 10e12);

    Material GetMaterial(void);
    Eigen::Vector3f GetCenter(void);
    Eigen::Vector3f GetMin(void);
    Eigen::Vector3f GetMax(void);

protected:
    Material m_cMaterial;
    Eigen::Vector3f m_vCenter;
    Eigen::Vector3f m_vMin, m_vMax;
};

class Polygon : public Shape {
public:
    Polygon(std::vector<Eigen::Vector3f> vVertices, Material sMaterial);
    Polygon(std::vector<Eigen::Vector3f> vVertices, std::vector<Eigen::Vector3f> vNormals, Material sMaterial);

    bool Trace(ray_t& ray, float fMin = 0, float fMax = 10e12);
    Eigen::Vector3f GetNormal(Eigen::Vector3f vPos, bool bPhong = false) const;
private:

    std::vector<Eigen::Vector3f> m_vVertices;
    std::vector<Eigen::Vector3f> m_vNormals;
};

class Sphere : public Shape {
public:
    Sphere(Eigen::Vector3f vCenter, float radius, Material cColor);
    bool Trace(ray_t& ray, float fMin = 0, float fMax = 10e12);
    Eigen::Vector3f GetNormal(Eigen::Vector3f vPos, bool bPhong = false) const;

private:

    float m_fRadius;
};

class Triangle : public Shape {
public:
    Triangle(Eigen::Vector3f vVertOne, Eigen::Vector3f vVertTwo, Eigen::Vector3f vVertThree, Material cColor);
    Triangle(Eigen::Vector3f vVertOne, Eigen::Vector3f vVertTwo, Eigen::Vector3f vVertThree, Eigen::Vector3f vNormOne, Eigen::Vector3f vNormTwo, Eigen::Vector3f vNormThree, Material cColor);
    bool Trace(ray_t& ray, float fMin = 0, float fMax = 10e12);
    Eigen::Vector3f GetNormal(Eigen::Vector3f vPos, bool bPhong = false) const;

private:

    Eigen::Vector3f m_vVertOne;
    Eigen::Vector3f m_vVertTwo;
    Eigen::Vector3f m_vVertThree;

    Eigen::Vector3f m_vNormOne;
    Eigen::Vector3f m_vNormTwo;
    Eigen::Vector3f m_vNormThree;
};



#endif // !SHAPE_H
