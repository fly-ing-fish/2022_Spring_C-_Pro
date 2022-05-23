#include"opencv2/opencv.hpp"  
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <Windows.h>


using namespace sf;
using namespace cv;
using namespace std;

//void connect_to_dabase() {
//    Database d;
//    d.connect();
//}

string writePath = "C:\\Users\\25874\\source\\repos\\OpenCV-demo\\imgs\\save\\";
int c_x = 0;
int c_y = 0;
vector<Vector2<int>> pos;
vector<Vector2<int>> eraser_path;
vector<vector<Vector2<int>>> pen_paths;
vector<vector<Vector2<int>>> eraser_paths;
int flag = 0;  //0: 仅光标，1：画点  2：描线  3：擦除
int show = 0;  //0: 无      1：点    2：线

/*基于YCrCb空间的肤色检测+二值化+开运算+高斯模糊*/
Mat skin(Mat& ImageIn)
{
    Mat Image_y;
    flip(ImageIn, Image_y, 1);//将图像沿y轴翻转，即镜像
    namedWindow("前置摄像头", WINDOW_FULLSCREEN | WINDOW_KEEPRATIO); imshow("前置摄像头", Image_y);

    Mat Image = Image_y.clone();//用clone()函数复制图像
    Mat YCrCb_Image;
    cvtColor(Image, YCrCb_Image, COLOR_BGR2YCrCb);
    vector<Mat>Y_Cr_Cb;
    split(YCrCb_Image, Y_Cr_Cb);
    Mat CR = Y_Cr_Cb[1];
    Mat CB = Y_Cr_Cb[2];
    Mat ImageOut = Mat::zeros(Image.size(), CV_8UC1);//zeros():构建一个全为0的矩阵，即创建一个全黑的图片

    //Cr>133 && Cr<173 && Cb>77 && Cb<127
    for (int i = 0; i < Image.rows; i++)
        for (int j = 0; j < Image.cols; j++)
            if (CR.at<uchar>(i, j) >= 133 && CR.at<uchar>(i, j) <= 173 && CB.at<uchar>(i, j) >= 77 && CB.at<uchar>(i, j) <= 127)
                ImageOut.at<uchar>(i, j) = 255;

    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));//结构元素 表示内核为一个3*3的矩形
    morphologyEx(ImageOut, ImageOut, MORPH_OPEN, kernel);//使用morphologyEx()函数进行开运算
    GaussianBlur(ImageOut, ImageOut, Size(3, 3), 5);

    return ImageOut;
}

/*连通空心部分+腐蚀*/
Mat Floodfill(Mat& Img_src)
{
    Size f_size = Img_src.size();
    Mat image = Mat::zeros(f_size.height + 2, f_size.width + 2, CV_8UC1);
    Img_src.copyTo(image(Range(1, f_size.height + 1), Range(1, f_size.width + 1)));
    floodFill(image, Point(0, 0), Scalar(255));
    Mat cutImg, Img_dst;
    image(Range(1, f_size.height + 1), Range(1, f_size.width + 1)).copyTo(cutImg);
    Img_dst = Img_src | (~cutImg);

    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(10, 10));//结构元素 表示内核为一个10*10的矩形
    morphologyEx(Img_dst, Img_dst, MORPH_ERODE, kernel1);//使用morphologyEx()函数进行腐蚀运算

    return Img_dst;
}

/*计算两点间距离*/
double distance(Point a, Point b)
{
    double distance = sqrt(abs((a.x - b.x) * (a.x - a.x) + (a.y - b.y) * (a.y - b.y)));
    return distance;
}

/*将坐标点连接成封闭图形*/
void draw(Mat Img1, Mat Img2)
{
    for (int i = 0; i < Img1.rows; i++)
    {
        if (i == Img1.rows - 1)
        {
            Vec2i point1 = Img1.at<Vec2i>(i);
            Vec2i point2 = Img1.at<Vec2i>(0);
            cv::line(Img2, point1, point2, Scalar(255, 255, 255), 2, 8, 0);
            break;
        }
        Vec2i point1 = Img1.at<Vec2i>(i);
        Vec2i point2 = Img1.at<Vec2i>(i + 1);
        cv::line(Img2, point1, point2, Scalar(255, 255, 255), 5, 8, 0);
    }
}

/*多边形拟合曲线绘制近似轮廓*/
Mat approx(Mat& Img_src) {
    Mat Img_dst = Mat::zeros(Img_src.size(), CV_8UC1);

    vector<vector<Point>>contours;
    vector<Vec4i>hierarchy;
    findContours(Img_src, contours, hierarchy, 0, 2, Point());

    for (int t = 0; t < contours.size(); t++) {
        Mat app;
        approxPolyDP(contours[t], app, 15, true);
        draw(app, Img_dst);
    }
    return Img_dst;
}

/*凸包检测+重心+ 鼠标操作 */
Mat CH(Mat& Image_src) {
    /*轮廓*/
    Mat ImageOut = approx(Image_src);
    vector<vector<Point>>contours;
    vector<Vec4i>hierarchy;
    findContours(ImageOut, contours, hierarchy, 0, 2, Point());

    /*画重心*/
    Moments moment = moments(ImageOut, true);
    Point center(moment.m10 / moment.m00, moment.m01 / moment.m00);
    //cout << "重心坐标为：x = " << center.x <<" y = " << center.y<<endl;
    c_x = center.x;
    c_y = center.y;
    if (flag == 1 || flag == 2) {
        pos.push_back(Vector2<int>(center.x, center.y));
    }
    else if (flag == 3) {
        eraser_path.push_back(Vector2<int>(center.x, center.y));
    }

    circle(ImageOut, center, 8, Scalar(55, 155, 0), -1);
    int dist{};
    int sum = 0; //凸包数量
    for (int t = 0; t < contours.size(); t++) {
        /*凸包检测*/
        vector<Point>hull;
        convexHull(contours[t], hull);

        for (size_t i = 0; i < hull.size(); i++) {
            int a = hull.size();

            if (i != hull.size() - 1)
                dist = distance(hull[i], hull[i + 1]);
            int dist1 = distance(hull[i], center);
            if (hull[i].y < center.y && dist> 15) {
                circle(ImageOut, hull[i], 10, Scalar(255, 255, 255), 2, 8, 0);
                sum += 1;
            }

            if (i == hull.size() - 1) {
                cv::line(ImageOut, hull[i], hull[0], Scalar(0, 0, 255), 5, 8, 0);
                break;
            }
            cv::line(ImageOut, hull[i], hull[i + 1], Scalar(0, 0, 255), 5, 8, 0);
        }
    }
    return	ImageOut;
}

// ===================   gui  =====================
void paint_point(RenderWindow& window, vector<Vector2<int>>& arr) {
    for (int i = 0; i < arr.size(); i++) {
        CircleShape circle(7, 100);
        circle.setFillColor(Color::Black);
        circle.setPosition((arr[i].x << 1) - 200, (arr[i].y << 1) - 200);
        window.draw(circle);
    }
}

void paint_line(RenderWindow& window, vector<Vector2<int>>& arr) {
    for (int i = 1; i < arr.size(); i++) {
        sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f((arr[i - 1].x << 1) - 200, (arr[i - 1].y << 1) - 200)),
            sf::Vertex(sf::Vector2f((arr[i].x << 1) - 200, (arr[i].y << 1) - 200))
        };
        line->color = Color::Black;
        window.draw(line, 2, sf::Lines);

    }
}

void eraser(RenderWindow& window, vector<Vector2<int>>& arr) {
    for (int i = 0; i < arr.size(); i++) {
        CircleShape circle(50, 100);
        circle.setFillColor(Color::White);
        circle.setPosition((arr[i].x << 1) - 200, (arr[i].y << 1) - 200);
        window.draw(circle);
    }
}

void show_eraser(RenderWindow& window) {
    eraser(window, eraser_path);
    for (int i = 0; i < eraser_paths.size(); i++) {
        eraser(window, eraser_paths[i]);
    }
}

void paint(RenderWindow& window) {
    switch (show) {
    case 1:
        paint_point(window, pos);
        for (int i = 0; i < pen_paths.size(); i++) {
            paint_point(window, pen_paths[i]);
        }
        break;
    case 2:
        paint_line(window, pos);
        for (int i = 0; i < pen_paths.size(); i++) {
            paint_line(window, pen_paths[i]);
        }
        break;
    default:
        break;
    }
    show_eraser(window);
}

void preWindow() {
    RenderWindow window(VideoMode(800, 800), "paintboard", Style::Close);
    window.setPosition(Vector2<int>(0, 0));

    Texture t1, t2, t3;
    t1.loadFromFile("imgs/pan.png"); t2.loadFromFile("imgs/pen.png"); t3.loadFromFile("imgs/eraser.png");
    Sprite s1(t1), s2(t2), s3(t3);
    s1.setPosition(560, 0); s2.setPosition(640, 0); s3.setPosition(700, 0);
    s1.scale(0.1, 0.1); s2.scale(0.1, 0.1); s3.scale(0.1, 0.1);

    while (window.isOpen()) {
        window.clear(Color::White);
        Event event;
        //光标
        CircleShape circle(7, 100);
        circle.setFillColor(Color::Red);
        circle.setPosition((c_x << 1) - 200, (c_y << 1) - 200);

        sf::Vector2i localPosition = sf::Mouse::getPosition(window);

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            //按住 左shift 开始绘制点
            if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::Key::LShift) {
                flag = 1;
                show = 1;
            }
            //按住 右shift 开始绘制线
            if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::Key::RShift) {
                flag = 2;
                show = 2;
            }
            //按住 右ctrl 擦除
            if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::Key::RControl) {
                flag = 3;
            }
            //按住 左ctrl 清空面板
            if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::Key::LControl) {
                pen_paths.clear();
                eraser_paths.clear();
            }
            //松开 按键 停止绘制
            if (event.type == sf::Event::EventType::KeyReleased) {
                if (flag == 1 || flag == 2) {
                    pen_paths.push_back(pos);
                    pos.clear();
                }
                else if ((flag == 3)) {
                    eraser_paths.push_back(eraser_path);
                    eraser_path.clear();
                }
                flag = 0;
            }
        }
        paint(window);
        window.draw(s1); window.draw(s2); window.draw(s3);
        window.draw(circle);
        window.display();
    }
}

//开启多线程
DWORD WINAPI UDP_S1(LPVOID lpParamter) {
    preWindow();
    return 0L;
}


int run() {
    /*调用摄像头*/
    VideoCapture capture(0);
    /*检查是否成功打开*/
    if (!capture.isOpened()) {
        cout << "摄像头打开失败T_T";
        return -1;
    }
    string name;
    int i = 0;

    HANDLE hThread1 = CreateThread(NULL, 0, UDP_S1, NULL, 0, NULL);
    CloseHandle(hThread1);

    while (1) {
        Mat In;
        capture >> In;

        Mat A = skin(In);
        Mat B = Floodfill(A);
        Mat Out = CH(B);

        Mat frame;
        capture >> frame;

        if (32 == cv::waitKey(10)) {			//空格拍照
            name = writePath + to_string(i) + ".jpg";
            imwrite(name, frame);
            cout << name << endl;
            i++;
        }

        namedWindow("Result", WINDOW_FULLSCREEN | WINDOW_KEEPRATIO); cv::imshow("Result", Out);

        if (27 == cv::waitKey(10)) break;  //按ESC退出            
    }
    return 0;
}


#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int go() {

    VideoCapture cap(0);

    cap.set(CAP_PROP_FRAME_WIDTH, 800);
    cap.set(CAP_PROP_FRAME_HEIGHT, 800);

    if (!cap.isOpened()) {
        cout << "Could not open Camera stream!";
        return -1;
    }

    HANDLE hThread1 = CreateThread(NULL, 0, UDP_S1, NULL, 0, NULL);
    CloseHandle(hThread1);

    // Load open_palm cascade and eye cascade files (.xml file) The XML files should be
    // in the same directory as the project.

    CascadeClassifier open_palm_cascade;
    CascadeClassifier closed_palm_cascade;
    open_palm_cascade.load("open_palm.xml");
    closed_palm_cascade.load("closed_palm.xml");

    if (open_palm_cascade.empty()) {
        cout << "Could not load open_palm configuration file! "
            "Check directory! " << endl << "Press Q to Quit!" << endl;
        while (char(waitKey(0)) != 'q') {}
        return -2;
    }

    if (closed_palm_cascade.empty()) {
        cout << "Could not load closed_palm configuration file! "
            "Check directory! " << endl << "Press Q to Quit!" << endl;
        while (char(waitKey(0)) != 'q') {}
        return -2;
    }

    // Start the open_palm and eye detection phase
    vector<cv::Rect> open_palms;
    vector<cv::Rect> closed_palms;

    while (char(waitKey(1)) != 'q' && cap.isOpened()) {
        double t0 = getTickCount();
        Mat frame;
        Mat image_fliped;

        cv::flip(frame, image_fliped, 1);

        cap >> image_fliped;

        if (image_fliped.empty()) {
            cout << "Video over!";
            break;
        }

        // Detect all the open_palms.
        open_palm_cascade.detectMultiScale(image_fliped, open_palms, 1.3, 4, 0, Size(50, 50));
        closed_palm_cascade.detectMultiScale(image_fliped, closed_palms, 1.3, 4, 0, Size(50, 50));

        if (closed_palms.size() != 0) {

            for (int i = 0; i < closed_palms.size(); i++) {
                if (closed_palms.size() > 1)continue;
                //cout << "=============Detected a closed_palm!=============" << endl;

                // Top left and bottom right points of rectangle.
                Point closed_palm_rect_p1(closed_palms[i].x, closed_palms[i].y);
                Point closed_palm_rect_p2(closed_palms[i].x + closed_palms[i].width, closed_palms[i].y + closed_palms[i].height);

                // Draw the rectangle in the image.
                rectangle(image_fliped, closed_palm_rect_p1, closed_palm_rect_p2, Scalar(0, 255, 0));
                putText(image_fliped, "Closed Palm", closed_palm_rect_p1, FONT_HERSHEY_SIMPLEX,
                    1, Scalar(0, 255, 0), 1, 5, false);

                c_x = (closed_palm_rect_p1.x + closed_palm_rect_p2.x) >> 1;
                c_y = ((closed_palm_rect_p1.y + closed_palm_rect_p2.y) >> 1);

                //cout <<"中心坐标为 x= ："<< c_x << "y = " << c_y << endl;


                if (flag == 1 || flag == 2) {
                    pos.push_back(Vector2<int>(c_x, c_y));
                }
                else if (flag == 3) {
                    eraser_path.push_back(Vector2<int>(c_x, c_y));
                }

            }
        }
        for (int i = 0; i < open_palms.size(); i++) {
            if (open_palms.size() > 1)continue;
            //cout << "=============Detected an open_palm!=============" << endl;

            // Top left and bottom right points of rectangle.
            Point open_palm_rect_p1(open_palms[i].x, open_palms[i].y);
            Point open_palm_rect_p2(open_palms[i].x + open_palms[i].width, open_palms[i].y + open_palms[i].height);

            // Draw the rectangle in the image.
            rectangle(image_fliped, open_palm_rect_p1, open_palm_rect_p2, Scalar(255, 0, 0));
            putText(image_fliped, "Open Palm", open_palm_rect_p1, FONT_HERSHEY_SIMPLEX,
                1, Scalar(255, 0, 0), 1, 5, false);


            c_x = (open_palm_rect_p1.x + open_palm_rect_p2.x) >> 1;
            c_y = ((open_palm_rect_p1.y + open_palm_rect_p2.y) >> 1);

            //cout << "中心坐标为 x= ：" << c_x << "y = " << c_y << endl;

            if (flag == 1 || flag == 2) {
                pos.push_back(Vector2<int>(c_x, c_y));
            }
            else if (flag == 3) {
                eraser_path.push_back(Vector2<int>(c_x, c_y));
            }

        }

        imshow("Video Capture", image_fliped);
        putText(frame, "Closed Palm", , FONT_HERSHEY_SIMPLEX,
            1, Scalar(0, 255, 0), 1, 5, false);
        //cout << "Frame rate = " << getTickFrequency() / (getTickCount() - t0) << endl;
    }
    return 0;
}

int main(int argc, const char** argv) {
    go();
    return 0;
}


