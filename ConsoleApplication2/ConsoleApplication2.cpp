#include "header.h"
#include "opencv2/opencv.hpp"

#include "utils.h"

using namespace std;
using namespace cv;

#include <Windows.h>
#include <random>
class ScreenShot {
public:
    HDC hScreen;
    HDC hDC;
    HBITMAP hBitmap;
    BITMAPINFOHEADER bi;
    int width;
    int height;
    cv::Mat src;

    ScreenShot(HWND hwnd) {
        RECT rect;
        GetClientRect(hwnd, &rect);

        // 根据 DPI 缩放比例调整客户区域大小
        float scale = (float)GetDpiForWindow(hwnd) / USER_DEFAULT_SCREEN_DPI;
        width = rect.right * scale;
        height = rect.bottom * scale;

        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        hScreen = GetDC(hwnd);
        hDC = CreateCompatibleDC(hScreen);
    }

    ~ScreenShot() {
        DeleteDC(hDC);
        ReleaseDC(NULL, hScreen);
        DeleteObject(hBitmap);
    }

    cv::Mat capture() {
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
        HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
        BOOL bRet = BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

        cv::Mat src;
        src.create(height, width, CV_8UC4);

        SelectObject(hDC, old_obj);
        StretchBlt(hDC, 0, 0, width, height, hScreen, 0, 0, width, height, SRCCOPY);
        GetDIBits(hDC, hBitmap, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

        DeleteObject(hBitmap);

        return src;
    }
};


DWORD WINAPI ThreadFunc(LPVOID param) {
    POINT point = *(POINT*)param;

    int x_step = (point.x < 0) ? -random(5) : random(6); // x方向步长为负数或正数
    int y_step = (point.y < 0) ? -1 : 1; // y方向步长为负数或正数

    int max_steps = max(abs(point.x), abs(point.y)); // 获取x和y中较大的绝对值作为循环次数

    for (int i = 0; i < max_steps; i++) {
        if (i < abs(point.x)) {
            mouse_event(MOUSEEVENTF_MOVE, x_step, 0, 0, 0); // 移动x方向
        }
        if (i < abs(point.y)) {
            mouse_event(MOUSEEVENTF_MOVE, 0, y_step, 0, 0); // 移动y方向
        }
        Sleep(random(100) % 2);
    }

    return 0;
}

DWORD WINAPI ThreadFunc2(LPVOID lpParam) {
    char c = *(char*)lpParam;

    for (int i = 0; i < random(90, 30); i++) {
        keybd_event(c, MapVirtualKey(c, 0), KEYEVENTF_EXTENDEDKEY, 0);
        Sleep(50);
    }
    keybd_event(c, MapVirtualKey(c, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

    return 0;
}

int main() {
    ProcessWindowInfo info;
    GetProcessByName(L"r5apex.exe", &info);
    EnumWindows(enumWindowCallback, (LPARAM)&info);
    SetFocus(info.hwnd);

    std::vector<Rect> h_found;
    CascadeClassifier cascade("haarcascade_fullbody.xml");
    Mat d_gray;
    char c[] = { 'W', 'A', 'S', 'D' };

    ScreenShot screen(info.hwnd);
    for (uint64_t count = 0;;) {
        Mat frame = screen.capture();
        cvtColor(frame, d_gray, cv::COLOR_BGRA2GRAY);
        medianBlur(d_gray, d_gray, 5);

        cascade.detectMultiScale(d_gray, h_found, 1.1, 3);

        for (int i = 0; i < h_found.size(); ++i) {
            rectangle(frame, Point(h_found[i].x, h_found[i].y),
                      Point(h_found[i].x + h_found[i].width, h_found[i].y + h_found[i].height), Scalar(0, 255, 255), 3);

            count = 0;

            // Calculate center of the detected rectangle
            int centerX = h_found[i].x + h_found[i].width / 2;
            int centerY = h_found[i].y + h_found[i].height / 2;

            // Calculate the difference between the center of the rectangle and the center of the screen
            // Adjust the values according to your screen resolution and mouse sensitivity
            int dx = centerX - frame.cols / 2;
            int dy = centerY - frame.rows / 2;

            POINT point = { dx, dy };

            ThreadFunc(&point);
            CreateThread(NULL, 0, ThreadFunc2, &c[0], 0, NULL);
        }

        if(count++ > 10) {
            mouse_event(MOUSEEVENTF_MOVE, 50 * random(3), 0, 0, 0); // 移动x方向
            char val = c[random(20) % 4];
            CreateThread(NULL, 0, ThreadFunc2, &val, 0, NULL);

            count = 0;
        }

        //imshow("Result", frame);
        //waitKey(1);
    }
}
