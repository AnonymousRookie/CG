// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

// 判断点是否在三角形内
static bool insideTriangle(int x, int y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]

    // A: _v[0], B: _v[1], C: _v[2]
    Eigen::Vector2f P(x, y);

    Eigen::Vector2f AB = _v[1].head(2) - _v[0].head(2);
    Eigen::Vector2f BC = _v[2].head(2) - _v[1].head(2);
    Eigen::Vector2f CA = _v[0].head(2) - _v[2].head(2);

    Eigen::Vector2f AP = P - _v[0].head(2);
    Eigen::Vector2f BP = P - _v[1].head(2);
    Eigen::Vector2f CP = P - _v[2].head(2);

    // 二维向量叉乘：(x1,y1)×(x2,y2) = x1*y2-x2*y1
    // 若值为正则(x2,y2)在(x1,y1)逆时针方向

    int cross1 = AB[0] * AP[1] - AP[0] * AB[1];
    int cross2 = BC[0] * BP[1] - BP[0] * BC[1];
    int cross3 = CA[0] * CP[1] - CP[0] * CA[1];

    return cross1 > 0 && cross2 > 0 && cross3 > 0;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    float box_x1 = std::min(std::min(v[0][0], v[1][0]), v[2][0]);
    float box_y1 = std::min(std::min(v[0][1], v[1][1]), v[2][1]);
    float box_x2 = std::max(std::max(v[0][0], v[1][0]), v[2][0]);
    float box_y2 = std::max(std::max(v[0][1], v[1][1]), v[2][1]);

    bool use_MSAA = true;

    if (!use_MSAA) {
        // iterate through the pixel and find if the current pixel is inside the triangle
        for (int x = box_x1; x <= box_x2; ++x) {
            for (int y = box_y1; y <= box_y2; ++y) {
                // 判断像素中心(x+0.5, y+0.5)是否位于三角形中
                float center_x = x + 0.5;
                float center_y = y + 0.5;
                if (insideTriangle(center_x, center_y, t.v)) {
                    // 获取像素中心点的深度
                    // If so, use the following code to get the interpolated z value.
                    auto[alpha, beta, gamma] = computeBarycentric2D(center_x, center_y, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;

                    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                    int buf_index = get_index(x, y);
                    if (z_interpolated < depth_buf[buf_index]) {
                        // 如果该点的深度 < 深度缓存中的深度, 则更新深度缓存并在屏幕上画出
                        depth_buf[buf_index] = z_interpolated;
                        Eigen::Vector3f point(x, y, z_interpolated);
                        Eigen::Vector3f color = t.getColor();
                        set_pixel(point, color);
                    }
                }
            }
        }
    }
    else {
        // 对每个像素进行2 * 2采样
        std::vector<Eigen::Vector2f> pos {
            {0.25, 0.25},
            {0.75, 0.25},
            {0.25, 0.75},
            {0.75, 0.75},
        };

        for (int x = box_x1; x <= box_x2; ++x) {
            for (int y = box_y1; y <= box_y2; ++y) {
                int count = 0;
                float min_depth = FLT_MAX; 
                for (int i = 0; i < pos.size(); ++i) {
                    float center_x = x + pos[i][0];
                    float center_y = y + pos[i][1];
                    if (insideTriangle(center_x, center_y, t.v)) {
                        auto[alpha, beta, gamma] = computeBarycentric2D(center_x, center_y, t.v);
                        float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                        float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                        z_interpolated *= w_reciprocal;

                        min_depth = std::min(min_depth, z_interpolated);
                        ++count;
                    }
                }

                if (count > 0) {
                    int buf_index = get_index(x, y);
                    if (min_depth < depth_buf[buf_index]) {
                        depth_buf[buf_index] = min_depth;
                        Eigen::Vector3f point(x, y, min_depth);
                        Eigen::Vector3f color = t.getColor() * count / pos.size();
                        set_pixel(point, color);
                    }
                }
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on