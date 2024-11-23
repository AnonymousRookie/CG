#include <vector>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "camera.h"
#include "tr_gl.h"

Model* model = NULL;
const int width = 800;
const int height = 800;

Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f light_dir = Vec3f(1, 1, 1).normalize();
Vec3f up(0, 1, 0);

//Camera camera(eye, Vec3f(0, 1, 0), center - eye);

// (x, y, z, w) -> (x / w, y / w, z / w)
Vec3f m2v(Matrix m) {
    //return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
    return Vec3f(int(m[0][0] / m[3][0] + 0.5), int(m[1][0] / m[3][0] + 0.5), int(m[2][0] / m[3][0] + 0.5));
}

// (x, y, z) -> (x, y, z, 1)
Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.0f;
    return m;
}

struct GouraudShader : public IShader {
    Vec3f varying_intensity;  // written by vertex shader, read by fragment shader
    Vec2f varying_uv[3];
    virtual Vec3f vertex(int iface, int nthvert) {
        Vec3f v = model->vert(iface, nthvert);
        v = m2v(g_Viewport * g_Projection * g_ModeView * v2m(v));
        varying_intensity[nthvert] = std::max(0.0f, model->norm(iface, nthvert)*light_dir);
        varying_uv[nthvert] = model->uv(iface, nthvert);
        return v;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        float intensity = varying_intensity * bar;
        //color = TGAColor(255, 255, 255) * intensity;
        Vec2f uv = varying_uv[0] * bar.x + varying_uv[1] * bar.y + varying_uv[2] * bar.z;
        color = model->diffuse(uv) * intensity;
        return false;  // do not discard this pixel
    }
};

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);

    float* zbuffer = new float[width * height];

    g_ModeView = lookat(eye, center, up);
    projection(-1.0 / (eye - center).norm());
    g_Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    for (int i = width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    GouraudShader shader;
    for (int i = 0; i < model->nfaces(); ++i) {
        Vec3f screen_coords[3];
        for (int j = 0; j < 3; ++j) {
            Vec3f v = model->vert(i, j);
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, zbuffer, image, width);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete model;
    return 0;
}

