#include"opencv2/opencv.hpp"  
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <windows.h>
#include <iostream>
#include<cmath>
using namespace sf;
using namespace cv;
using namespace std;

string writePath = "C:\\Users\\25874\\source\\repos\\OpenCV-demo\\imgs\\save\\";
int c_x = 0;
int c_y = 0;
vector<Vector2<int>> pos;
vector<Vector2<int>> eraser_path;
vector<vector<Vector2<int>>> pen_paths;
vector<vector<Vector2<int>>> eraser_paths;
bool flag = false;  //false: 张开 ，true : 握拳
bool isSave = false;  //flag切换为false时 isSave改为true,保存此次轨迹,保存完毕后isSave改为false;
bool isTake = false;  //拍照
bool isChoose = false;
int show = 0;  //0: 无      1：点    2：线    3:擦除

// ===================   gui  =====================
void paint_point(RenderWindow& window, vector<Vector2<int>>& arr) {
    int size = arr.size();
    for (int i = 0; i < size; i++) {
        CircleShape circle(7, 100);
        circle.setFillColor(Color::Black);
        circle.setPosition(arr[i].x, arr[i].y);
        window.draw(circle);
    }
}

void paint_line(RenderWindow& window, vector<Vector2<int>>& arr) {
    int size = arr.size();
    int lx = 0;
    int ly = 0;
    for (int i = 1; i < size; i++) {
        lx = abs(arr[i - 1].x - arr[i].x);
        ly = abs(arr[i - 1].y - arr[i].y);
        sf::Vertex RectangleShape[] =
        {
            sf::Vertex(sf::Vector2f(arr[i - 1].x, arr[i - 1].y)),
            sf::Vertex(sf::Vector2f(arr[i].x , arr[i].y)),
            sf::Vertex(sf::Vector2f(arr[i].x , arr[i].y + ly + 5)),
            sf::Vertex(sf::Vector2f(arr[i - 1].x  , arr[i - 1].y+5)),

        };

        RectangleShape[0].color = Color::Black;
        RectangleShape[1].color = Color::Black;
        RectangleShape[2].color = Color::Black;
        RectangleShape[3].color = Color::Black;
        window.draw(RectangleShape, 4, sf::Quads);

    }
}





void pop_step() {
    if (show == 2 && !pen_paths.empty())pen_paths.pop_back();
    else if (show == 3 && !eraser_paths.empty())eraser_paths.pop_back();
    isChoose = true;
}

void remove_all() {
    pos.clear();
    eraser_path.clear();
    pen_paths.clear();
    eraser_paths.clear();
    show = 0;
    isChoose = true;
}

void detect(RenderWindow& window) {
    isChoose = true;
    cout << "路径检测" << endl;
    vector<Vector2<int>>* paths = &pos;
    //对单次落笔的路径进行识别，paths 为当前经过的路径
}

void eraser(RenderWindow& window, vector<Vector2<int>>& arr) {
    for (int i = 0; i < arr.size(); i++) {
        CircleShape circle(25, 100);
        circle.setFillColor(Color::White);
        circle.setPosition(arr[i].x, arr[i].y);
        window.draw(circle);
    }
}

void show_eraser(RenderWindow& window) {
    eraser(window, eraser_path);
    for (int i = 0; i < eraser_paths.size(); i++) {
        eraser(window, eraser_paths[i]);
    }
}

void save_step() {
    if (show == 2) {
        pen_paths.push_back(pos);
        pos.clear();
    }
    else if (show == 3) {
        eraser_paths.push_back(eraser_path);
        eraser_path.clear();
    }
    isSave = false;
}

void paint(RenderWindow& window) {
    int size = 0;
    switch (show) {
    case 1:
        paint_point(window, pos);
        size = pen_paths.size();
        for (int i = 0; i < size; i++) {
            paint_point(window, pen_paths[i]);
        }
        break;
    default:
        paint_line(window, pos);
        size = pen_paths.size();
        for (int i = 0; i < size; i++) {
            paint_line(window, pen_paths[i]);
        }
        break;
    }
    show_eraser(window);
}

void clear_board() {
    isChoose = true;
    pen_paths.clear();
    eraser_paths.clear();
}

bool judge(int x1, int x2, int y1, int y2) {
    return c_x > x1 && c_y > y1 && c_x < x2&& c_y < y2;
}

void preWindow() {
    RenderWindow window(VideoMode(800, 600), "paintboard", Style::Close);
    window.setPosition(Vector2<int>(0, 0));

    Texture t1, t2, t3, t4, t5, t6, t7, t8, t0;
    t0.loadFromFile("imgs/pen_connect.png");
    t1.loadFromFile("imgs/pan.png"); t2.loadFromFile("imgs/pen.png");
    t3.loadFromFile("imgs/eraser.png"); t4.loadFromFile("imgs/board.png");
    t5.loadFromFile("imgs/return.png"); t6.loadFromFile("imgs/remake.png");
    t7.loadFromFile("imgs/save.png"); t8.loadFromFile("imgs/识别.png");
    Sprite s0(t0), s1(t1), s2(t2), s3(t3), s4(t4), s5(t5), s6(t6), s7(t7), s8(t8);
    s0.setPosition(540, 20);
    s1.setPosition(350, 20); s2.setPosition(540, 20);
    s3.setPosition(670, 20); s4.setPosition(120, 120);
    s5.setPosition(20, 20); s6.setPosition(20, 200);
    s7.setPosition(0, 300); s8.setPosition(20, 400);
    s0.scale(0.2, 0.2);
    s1.scale(0.2, 0.2); s2.scale(0.2, 0.2);
    s3.scale(0.2, 0.2); s4.scale(1, 0.8);
    s5.scale(0.2, 0.2); s6.scale(0.1, 0.1);
    s7.scale(0.7, 0.7); s8.scale(0.15, 0.15);

    sf::RectangleShape rectangle(sf::Vector2f(600.f, 500.f));
    rectangle.setPosition(150, 150);
    rectangle.setOutlineColor(Color::Black);
    rectangle.setOutlineThickness(2);


    sf::Time time = sf::milliseconds(10);
    while (window.isOpen()) {
        window.clear(Color::White);
        sleep(time);
        Event event;
        //光标

        CircleShape circle(7, 100);
        circle.setFillColor(Color::Blue);
        if (show == 2)circle.setFillColor(Color::Red);
        else if (show == 3)circle.setFillColor(Color::Green);
        if (isSave)save_step();
        circle.setPosition(c_x, c_y);
        if (judge(550, 680, 20, 150) && flag)show = 2;
        else if (judge(700, 800, 20, 150) && flag)show = 3;
        else if (judge(20, 150, 20, 150) && flag && !isChoose)pop_step();
        else if (judge(50, 150, 200, 300) && flag && !isChoose)remove_all();
        else if (judge(20, 150, 300, 400) && flag && !isChoose)isTake = true;
        else if (judge(20, 150, 400, 500) && flag && !isChoose)detect(window);

        sf::Vector2i localPosition = sf::Mouse::getPosition(window);

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }
        window.draw(s1); window.draw(s0);
        window.draw(rectangle); window.draw(s3);
        window.draw(s5); window.draw(s6);
        window.draw(s7); window.draw(s8);
        if (show == 1)window.draw(s2);
        paint(window);
        window.draw(circle);
        window.display();
    }
}

//开启多线程
DWORD WINAPI UDP_S1(LPVOID lpParamter) {
    preWindow();
    return 0L;
}

int go() {
    VideoCapture cap(0);
    cap.set(CAP_PROP_FRAME_WIDTH, 1600);
    cap.set(CAP_PROP_FRAME_HEIGHT, 900);

    string name;
    int pageCount = 0;
    if (!cap.isOpened()) {
        cout << "Could not open Camera stream!";
        return -1;
    }

    HANDLE hThread1 = CreateThread(NULL, 0, UDP_S1, NULL, 0, NULL);
    CloseHandle(hThread1);

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

    while (cap.isOpened()) {
        double t0 = getTickCount();
        Mat frame;
        Mat image_fliped;
        cv::flip(frame, image_fliped, 1);
        cap >> image_fliped;
        if (image_fliped.empty()) {
            cout << "Video over!";
            break;
        }
        open_palm_cascade.detectMultiScale(image_fliped, open_palms, 1.3, 4, 0, Size(50, 50));
        closed_palm_cascade.detectMultiScale(image_fliped, closed_palms, 1.3, 4, 0, Size(50, 50));

        string s;

        if (closed_palms.size() != 0) {
            flag = true;
            Point closed_palm_rect_p1(closed_palms[0].x, closed_palms[0].y);
            Point closed_palm_rect_p2(closed_palms[0].x + closed_palms[0].width, closed_palms[0].y + closed_palms[0].height);

            rectangle(image_fliped, closed_palm_rect_p1, closed_palm_rect_p2, Scalar(0, 255, 0));
            c_x = ((closed_palm_rect_p1.x + closed_palm_rect_p2.x) >> 1) - 200;
            c_y = ((closed_palm_rect_p1.y + closed_palm_rect_p2.y) >> 1) - 200;
            s = "("; s.append(to_string(c_x)); s += ", "; s.append(to_string(c_y)); s += ")";
            putText(image_fliped, s, Point(10, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 1, 5, false);

            if (judge(200, 750, 200, 550)) {
                isChoose = false;
                if (show == 2)pos.push_back(Vector2<int>(c_x, c_y));
                else if (show == 3)eraser_path.push_back(Vector2<int>(c_x, c_y));
            }
        }
        else {
            if (flag) {
                flag = false;
                isSave = true;
            }
            if (open_palms.size() != 0) {
                Point open_palm_rect_p1(open_palms[0].x, open_palms[0].y);
                Point open_palm_rect_p2(open_palms[0].x + open_palms[0].width, open_palms[0].y + open_palms[0].height);
                rectangle(image_fliped, open_palm_rect_p1, open_palm_rect_p2, Scalar(255, 0, 0));
                c_x = ((open_palm_rect_p1.x + open_palm_rect_p2.x) >> 1) - 200;
                c_y = ((open_palm_rect_p1.y + open_palm_rect_p2.y) >> 1) - 200;
                s = "("; s.append(to_string(c_x)); s += ", "; s.append(to_string(c_y)); s += ")";
                putText(image_fliped, s, Point(10, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 1, 5, false);
            }
        }
        if (isTake) {	//拍照
            Mat curr;
            cap >> curr;
            name = writePath + to_string(pageCount++) + ".jpg";
            imwrite(name, curr);
            cout << name << endl;
            isTake = false;
        }
        waitKey(10);
        imshow("Video Capture", image_fliped);
    }
    return 0;
}




float panelWinX = 1200.f;
float panelWinY = 1000.f;


void preWindow_test() {
    RenderWindow window(VideoMode(panelWinX, panelWinY), "paintboard", Style::Close);
    window.setPosition(Vector2<int>(0, 0));

    Texture t1, t2, t3, t4, t5, t6, t7, t8, t0;
    t0.loadFromFile("imgs/pen_connect.png");
    t1.loadFromFile("imgs/pan.png"); t2.loadFromFile("imgs/pen.png");
    t3.loadFromFile("imgs/eraser.png"); t4.loadFromFile("imgs/board.png");
    t5.loadFromFile("imgs/return.png"); t6.loadFromFile("imgs/remake.png");
    t7.loadFromFile("imgs/save.png"); t8.loadFromFile("imgs/识别.png");
    Sprite s0(t0), s1(t1), s2(t2), s3(t3), s4(t4), s5(t5), s6(t6), s7(t7), s8(t8);
    s0.setPosition(0.9 * panelWinX, panelWinY * 0.04);
    s1.setPosition(0.6 * panelWinX, panelWinY * 0.04); 
    s2.setPosition(0.35 * panelWinX, panelWinY * 0.04);
    s3.setPosition(1.1 * panelWinX, panelWinY * 0.04); 
    s4.setPosition(0.2 * panelWinX, panelWinY * 0.2);
    s5.setPosition(0.05 * panelWinX, panelWinY * 0.05); 
    s6.setPosition(0.03 * panelWinX, panelWinY * 0.4);
    s7.setPosition(0 * panelWinX, panelWinY * 0.6);
    s8.setPosition(0.03 * panelWinX, panelWinY * 0.8);
    s0.scale(0.2, 0.2);
    s1.scale(0.2, 0.2); s2.scale(0.2, 0.2);
    s3.scale(0.2, 0.2); s4.scale(1, 0.8);
    s5.scale(0.2, 0.2); s6.scale(0.1, 0.1);
    s7.scale(0.7, 0.7); s8.scale(0.15, 0.15);

    sf::RectangleShape rectangle(sf::Vector2f(panelWinX*0.7, panelWinY*0.7));
    rectangle.setPosition(0.25 * panelWinX, 0.25 * panelWinY);
    rectangle.setOutlineColor(Color::Black);
    rectangle.setOutlineThickness(2);


    sf::Time time = sf::milliseconds(10);
    while (window.isOpen()) {
        window.clear(Color::White);
        sleep(time);
        Event event;
        //光标
        Vector2<int> p = Mouse::getPosition(window);
        c_x = p.x;
        c_y = p.y;

        if (judge(200, 750, 200, 550) && flag) {
            if (show == 2)pos.push_back(Vector2<int>(c_x, c_y));
            else if (show == 3)eraser_path.push_back(Vector2<int>(c_x, c_y));
        }

        CircleShape circle(7, 100);
        circle.setFillColor(Color::Blue);
        if (show == 2)circle.setFillColor(Color::Red);
        else if (show == 3)circle.setFillColor(Color::Green);
        if (isSave)save_step();
        circle.setPosition(c_x, c_y);
        if (judge(550, 680, 20, 150) && flag)show = 2;
        else if (judge(700, 800, 20, 150) && flag)show = 3;
        else if (judge(20, 150, 20, 150) && flag)pop_step();
        else if (judge(50, 150, 200, 300) && flag)remove_all();
        else if (judge(20, 150, 300, 400) && flag)isTake = true;
        else if (judge(20, 150, 400, 500) && flag)detect(window);

        sf::Vector2i localPosition = sf::Mouse::getPosition(window);

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
            if (event.type == Event::MouseButtonPressed) {
                flag = true;
            }
            if (event.type == Event::MouseButtonReleased) {
                flag = false;
                isSave = true;
            }
        }

        window.draw(s1); window.draw(s0);
        window.draw(s2);
        window.draw(rectangle); window.draw(s3);
        window.draw(s5); window.draw(s6);
        window.draw(s7); window.draw(s8);
        //if (show == 1)window.draw(s2);
        paint(window);
        window.draw(circle);
        window.display();
    }
}


int main(int argc, const char** argv) {
    preWindow_test();
    //go();
    return 0;
}