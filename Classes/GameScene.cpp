#include "GameScene.h"

#include "GameBoard.h"

#include "2d/CCDrawNode.h"
#include "2d/CCLabel.h"
#include "2d/CCNode.h"
#include "base/CCDirector.h"

using namespace cocos2d;

namespace {
constexpr int BOARD_ROWS = 8;
constexpr int BOARD_COLS = 9;
constexpr float CELL_SIZE = 64.0F;
constexpr float BOARD_MARGIN = 8.0F;
}

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // 顶部标题用于确认当前运行的是原生 Cocos2d-x 骨架。
    auto winSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto title = Label::createWithSystemFont("Cocos2d-x 4.0消除游戏骨架", "Arial", 24);
    title->setPosition(Vec2(origin.x + winSize.width * 0.5F, origin.y + winSize.height - 40.0F));
    addChild(title);

    createBoardPlaceholder();

    return true;
}

void GameScene::createBoardPlaceholder() {
    // 这里先只绘制棋盘占位，后续会替换成真正的棋子节点层。
    auto winSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    const float boardWidth = BOARD_COLS * CELL_SIZE + (BOARD_COLS - 1) * BOARD_MARGIN;
    const float boardHeight = BOARD_ROWS * CELL_SIZE + (BOARD_ROWS - 1) * BOARD_MARGIN;
    const float startX = origin.x + (winSize.width - boardWidth) * 0.5F;
    const float startY = origin.y + (winSize.height - boardHeight) * 0.5F;

    auto boardNode = Node::create();
    boardNode->setPosition(Vec2::ZERO);
    addChild(boardNode);

    auto draw = DrawNode::create();
    boardNode->addChild(draw);

    // 按 8 行 x 9 列绘制规则网格，验证棋盘边界与布局是否正确。
    for (int row = 0; row < BOARD_ROWS; ++row) {
        for (int col = 0; col < BOARD_COLS; ++col) {
            const float x = startX + col * (CELL_SIZE + BOARD_MARGIN) + CELL_SIZE * 0.5F;
            const float y = startY + (BOARD_ROWS - 1 - row) * (CELL_SIZE + BOARD_MARGIN) + CELL_SIZE * 0.5F;

            // 每个格子先画成深色底块，表示可交互区域。
            draw->drawSolidRect(
                Vec2(x - CELL_SIZE * 0.5F, y - CELL_SIZE * 0.5F),
                Vec2(x + CELL_SIZE * 0.5F, y + CELL_SIZE * 0.5F),
                Color4F(0.15F, 0.18F, 0.22F, 1.0F));

            // 白色描边用于观察 8x9 网格是否整齐。
            draw->drawRect(
                Vec2(x - CELL_SIZE * 0.5F, y - CELL_SIZE * 0.5F),
                Vec2(x + CELL_SIZE * 0.5F, y + CELL_SIZE * 0.5F),
                Color4F::WHITE);
        }
    }

    auto boardTip = Label::createWithSystemFont("8x9 Board", "Arial", 20);
    boardTip->setPosition(Vec2(origin.x + winSize.width * 0.5F, startY - 30.0F));
    addChild(boardTip);
}
