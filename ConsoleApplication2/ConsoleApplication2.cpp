#include "header.h"
#include "opencv2/opencv.hpp"
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaobjdetect.hpp>

#include "utils.h"

using namespace std;
using namespace cv;

#include <Windows.h>
#include <random>

cv::Mat screenshot(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);

    // 根据 DPI 缩放比例调整客户区域大小
    float scale = (float)GetDpiForWindow(hwnd) / USER_DEFAULT_SCREEN_DPI;
    int width = rect.right * scale;
    int height = rect.bottom * scale;

    HDC hScreen = GetDC(hwnd);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL bRet = BitBlt(hDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

    cv::Mat src;
    src.create(height, width, CV_8UC4);

    BITMAPINFOHEADER bi;
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

    SelectObject(hDC, old_obj);
    StretchBlt(hDC, 0, 0, width, height, hScreen, 0, 0, width, height, SRCCOPY);
    GetDIBits(hDC, hBitmap, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);

    return src;
}

DWORD WINAPI ThreadFunc(LPVOID lpParam) {
    for (int i = 0; i < 20; i++) {
        keybd_event('W', MapVirtualKey('W', 0), KEYEVENTF_EXTENDEDKEY, 0);
        Sleep(50);
    }
    keybd_event('W', MapVirtualKey('W', 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

    return 0;
}

int main() {
    ProcessWindowInfo info;
    GetProcessByName(L"r5apex.exe", &info);
    EnumWindows(enumWindowCallback, (LPARAM)&info);
    SetFocus(info.hwnd);

    std::vector<Rect> h_found;
    Ptr<cuda::CascadeClassifier> cascade = cuda::CascadeClassifier::create("haarcascade_fullbody.xml");
    cuda::GpuMat d_frame, d_gray, d_found;

    Mat frame = screenshot(info.hwnd);

    int height = frame.rows;
    int width = frame.cols;

    char wasd[4] = {'W', 'A', 'S', 'D'};

    // 设置视频编解码器和输出视频文件名
    //cv::VideoWriter video("001.mp4", cv::VideoWriter::fourcc('X', '2', '6', '4'), 24, frame.size(), true);

    for (;;) {
        frame = screenshot(info.hwnd);
        d_frame.upload(frame);
        cuda::cvtColor(d_frame, d_gray, cv::COLOR_BGRA2GRAY);

        cascade->detectMultiScale(d_gray, d_found);
        cascade->convert(d_found, h_found);

        HANDLE threads = NULL;
        for (int i = 0; i < h_found.size(); ++i) {
            rectangle(frame, Point(h_found[i].x, h_found[i].y),
                      Point(h_found[i].x + h_found[i].width, h_found[i].y + h_found[i].height), Scalar(0, 255, 255), 3);

            // Calculate center of the detected rectangle
            int centerX = h_found[i].x + h_found[i].width / 2;
            int centerY = h_found[i].y + h_found[i].height / 2;

            // Calculate the difference between the center of the rectangle and the center of the screen
            // Adjust the values according to your screen resolution and mouse sensitivity
            int dx = centerX - frame.cols / 2;
            int dy = centerY - frame.rows / 2;

            // Move the mouse accordingly
            mouse_event(MOUSEEVENTF_MOVE, dx, dy, 0, 0);
            threads = CreateThread(NULL, 0, ThreadFunc, 0, 0, NULL);
        }

        if(h_found.size() <= 0) {
            int randomNumber = random();
            mouse_event(MOUSEEVENTF_MOVE, randomNumber, 0, 0, 0);
        }
       
        imshow("Result", frame);
        waitKey(1);
        CloseHandle(threads);
        // 写入图像帧到视频文件
        //cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);
        //video.write(frame);
    }
}
