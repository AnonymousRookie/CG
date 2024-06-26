#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm

    /*
    auto &p_0 = control_points[0];
    auto &p_1 = control_points[1];
    auto &p_2 = control_points[2];
    auto &p_3 = control_points[3];

    auto p_01 = (1-t) * p_0 + t * p_1;
    auto p_11 = (1-t) * p_1 + t * p_2;
    auto p_21 = (1-t) * p_2 + t * p_3;

    auto p_02 = (1-t) * p_01 + t * p_11;
    auto p_12 = (1-t) * p_11 + t * p_21;

    auto p_03 = (1-t) * p_02 + t * p_12;


    return cv::Point2f(p_03);
    */

    if (1 == control_points.size()) 
    {
        return control_points[0];
    }
    std::vector<cv::Point2f> points;
    for (int i = 0; i < control_points.size() - 1; ++i)
    {
        auto pt = (1-t) * control_points[i] + t * control_points[i+1];
        points.push_back(pt);
    }
    return recursive_bezier(points, t);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.

    /*
    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = recursive_bezier(control_points, t);
        // 设置为绿色
        window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
    }
    */

    // 反走样, 对于一个曲线上的点，不只把它对应于一个像素，需要根据到像素中心的距离来考虑与它相邻的像素的颜色。
    // 每得到一个曲线上的点时，就根据它到自己周围的3*3个像素中心的距离dist来为这些像素填色，以达到平滑过渡的效果。
    // 每个像素的颜色是255*ratio，dist的范围是[0, 3/sqrt(2)]，ratio的范围是[0, 1]，
    // 则ratio关于dist的函数就是ratio = 1 - std::sqrt(2) / 3.0 * dist
    // 重复计算的点就按照该点的颜色最大值算。
    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = recursive_bezier(control_points, t);
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                int x = point.x + i;
                int y = point.y + j;

                double dist = std::sqrt(std::pow(point.x - x, 2) + std::pow(point.y - y, 2));
                double ratio = 1 - std::sqrt(2) / 3.0 * dist;
                
                double lastClr = window.at<cv::Vec3b>(y, x)[1];
                window.at<cv::Vec3b>(y, x)[1] = std::max(lastClr, 255.0 * ratio);
            }
        }
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            // 同时调用 naive_bezier 和 bezier 函数，如果实现正确，则两者均应写入大致相同的像素，因此该曲线将表现为黄色
            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
