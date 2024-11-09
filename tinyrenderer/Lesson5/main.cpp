#include <vector>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "camera.h"

const int depth = 255;
const int width = 800;
const int height = 800;

Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f light_dir = Vec3f(1, -1, 1).normalize();

Camera camera(eye, Vec3f(0, 1, 0), center - eye);

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

// 视图变换矩阵
Matrix viewMatrix() 
{
    // M = T * R, 先旋转再平移
    Matrix view = Matrix::identity(4);
    Matrix r_inverse = Matrix::identity(4);  // R的逆矩阵
    Matrix t_inverse = Matrix::identity(4);  // T的逆矩阵

    for (int i = 0; i < 3; ++i)
    {
        r_inverse[0][i] = camera.right_[i];
        r_inverse[1][i] = camera.up_[i];
        r_inverse[2][i] = -camera.front_[i];
        t_inverse[i][3] = -camera.position_[i];
    }

    view = r_inverse * t_inverse;
    return view;
}

void triangle(Vec3f t0, Vec3f t1, Vec3f t2, float ity0, float ity1, float ity2, TGAImage &image, float *zbuffer) {
    if (t0.y == t1.y && t0.y == t2.y) return;
    if (t0.y > t1.y) { std::swap(t0, t1); std::swap(ity0, ity1); }
    if (t0.y > t2.y) { std::swap(t0, t2); std::swap(ity0, ity2); }
    if (t1.y > t2.y) { std::swap(t1, t2); std::swap(ity1, ity2); }

    int total_height = t2.y - t0.y;
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float)i / total_height;
        float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
        Vec3f A = t0 + Vec3f(t2 - t0)*alpha;
        Vec3f B = second_half ? t1 + Vec3f(t2 - t1)*beta : t0 + Vec3f(t1 - t0)*beta;
        float ityA = ity0 + (ity2 - ity0)*alpha;
        float ityB = second_half ? ity1 + (ity2 - ity1)*beta : ity0 + (ity1 - ity0)*beta;
        if (A.x > B.x) { std::swap(A, B); std::swap(ityA, ityB); }
        for (int j = A.x; j <= B.x; j++) {
            float phi = B.x == A.x ? 1. : (float)(j - A.x) / (B.x - A.x);
            Vec3f P = Vec3f(A) + Vec3f(B - A)*phi;
            float ityP = ityA + (ityB - ityA)*phi;
            int idx = P.x + P.y*width;
            if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                image.set(P.x, P.y, TGAColor(255, 255, 255) * ityP);
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

    TGAImage image(width, height, TGAImage::RGB);

    float* zbuffer = new float[width * height];

    //Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));
    Matrix modelView = viewMatrix();

    // 将投影平面设为z=0
    // 摄像机坐标(0,0,c), 待投影点P(x,y,z), 投影点P(x',y',z')
    // 由相似三角形: x' = x / (1 - z / c), y' = y / (1 - z / c)
    Matrix projection = Matrix::identity(4);
    projection[3][2] = -1.0 / (eye - center).norm();
    Matrix viewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    for (int i = width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    for (int i = 0; i < model->nfaces(); ++i) {
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        float intensity[3];
        for (int j = 0; j < 3; ++j) {
            Vec3f v = model->vert(i, j);
            screen_coords[j] = m2v(viewPort * projection * modelView * v2m(v));
            world_coords[j] = v;
            intensity[j] = model->norm(i, j)*light_dir;
        }
        triangle(screen_coords[0], screen_coords[1], screen_coords[2], intensity[0], intensity[1], intensity[2], image, zbuffer);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");

    { 
        // dump z-buffer (debugging purposes only)
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                zbimage.set(i, j, TGAColor(zbuffer[i + j*width]));
            }
        }
        zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        zbimage.write_tga_file("zbuffer.tga");
    }

    delete model;
    return 0;
}

