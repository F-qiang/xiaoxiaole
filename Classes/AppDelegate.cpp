#include "AppDelegate.h"

#include "GameScene.h"

using namespace cocos2d;

namespace {
constexpr float DESIGN_WIDTH = 720.0F;
constexpr float DESIGN_HEIGHT = 960.0F;
}

AppDelegate::AppDelegate() = default;
AppDelegate::~AppDelegate() = default;

void AppDelegate::initGLContextAttrs() {
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    auto* director = Director::getInstance();
    auto* glView = director->getOpenGLView();

    if (glView == nullptr) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glView = GLViewImpl::createWithRect("xiaoxiaole", cocos2d::Rect(0, 0, DESIGN_WIDTH, DESIGN_HEIGHT));
#else
        glView = GLViewImpl::create("xiaoxiaole");
#endif
        director->setOpenGLView(glView);
    }

    director->setDisplayStats(false);
    director->setAnimationInterval(1.0F / 60.0F);
    glView->setDesignResolutionSize(DESIGN_WIDTH, DESIGN_HEIGHT, ResolutionPolicy::FIXED_HEIGHT);
    FileUtils::getInstance()->addSearchPath("Resources");
    FileUtils::getInstance()->addSearchPath("resourse");

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
