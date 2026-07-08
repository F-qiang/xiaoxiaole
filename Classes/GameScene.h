#pragma once

#include "2d/CCScene.h"

class BoardRenderer;
class GameBoard;

/**
 * 游戏主场景。
 *
 * 负责组织场景生命周期、标题显示和棋盘节点挂载，不直接承担棋盘绘制细节。
 */
class GameScene final : public cocos2d::Scene {
public:
    /**
     * 析构场景，释放内部创建的渲染器和棋盘对象。
     */
    ~GameScene() override;

    /**
     * 创建场景实例的静态入口。
     */
    static cocos2d::Scene* createScene();

    /**
     * 场景初始化。
     *
     * 这里负责创建标题，并委托渲染器绘制棋盘。
     */
    bool init() override;

    CREATE_FUNC(GameScene);

private:
    /**
     * 创建并渲染棋盘内容。
     */
    void createBoardPlaceholder();

    /** 棋盘渲染器对象。 */
    BoardRenderer* mBoardRenderer {nullptr};
    /** 当前棋盘逻辑对象。 */
    GameBoard* mBoardModel {nullptr};
};