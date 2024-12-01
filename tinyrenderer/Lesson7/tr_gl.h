#pragma once

#include "tgaimage.h"
#include "geometry.h"

const float depth = 2000;

extern Matrix g_ModeView;
extern Matrix g_Viewport;
extern Matrix g_Projection;

// 将[-1,1]^2中的点变换到以(x,y)为原点，w,h为宽与高的屏幕区域内
Matrix viewport(int x, int y, int w, int h);

void projection(float coeff = 0.0f);

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
    virtual ~IShader();
    virtual Vec3f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void triangle(Vec3f *pts, IShader &shader, float *zbuffer, TGAImage &image, int width);
