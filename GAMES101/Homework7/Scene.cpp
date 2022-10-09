//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

// 求一条光线与场景的交点
Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

// 在场景的所有光源上按面积 uniform 地 sample 一个点，并计算该 sample 的概率密度
void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Vector3f L_dir(0, 0, 0);
    Vector3f L_indir(0, 0, 0);

    Intersection inter = intersect(ray);
    
    // 如果从像素发出的ray没有打到物体(即没有交点), 直接返回(0, 0, 0)
    if (!inter.happened) {
        return Vector3f(0, 0, 0);
    }

    // 如果从像素发出的ray打到光源, 返回光源信息
    if (inter.m->hasEmission()) {
        if (0 == depth) {
            return inter.m->getEmission();
        }
        else {
            return Vector3f(0, 0, 0);
        }
    }

    // 如果从像素发出的ray打到物体

    // 1. Contribution from the light source
    // 随机sample灯光, 用该sample的结果判断射线是否击中光源
    Intersection lightInter;
    float pdf_light = 0.0f;
    sampleLight(lightInter, pdf_light);

    auto& N = inter.normal;        // 物体表面的法线
    auto& NN = lightInter.normal;  // 灯光表面的法线

    auto& objPos = inter.coords;
    auto& lightPos = lightInter.coords;

    auto obj2Light = lightPos - objPos;
    auto obj2LightDir = obj2Light.normalized();
    float obj2LightDistance = obj2Light.norm();  // 物体到光源的距离

    // 再次发出一条光线, 判断物体与光源中间是否有遮挡
    Ray light(objPos, obj2LightDir);
    Intersection obj2LightInter = intersect(light);

    // 如果反射光线击中光源
    if (obj2LightInter.happened && (obj2LightInter.coords - lightPos).norm() < 1e-2)
    {
        Vector3f f_r = inter.m->eval(ray.direction, obj2LightDir, N);
        L_dir = lightInter.emit * f_r * dotProduct(obj2LightDir, N) * dotProduct(-obj2LightDir, NN) / std::pow(obj2LightDistance, 2) / pdf_light;
    }

    // 2. Contribution from other reflectors
    float P_RR = get_random_float();
    if (P_RR < RussianRoulette)
    {
        // 按照该材质的性质，给定入射方向与法向量，用某种分布采样一个出射方向
        Vector3f outDir = inter.m->sample(ray.direction, N).normalized();
        Ray outRay(objPos, outDir);
        Intersection outInter = intersect(outRay);
        // outRay打到另一个物体
        if (outInter.happened && !outInter.m->hasEmission()) 
        {
            // 给定一对入射、出射方向与法向量，计算 sample 方法得到该出射方向的概率密度
            float pdf = inter.m->pdf(ray.direction, outDir, N);
            Vector3f f_r = inter.m->eval(ray.direction, outDir, N);
            L_indir = castRay(outRay, depth+1) * f_r * dotProduct(outDir, N) / pdf / RussianRoulette;
        }
    }

    return L_dir + L_indir;
}
