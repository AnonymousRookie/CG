//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    // 使用双线性插值进行纹理采样
    Eigen::Vector3f getColorBilinear(float u, float v)
    {		
        if (u < 0) u = 0;
        if (u > 1) u = 1;
        if (v < 0) v = 0;
        if (v > 1) v = 1;

        auto u_img = u * width;
        auto v_img = (1 - v) * height;

        float u_min = std::floor(u_img);
        float u_max = std::min((float)width, std::ceil(u_img));
        float v_min = std::floor(v_img);
        float v_max = std::min((float)height, std::ceil(v_img));

        /*
             u1
        u01------u11
        |         |
        |         |
        |         |
        u00------u10
             u0
        */

        // opencv坐标的高和uv坐标和高是相反的
        auto u00 = image_data.at<cv::Vec3b>(v_max, u_min);
        auto u10 = image_data.at<cv::Vec3b>(v_max, u_max);
        auto u01 = image_data.at<cv::Vec3b>(v_min, u_min);
        auto u11 = image_data.at<cv::Vec3b>(v_min, u_max);


        // k = (x-x0) / (x1-x0) = (y-y0) / (y1-y0)
        // x = (1-k)*x0 + k*x1     
        // y = (1-k)*y0 + k*y1     

        auto uk = (u_img - u_min) / (u_max - u_min);
        auto u0 = (1 - uk) * u00 + uk * u10;
        auto u1 = (1 - uk) * u01 + uk * u11;

        auto vk = (v_img - v_max) / (v_min - v_max);
        auto p = (1 - vk) * u0 + vk * u1;

        return Eigen::Vector3f(p[0], p[1], p[2]);
    }   
};
#endif //RASTERIZER_TEXTURE_H
