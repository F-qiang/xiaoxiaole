#pragma once

#include "cocos2d.h"

/**
 * 应用入口委托类。
 *
 * 职责：
 * 1. 初始化 OpenGL 上下文属性
 * 2. 创建并启动首个场景
 * 3. 处理进入后台与恢复前台时的暂停/恢复逻辑
 */
class AppDelegate final : private cocos2d::Application {
public:
    AppDelegate();
    ~AppDelegate() override;

    /**
     * 设置 OpenGL 上下文属性。
     *
     * 这里沿用 Cocos2d-x 原生模板的默认配置，保证 Windows 下渲染环境一致。
     */
    void initGLContextAttrs() override;

    /**
     * 应用启动完成后的初始化入口。
     * 返回 true 表示初始化成功。
     */
    bool applicationDidFinishLaunching() override;

    /**
     * 应用进入后台时调用。
     */
    void applicationDidEnterBackground() override;

    /**
     * 应用回到前台时调用。
     */
    void applicationWillEnterForeground() override;
};
