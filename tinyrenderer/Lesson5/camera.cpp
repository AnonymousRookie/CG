#include "camera.h"

Camera::Camera(Vec3f position, Vec3f worldUp, Vec3f front)
{
    position_ = position;
    worldUp_ = worldUp;
    front_ = front.normalize();
    right_ = (front_ ^ worldUp_).normalize();
    up_ = (right_ ^ front_).normalize();
}

Camera::~Camera()
{
}

