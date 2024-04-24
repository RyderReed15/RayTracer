#pragma once

#ifndef STRUCTS_H
#define STRUCTS_H
#include <Eigen/Dense>

struct Color;
struct float3;


struct float3 {
    float x, y, z;
    float& operator [](const int& a)
    {
        switch (a) {
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        default:
            throw std::out_of_range("");
        }
    }
    float3& operator=(const Eigen::Vector3f& v) {
        x = v[0];
        y = v[1];
        z = v[2];
        return *this;
    }
    
};

struct Viewpoint {
    Eigen::Vector3f from;
    Eigen::Vector3f at;
    Eigen::Vector3f up;

    float  angle;
    float  hither;
    int     xres;
    int     yres;
};

struct Color {
    float& operator [](const int& a)
    {
        switch (a) {
        case 0:
            return r;
        case 1:
            return g;
        case 2:
            return b;
        default:
            throw std::out_of_range("");
        }
    }
    
    float r;
    float g;
    float b;
        
};
struct Material {
    Color color;
    float Kd;
    float Ks;
    float shine;
    float t;
    float indexOfRefraction;
};

struct Light {
    Eigen::Vector3f pos;
    Color color;
};

struct ray_t {
    float3 origin;
    float3 direction;
    float hit;
    Material material;
    float3 normal;

};




#endif // !STRUCTS_H