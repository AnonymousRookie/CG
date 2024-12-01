#pragma once

#include "geometry.h"


class Camera
{
public:
    Camera(Vec3f position, Vec3f worldUp, Vec3f front);
    ~Camera();

public:
    Vec3f position_;  // �������������ϵ�е�λ��
    Vec3f worldUp_;   // ��������, һ������Ϊ(0, 1, 0), ��Y�������
    Vec3f front_;     // �������ָ��۲��(v�Ḻ����)
    Vec3f up_;        // u��������
    Vec3f right_;     // r��������
};

