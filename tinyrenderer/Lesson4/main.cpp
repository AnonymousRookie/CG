#include <vector>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0, 255, 0, 255);

const int depth = 255;

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

// 将[-1,1]^2中的点变换到以(x,y)为原点，w,h为宽与高的屏幕区域内
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.0f;
    m[1][3] = y+h/2.0f;
    m[2][3] = depth/2.0f;

    m[0][0] = w/2.0f;
    m[1][1] = h/2.0f;
    m[2][2] = depth/2.0f;
    return m;
}

// 平移
// 1 0 0 vx
// 0 1 0 vy
// 0 0 1 vz
// 0 0 0 1
Matrix translation(Vec3f v) {
    Matrix Tr = Matrix::identity(4);
    Tr[0][3] = v.x;
    Tr[1][3] = v.y;
    Tr[2][3] = v.z;
    return Tr;
}

// 缩放
// f 0 0 0
// 0 f 0 0
// 0 0 f 0
// 0 0 0 1
Matrix zoom(float factor) {
    Matrix Z = Matrix::identity(4);
    Z[0][0] = Z[1][1] = Z[2][2] = factor;
    return Z;
}

// 绕X轴旋转
// 1  0   0    0
// 0  cos -sin 0
// 0  sin cos  0
// 0  0   0    1
Matrix rotation_x(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[1][1] = R[2][2] = cosangle;
    R[1][2] = -sinangle;
    R[2][1] =  sinangle;
    return R;
}

// 绕Y轴旋转
// cos   0   sin  0
// 0     0   0    0
// -sin  0   cos  0
// 0     0   0    1

Matrix rotation_y(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[0][0] = R[2][2] = cosangle;
    R[0][2] =  sinangle;
    R[2][0] = -sinangle;
    return R;
}

// 绕Z轴旋转
// cos - sin 0  0
// sin  cos  0  0
// 0    0    0  0
// 0    0    0  1
Matrix rotation_z(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[0][0] = R[1][1] = cosangle;
    R[0][1] = -sinangle;
    R[1][0] =  sinangle;
    return R;
}

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

    float* zbuffer = new float[width * height];

    Vec3f light_dir(0, 0, -1);
    Vec3f camera(0, 0, 3);

    // 将投影平面设为z=0
    // 摄像机坐标(0,0,c), 待投影点P(x,y,z), 投影点P(x',y',z')
    // 由相似三角形: x' = x / (1 - z / c), y' = y / (1 - z / c)
    Matrix projection = Matrix::identity(4);
    projection[3][2] = -1.0 / camera.z;
    Matrix viewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    for (int i = width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    for (int i = 0; i < model->nfaces(); ++i) {
        Vec3f screen_coords[3];
        Vec3f world_corrds[3];
        for (int j = 0; j < 3; ++j) {
            Vec3f v = model->vert(i, j);
            //screen_coords[j] = world2screen(v, width, height);
            screen_coords[j] = m2v(viewPort * projection * v2m(v));
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
    image.write_tga_file("output.tga");

    delete model;
    return 0;
}


