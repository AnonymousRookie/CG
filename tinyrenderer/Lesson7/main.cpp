#include <vector>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "camera.h"
#include "tr_gl.h"

Model* model = NULL;
float* shadowbuffer = NULL;

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
    Vec2f uv[3];
    Matrix varying_tri = Matrix(3, 3);
    Matrix uniform_Mshadow = Matrix(3, 3); // transform framebuffer screen coordinates to shadowbuffer screen coordinates

    virtual Vec3f vertex(int iface, int nthvert) {
        Vec3f gl_Vertex = model->vert(iface, nthvert);
        gl_Vertex = m2v(g_Viewport * g_Projection * g_ModeView * v2m(gl_Vertex));
        varying_intensity[nthvert] = std::max(0.0f, model->norm(iface, nthvert)*light_dir);
        uv[nthvert] = model->uv(iface, nthvert);
        varying_tri.setColVal(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        float intensity = varying_intensity * bar;

        // 当前绘制的像素的屏幕坐标
        Vec3f p = varying_tri * bar;

        // 转换到shadowbuffer屏幕空间
        p = m2v(uniform_Mshadow * v2m(p));

        int idx = int(p.x + p.y*width); // index in the shadowbuffer array
        float shadow = .3 + .7*(shadowbuffer[idx] < p[2] + 43.34); // magic coeff to avoid z-fighting

        Vec2f uvP = uv[0] * bar.x + uv[1] * bar.y + uv[2] * bar.z;
        color = model->diffuse(uvP) * intensity * shadow;

        return false;  // do not discard this pixel
    }
};

struct DepthShader : public IShader {
    Matrix varying_tri = Matrix(3, 3);

    DepthShader() {}

    virtual Vec3f vertex(int iface, int nthvert) {
        Vec3f gl_Vertex = model->vert(iface, nthvert);
        gl_Vertex = m2v(g_Viewport * g_Projection * g_ModeView * v2m(gl_Vertex));
        varying_tri.setColVal(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        Vec3f p = Vec3f(varying_tri[0][0] * bar[0] + varying_tri[0][1] * bar[1] + varying_tri[0][2] * bar[2],
            varying_tri[1][0] * bar[0] + varying_tri[1][1] * bar[1] + varying_tri[1][2] * bar[2],
            varying_tri[2][0] * bar[0] + varying_tri[2][1] * bar[1] + varying_tri[2][2] * bar[2]);

        color = TGAColor(255, 255, 255) * (p.z / depth);

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

    shadowbuffer = new float[width * height];
    float* zbuffer = new float[width * height];

    for (int i = width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());
    for (int i = width*height; i--; shadowbuffer[i] = -std::numeric_limits<float>::max());

    {
        g_ModeView = lookat(light_dir, center, up);
        projection(0);
        g_Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

        TGAImage image(width, height, TGAImage::RGB);
        DepthShader shader;
        for (int i = 0; i < model->nfaces(); ++i) {
            Vec3f screen_coords[3];
            for (int j = 0; j < 3; ++j) {
                screen_coords[j] = shader.vertex(i, j);
            }
            triangle(screen_coords, shader, shadowbuffer, image, width);
        }

        image.flip_vertically();
        image.write_tga_file("depth.tga");
    }

    Matrix M = g_Viewport * g_Projection * g_ModeView;

    {
        g_ModeView = lookat(eye, center, up);
        projection(-1.0 / (eye - center).norm());
        g_Viewport = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

        TGAImage image(width, height, TGAImage::RGB);
        GouraudShader shader;
        shader.uniform_Mshadow = M*(g_Viewport * g_Projection * g_ModeView).inverse();
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
    }

    delete model;
    return 0;
}
