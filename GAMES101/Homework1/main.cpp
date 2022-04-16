#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

inline double deg_2_rad(float deg) {
    return deg / 180 * MY_PI;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 
                 0, 1, 0, -eye_pos[1], 
                 0, 0, 1, -eye_pos[2], 
                 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    double rad = deg_2_rad(rotation_angle);

    Eigen::Matrix4f translate;
    translate << std::cos(rad), -std::sin(rad), 0, 0,
                 std::sin(rad), std::cos(rad),  0, 0,
                 0,0,1,0,
                 0,0,0,1;

    model = translate * model;

    return model;
}

// 得到绕任意过原点的轴的旋转变换矩阵
Eigen::Matrix4f get_rotation(Eigen::Vector3f axis, float angle)
{
    // Rotation by angle rotation_angle around axis

    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();

    double nx = axis[0];
    double ny = axis[1];
    double nz = axis[2];

    Eigen::Matrix3f N;
    N << 0, -nz, ny,
         nz, 0, -nx,
         -ny, nx, 0;

    double rad = deg_2_rad(angle);

    // Rodrigues' Rotation Formula
    Eigen::Matrix3f R = std::cos(rad) * I + (1 - std::cos(rad)) * axis * axis.transpose() + std::sin(rad) * N;

    Eigen::Matrix4f rotation;
    rotation << R(0,0), R(0,1), R(0,2), 0,
                 R(1,0), R(1,1), R(1,2), 0,
                 R(2,0), R(2,1), R(2,2), 0,
                 0, 0, 0, 1;

    return rotation;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    double t = zNear * tan(deg_2_rad(eye_fov/2));
    double b = -t;
    double r = t * aspect_ratio;
    double l = -r;

    Eigen::Matrix4f persp2ortho = Eigen::Matrix4f::Identity();
    persp2ortho << zNear, 0, 0, 0,
                0, zNear, 0, 0,
                0, 0, zNear+zFar, -zNear*zFar,
                0,0,1,0;

    
    Eigen::Matrix4f T = Eigen::Matrix4f::Identity();
    T << 1, 0, 0, -(r+l)/2,
         0, 1, 0, -(t+b)/2,
         0, 0, 1, -(zNear+zFar)/2,
         0, 0, 0, 1;

    Eigen::Matrix4f S = Eigen::Matrix4f::Identity();
    S << 2 / (r - l), 0, 0, 0,
         0, 2 / (t - b), 0, 0, 
         0, 0, 2 / (zNear - zFar), 0,
         0, 0, 0, 1;


    Eigen::Matrix4f ortho = Eigen::Matrix4f::Identity();
    ortho = S * T;

    
    projection = ortho * persp2ortho;


    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    Eigen::Vector3f axisN = {0, 0, 1};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //r.set_model(get_model_matrix(angle));
        r.set_model(get_rotation(axisN, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //r.set_model(get_model_matrix(angle));
        r.set_model(get_rotation(axisN, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
