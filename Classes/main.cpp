#include "AppDelegate.h"

/**
 * Windows / 桌面端主入口。
 *
 * Cocos2d-x 原生模板下通常使用 _tWinMain 作为 Windows 入口。
 */
int WINAPI _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    AppDelegate app;
    return cocos2d::Application::getInstance()->run();
}
