//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
class Object;
class Sphere;

struct Intersection
{
    Intersection(){
        happened=false;
        coords=Vector3f();
        normal=Vector3f();
        distance= std::numeric_limits<double>::max();
        obj =nullptr;
        m=nullptr;
    }
    bool happened;   // 是否相交
    Vector3f coords; // 交点坐标
    Vector3f normal; // 交点所在平面法线
    double distance; // 光线起点到交点的距离
    Object* obj;     // 交点所在物体的物体类型
    Material* m;     // 交点所在物体的材料类型
};
#endif //RAYTRACING_INTERSECTION_H
