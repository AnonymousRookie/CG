#pragma once

#include "geometry.h"


class Camera
{
public:
    Camera(Vec3f position, Vec3f worldUp, Vec3f front);
    ~Camera();

public:
    Vec3f position_;  // 相机在世界坐标系中的位置
    Vec3f worldUp_;   // 辅助向量, 一般设置为(0, 1, 0), 即Y轴的正向
    Vec3f front_;     // 从摄像机指向观察点(v轴负方向)
    Vec3f up_;        // u轴正方向
    Vec3f right_;     // r轴正方向
};

