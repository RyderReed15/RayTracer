// RayTracer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// 435 Project 1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include "NFFFile.h"
#include "Shape.h"
#include <chrono>
#include "BVH.h"
#include <thread>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <math.h>




void RenderNFF(const NFFFile& file, const char* outputFile);

int main(int argc, char* argv[]) {

    if (argc < 3) {
        std::cerr << "Not enough arguments provided" << std::endl;
        return 0;
    }
    

    std::cout << "Rendering " << argv[argc - 2] << " ..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    NFFFile file(argv[argc - 2]);

    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << "Parsed " << argv[argc - 2] << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "ms" << std::endl;

    RenderNFF(file, argv[argc - 1]);
}

Color GetColor(ray_t& ray, Color cBackground, BVH& BVHTree,std::vector<Light> vLights, uint32_t steps) {
    if (ray.hit == INFINITY) return { cBackground.r, cBackground.g, cBackground.b };

    Eigen::Vector3f vOrigin = { ray.origin.x, ray.origin.y, ray.origin.z };
    Eigen::Vector3f vDir = { ray.direction.x, ray.direction.y, ray.direction.z };
    __m128 color4 = _mm_set_ps1(0);

    Eigen::Vector3f hitLocation = vOrigin + ray.hit * vDir;


    Eigen::Vector3f N = { ray.normal.x, ray.normal.y, ray.normal.z };
    

    __m128 lightIntensity4 = _mm_rsqrt_ps(_mm_set_ps1(vLights.size()));
    __m128 ks4 = _mm_set_ps1(ray.material.Ks);

    for (auto& light : vLights) {

        ray_t lightRay;
        //There should be a negative infront of w.norm but it doesnt work if i do that and i dont know why?
        // -wd + uu + vv
        lightRay.direction = light.pos - hitLocation;
        lightRay.origin = hitLocation;
        lightRay.hit = INFINITY;

        BVHTree.IntersectBVH(lightRay, 0, 10e-4);

        //Shadow check
        if (lightRay.hit < (light.pos - hitLocation).norm()) continue;

        Eigen::Vector3f L = (hitLocation - light.pos).normalized();
        Eigen::Vector3f H = (L + vDir).normalized();

        //std::cout << N << L << std::endl;

        double diffuse = std::max(0.0f, N.dot(L));
        double specular = pow(std::max(0.0f, N.dot(H)), ray.material.shine);
        color4 = _mm_add_ps(color4, _mm_mul_ps(_mm_add_ps(_mm_mul_ps(_mm_set_ps1(ray.material.Kd), _mm_mul_ps(_mm_set_ps(0, ray.material.color.b, ray.material.color.g, ray.material.color.r), _mm_set_ps1(diffuse))), _mm_mul_ps(ks4, _mm_set_ps1(specular))), lightIntensity4));
        /*outColor[0] += (ray.material.Kd * ray.material.color.r * diffuse + ray.material.Ks * specular) * lightIntensity;
        outColor[1] += (ray.material.Kd * ray.material.color.g * diffuse + ray.material.Ks * specular) * lightIntensity;
        outColor[2] += (ray.material.Kd * ray.material.color.b * diffuse + ray.material.Ks * specular) * lightIntensity;*/
    }


    

    //Calculate bounces

    for (int i = 0; i < steps && ks4.m128_f32[0] > 0; i++) {
        ray.origin = hitLocation;
        ray.direction = vDir - 2 * (vDir.dot(N)) * N;
        vDir = { ray.direction.x, ray.direction.y, ray.direction.z };
        ray.hit = INFINITY;

        BVHTree.IntersectBVH(ray, 0, 10e-4);

        if (ray.hit == INFINITY) {
            //reflection of the sky
            color4 = _mm_add_ps(color4, _mm_mul_ps(_mm_set_ps(0, cBackground.b, cBackground.g, cBackground.r), ks4));
            break;
        }

        N = { ray.normal.x, ray.normal.y, ray.normal.z };

        for (auto& light : vLights) {

            ray_t lightRay;
            //There should be a negative infront of w.norm but it doesnt work if i do that and i dont know why?
            // -wd + uu + vv
            lightRay.direction = light.pos - hitLocation;
            lightRay.origin = hitLocation;
            lightRay.hit = INFINITY;

            BVHTree.IntersectBVH(lightRay, 0, 10e-4);

            if (lightRay.hit < (light.pos - hitLocation).norm()) continue;

            Eigen::Vector3f L = (hitLocation - light.pos).normalized();
            Eigen::Vector3f H = (L + vDir).normalized();

            double diffuse = std::max(0.0f, N.dot(L));
            double specular = pow(std::max(0.0f, N.dot(H)), ray.material.shine);
            color4 = _mm_add_ps(color4, _mm_mul_ps(ks4, _mm_mul_ps(_mm_add_ps(_mm_mul_ps(_mm_set_ps1(ray.material.Kd), _mm_mul_ps(_mm_set_ps(0, ray.material.color.b, ray.material.color.g, ray.material.color.r), _mm_set_ps1(diffuse))), _mm_mul_ps(_mm_set_ps1(ray.material.Ks), _mm_set_ps1(specular))), lightIntensity4)));

            /*outColor[0] += (ray.material.Kd * ray.material.color.r * diffuse + ray.material.Ks * specular) * lightIntensity * ks;
            outColor[1] += (ray.material.Kd * ray.material.color.g * diffuse + ray.material.Ks * specular) * lightIntensity * ks;
            outColor[2] += (ray.material.Kd * ray.material.color.b * diffuse + ray.material.Ks * specular) * lightIntensity * ks;*/

            //reflection should be amount reflected times previous amount reflected and keep getting smaller
            ks4 = _mm_mul_ps(ks4, _mm_set_ps1(ray.material.Ks));
        }
    }
    
    color4 = _mm_max_ps(_mm_min_ps(color4, _mm_set_ps1(1.0f)), _mm_set_ps1(0.0f));
    Color outColor; _mm_store3_ps(*(float3*)&outColor, color4);

    return outColor;
}


void TraceThread(int start, int finish, char***& pixels, BVH& BVHTree, const NFFFile& file) {

    Viewpoint fileView = file.GetViewpoint();
    Color cBackground = file.GetBackground();
    std::vector<Light> vLights = file.GetLights();

    Eigen::Vector3f w = fileView.at - fileView.from;
    Eigen::Vector3f u = fileView.up.cross(w);
    Eigen::Vector3f v = w.cross(u);

    float sizeHelper = tan(fileView.angle / 2 * M_PI / 180);
    //Setup view bounds
    float l = sizeHelper;
    float r = -sizeHelper;
    float b = sizeHelper;
    float t = -sizeHelper;

    w.normalize();
    u.normalize();
    v.normalize();

    //For each pixel cast a ray and check if it hits any objects
    for (int x = start; x < finish; x++) {
        for (int y = 0; y < fileView.yres; y++) {
            Color outColor = { 0,0,0 };




            ray_t ray{};
            //There should be a negative infront of w.norm but it doesnt work if i do that and i dont know why?
            // -wd + uu + vv
            ray.direction = w.norm() * w + (l + (r - l) * (y + .5) / fileView.yres) * u + (b + (t - b) * (x + .5) / fileView.xres) * v;
            ray.origin = fileView.from;
            ray.hit = INFINITY;
            ray.material = { cBackground.r, cBackground.g, cBackground.b };
            ray.normal = { 1,1,1 };

            BVHTree.IntersectBVH(ray, 0);

            outColor = GetColor(ray, cBackground, BVHTree, vLights, 5);
            //outColor = { ray.material.color.r, ray.material.color.g, ray.material.color.b };

            pixels[x][y][0] = outColor[0] * 255;
            pixels[x][y][1] = outColor[1] * 255;
            pixels[x][y][2] = outColor[2] * 255;
        }
    }
}


void RenderNFF(const NFFFile& file, const char* outputFile) {
    Viewpoint fileView = file.GetViewpoint();

    char*** pixels = new char** [fileView.xres];

    for (int x = 0; x < fileView.xres; x++) {
        pixels[x] = new char* [fileView.yres];
        for (int y = 0; y < fileView.yres; y++) {
            pixels[x][y] = new char[3];
        }
    }



    std::vector<Shape*> vShapes = file.GetShapes();

    auto start = std::chrono::high_resolution_clock::now();

    BVH BVHTree = BVH(vShapes);

    auto stop = std::chrono::high_resolution_clock::now();

    std::cout << "Built BVH tree in " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "ms" << std::endl;

    start = std::chrono::high_resolution_clock::now();

    

    

    //Multithread to improve speed
    const unsigned int threadCount = std::thread::hardware_concurrency() * 4;
    std::vector<std::thread>  threads;

    for (int i = 0; i < threadCount; i++) {
        threads.push_back(std::thread(TraceThread, fileView.xres * i / threadCount, fileView.xres * (i + 1) / threadCount, std::ref(pixels), std::ref(BVHTree), std::ref(file)));

    }
    for (int i = 0; i < threadCount; i++) {
        threads[i].join();
    }


    

    stop = std::chrono::high_resolution_clock::now();

    std::cout << "Rendered in " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << "ms" << std::endl;

    std::ofstream fileOut;
    fileOut.open(outputFile, std::ios::out | std::ios::binary);
    fileOut << "P6\n" << fileView.yres << " " << fileView.xres << "\n255\n";

    for (int x = 0; x < fileView.xres; x++) {
        for (int y = 0; y < fileView.yres; y++) {

            fileOut << pixels[x][y][0] << pixels[x][y][1] << pixels[x][y][2];
        }
    }

    fileOut.close();


    for (int x = 0; x < fileView.xres; x++) {
        for (int y = 0; y < fileView.yres; y++) {
            delete[] pixels[x][y];
        }
        delete[] pixels[x];
    }
    delete[] pixels;
}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
