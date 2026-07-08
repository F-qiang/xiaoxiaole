#pragma once

#include "2d/CCScene.h"

/**
 * 游戏主场景。
 *
 * 当前阶段仅负责：
 * 1. 展示基础棋盘占位
 * 2. 为后续棋盘交互、消除、结算提供承载节点
 */
class GameScene final : public cocos2d::Scene {
public:
    /**
     * 创建场景实例的静态入口。
     */
    static cocos2d::Scene* createScene();

    /**
     * 场景初始化。
     *
     * 这里负责创建标题、棋盘占位和后续玩法所需的容器节点。
     */
    bool init() override;

    CREATE_FUNC(GameScene);

private:
    /**
     * 绘制 8 行 x 9 列棋盘占位，用于验证当前项目骨架是否可正常运行。
     */
    void createBoardPlaceholder();
};
