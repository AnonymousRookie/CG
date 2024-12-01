#include <cmath>
#include <limits>
#include <algorithm>
#include <cstdlib>
#include "tr_gl.h"

Matrix g_ModeView;
Matrix g_Viewport;
Matrix g_Projection;

IShader::~IShader() {}

// 将[-1,1]^2中的点变换到以(x,y)为原点，w,h为宽与高的屏幕区域内
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.0f;
    m[1][3] = y + h / 2.0f;
    m[2][3] = depth / 2.0f;

    m[0][0] = w / 2.0f;
    m[1][1] = h / 2.0f;
    m[2][2] = depth / 2.0f;
    return m;
}

void projection(float coeff) {
    g_Projection = Matrix::identity(4);
    g_Projection[3][2] = coeff;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up^z).normalize();
    Vec3f y = (z^x).normalize();

    Matrix r_inverse = Matrix::identity(4);
    Matrix t_inverse = Matrix::identity(4);

    Matrix modelView = Matrix::identity(4);
    for (int i = 0; i < 3; i++) {
        r_inverse[0][i] = x[i];
        r_inverse[1][i] = y[i];
        r_inverse[2][i] = z[i];
        t_inverse[i][3] = -eye[i];
    }

    modelView = r_inverse * t_inverse;
    return modelView;
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

void triangle(Vec3f *pts, IShader &shader, float *zbuffer, TGAImage &image, int width) {
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
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
                continue;

            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += pts[i][2] * bc_screen[i];

            bool discard = shader.fragment(bc_screen, color);
            if (!discard) {
                if (zbuffer[int(P.x + P.y*width)] < P.z) {
                    zbuffer[int(P.x + P.y*width)] = P.z;
                    image.set(P.x, P.y, color);
                }
            }
        }
    }
}
