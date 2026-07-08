#include "AppDelegate.h"

#include "GameScene.h"

using namespace cocos2d;

namespace {
constexpr float DESIGN_WIDTH = 720.0F;
constexpr float DESIGN_HEIGHT = 1280.0F;
}

AppDelegate::AppDelegate() = default;
AppDelegate::~AppDelegate() = default;

void AppDelegate::initGLContextAttrs() {
    // 使用官方模板常见的 OpenGL 参数，保证跨平台渲染行为稳定。
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    // Director 是 Cocos2d-x 的全局调度核心，负责场景切换、渲染与时间驱动。
    auto* director = Director::getInstance();
    auto* glView = director->getOpenGLView();

    // 如果外部还没创建视图，这里手动创建原生窗口。
    if (glView == nullptr) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glView = GLViewImpl::createWithRect("xiaoxiaole", cocos2d::Rect(0, 0, DESIGN_WIDTH, DESIGN_HEIGHT));
#else
        glView = GLViewImpl::create("xiaoxiaole");
#endif
        director->setOpenGLView(glView);
    }

    // 开发阶段先开启帧率显示，便于观察运行情况。
    director->setDisplayStats(true);
    director->setAnimationInterval(1.0F / 60.0F);

    // 设定设计分辨率，后续棋盘和 UI 都按这套坐标系布局。
    glView->setDesignResolutionSize(DESIGN_WIDTH, DESIGN_HEIGHT, ResolutionPolicy::SHOW_ALL);

    // 资源搜索路径：
    // Resources 是标准资源目录；resourse 是当前仓库里已有的资源目录，先兼容保留。
    FileUtils::getInstance()->addSearchPath("Resources");
    FileUtils::getInstance()->addSearchPath("resourse");

    // 启动首个场景。
    auto scene = GameScene::createScene();
    director->runWithScene(scene);
    return true;
}

void AppDelegate::applicationDidEnterBackground() {
    // 进入后台时暂停渲染循环，节省资源。
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
    // 回到前台后恢复渲染循环。
    Director::getInstance()->startAnimation();
}
