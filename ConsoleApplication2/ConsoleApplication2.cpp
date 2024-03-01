#include "header.h"
#include "opencv2/opencv.hpp"
#include "utils.h"

using namespace std;
using namespace cv;

#include <Windows.h>
#include <opencv2/opencv.hpp>

#include <Windows.h>
#include <opencv2/opencv.hpp>


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

void type_string(const std::string& str) {
    INPUT ip;
    ZeroMemory(&ip, sizeof(INPUT));

    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    for (const char& c : str) {
        ip.ki.wVk = 0;
        ip.ki.wScan = c;
        ip.ki.dwFlags = KEYEVENTF_UNICODE; // Use Unicode
        SendInput(1, &ip, sizeof(INPUT));

        ip.ki.dwFlags |= KEYEVENTF_KEYUP; // Key release
        SendInput(1, &ip, sizeof(INPUT));
    }
}

void click(int x, int y) {
    INPUT ip;
    ZeroMemory(&ip, sizeof(INPUT));

    ip.type = INPUT_MOUSE;
    ip.mi.dx = x * (65536 / GetSystemMetrics(SM_CXSCREEN));//x being coord in pixels
    ip.mi.dy = y * (65536 / GetSystemMetrics(SM_CYSCREEN));//y being coord in pixels

    ip.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &ip, sizeof(INPUT));

    ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &ip, sizeof(INPUT));
}

class SimilarityDetector {
public:
    Mat image;
    Mat target;
    Mat result;
    float scale;

public:
    SimilarityDetector() {}

    enum class MbrName {
        set_target,
        set_scale,
        set_image,
        locate_on_image,
        save_res
    };

    virtual void set_target(const string& path) {
        target = imread(path);
        cvtColor(target, target, COLOR_BGR2BGRA);
        resize(target, target, Size(), scale, scale);
    }

    virtual void set_scale(float dpi) {
        scale = dpi / USER_DEFAULT_SCREEN_DPI;
    }

    virtual void set_image(Mat mat) {
        image = mat;
    }

    virtual Rect locate_on_image(double threshold) {
        double minVal = 0.0;
        double maxVal = 0.0;

        Point minLoc = { 0 };
        Point point = { 0 };

        matchTemplate(image, target, result, TM_CCOEFF_NORMED);
        minMaxLoc(result, &minVal, &maxVal, &minLoc, &point, Mat());
        cout << maxVal << endl;

        return Rect(point.x, point.y, result.cols, result.rows);
    }

    virtual void save_res(Rect rect) {
        Point p1 = { rect.x, rect.y };
        Point p2 = { (int)(rect.width / scale), (int)(rect.height / scale) };

        rectangle(image, p1, p2, Scalar(255, 0, 255, 0), 20);
        imwrite("image.png", image);
    }

    using MethodHandler = void(*)(const SimilarityDetector*, uintptr_t);
    const uintptr_t* vtable = *(uintptr_t**)this;

    constexpr auto get_method(const MbrName& index) const& {
        return [this, index](uintptr_t p) {
            MethodHandler handle = (MethodHandler)vtable[(int)index];
            return handle(this, p);
        };
    }
};

int main() {
    SimilarityDetector detector;

    detector.set_scale(GetDpiForWindow(GetDesktopWindow()));
    detector.set_target("Next.png");

    auto aptr = detector.get_method(2);
    Maybe<uintptr_t> v((uintptr_t)&detector.target);
    v.and_then(aptr);

    return 0;
}