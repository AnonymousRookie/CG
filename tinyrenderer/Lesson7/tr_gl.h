#pragma once

#include "tgaimage.h"
#include "geometry.h"

const float depth = 2000;

extern Matrix g_ModeView;
extern Matrix g_Viewport;
extern Matrix g_Projection;

// ��[-1,1]^2�еĵ�任����(x,y)Ϊԭ�㣬w,hΪ����ߵ���Ļ������
Matrix viewport(int x, int y, int w, int h);

void projection(float coeff = 0.0f);

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
    virtual ~IShader();
    virtual Vec3f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void triangle(Vec3f *pts, IShader &shader, float *zbuffer, TGAImage &image, int width);
