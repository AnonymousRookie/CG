#include <vector>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void line01(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    for (float t = 0.0; t < 1.0; t += 0.1) {
        int x = x0 * (1.0 - t) + x1 * t;
        int y = y0 * (1.0 - t) + y1 * t;
        image.set(x, y, color);
    }
}

void line02(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = y0 * (1.0 - t) + y1 * t;
        image.set(x, y, color);
    }
}

void line03(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1) < std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    
    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = y0 * (1.0 - t) + y1 * t;
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
    }
}

void line04(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1) < std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    float derror = std::abs(dy / float(dx));
    float error = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error += derror;
        if (error > 0.5) {
            y += (y1 > y0 ? 1 : -1);
            error -= 1;
        }
    }
}

void line05(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1) < std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    float derror2 = std::abs(dy) * 2;
    float error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

int main1(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);
    line05(13, 20, 80, 40, image, white);
    line05(20, 13, 40, 80, image, red);
    line05(80, 40, 13, 20, image, red);
    image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}

int main(int argc, char** argv) {
    Model* model = NULL;
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    const int width  = 200;
    const int height = 200;

    TGAImage image(width, height, TGAImage::RGB);

    for (int i = 0; i < model->nfaces(); ++i) {
        std::vector<int> face = model->face(i);
        for (int j = 0; j < 3; ++j) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j + 1) % 3]);
            int x0 = (v0.x + 1.0) * width / 2.0;
            int y0 = (v0.y + 1.0) * height / 2.0;
            int x1 = (v1.x + 1.0) * width / 2.0;
            int y1 = (v1.y + 1.0) * height / 2.0;

            line05(x0, y0, x1, y1, image, white);
        }
    }
    
    image.flip_vertically();
    image.write_tga_file("output.tga");

    delete model;
    return 0;
}

