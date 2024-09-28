#include <vector>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255, 0, 255);

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = s[0] ^ s[1];
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

Vec3f world2screen(Vec3f v, int width, int height) {
    return Vec3f(int((v.x + 1.0) * width / 2.0 + 0.5), int((v.y + 1.0) * height / 2.0 + 0.5), v.z);
}

void triangle01(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color, int width) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) 
                continue;
            P.z = 0;
            for (int i = 0; i < 3; i++) 
                P.z += pts[i][2] * bc_screen[i];

            if (zbuffer[int(P.x + P.y*width)] < P.z) {
                zbuffer[int(P.x + P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

void triangle02(Vec3f *pts, Vec2f* uvs, float *zbuffer, TGAImage &image, Model* model, int width, float intensity) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    Vec2f uvP;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;

            P.z = 0;
            for (int i = 0; i < 3; i++) {
                P.z += pts[i][2] * bc_screen[i];
            }

            if (zbuffer[int(P.x + P.y*width)] < P.z) {
                zbuffer[int(P.x + P.y*width)] = P.z;
                uvP = uvs[0] * bc_screen.x + uvs[1] * bc_screen.y + uvs[2] * bc_screen.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, TGAColor(intensity * color.r, intensity * color.g, intensity * color.b, 255));
            }
        }
    }
}

int main(int argc, char** argv) {
    Model* model = NULL;
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    const int width = 800;
    const int height = 800;

    TGAImage image(width, height, TGAImage::RGB);

    Vec3f light_dir(0, 0, -1);

    float* zbuffer = new float[width * height];
    for (int i = width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    for (int i = 0; i < model->nfaces(); ++i) {
        Vec3f screen_coords[3];
        Vec3f world_corrds[3];
        for (int j = 0; j < 3; ++j) {
            Vec3f v = model->vert(i, j);
            screen_coords[j] = world2screen(v, width, height);
            world_corrds[j] = v;
        }
        Vec3f n = (world_corrds[2] - world_corrds[0]) ^ (world_corrds[1] - world_corrds[0]);
        n.normalize();
        float intensity = n * light_dir;
        if (intensity > 0) {
            //triangle01(screen_coords, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255), width);
            Vec2f uv[3];
            for (int j = 0; j < 3; ++j) {
                uv[j] = model->uv(i, j);
            }
            triangle02(screen_coords, uv, zbuffer, image, model, width, intensity);
        }
    }

    image.flip_vertically();
    image.write_tga_file("framebuffer.tga");

    delete model;
    return 0;
}

