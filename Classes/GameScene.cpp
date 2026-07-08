#include "GameScene.h"

#include "BoardRenderer.h"
#include "GameBoard.h"

#include "2d/CCLabel.h"
#include "base/CCDirector.h"

using namespace cocos2d;

namespace {
constexpr float TITLE_TOP_OFFSET = 40.0F;
constexpr float BOARD_TIP_BOTTOM_OFFSET = 70.0F;
constexpr const char* TITLE_TEXT = "Cocos2d-x 4.0 Match3 Prototype";
constexpr const char* BOARD_TIP_TEXT = "Bottom 4 rows obstacles; row 5 center 3 open";
constexpr const char* FONT_NAME = "Arial";
constexpr int TITLE_FONT_SIZE = 24;
constexpr int TIP_FONT_SIZE = 20;
}

GameScene::~GameScene() {
    delete mBoardRenderer;
    delete mBoardModel;
}

Scene* GameScene::createScene() {
    return GameScene::create();
}

bool GameScene::init() {
    if (!Scene::init()) {
        return false;
    }

    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();

    const auto title = Label::createWithSystemFont(TITLE_TEXT, FONT_NAME, TITLE_FONT_SIZE);
    title->setPosition(Vec2(
        visibleOrigin.x + visibleSize.width * 0.5F,
        visibleOrigin.y + visibleSize.height - TITLE_TOP_OFFSET));
    addChild(title);

    createBoardPlaceholder();
    return true;
}

void GameScene::createBoardPlaceholder() {
    mBoardModel = new GameBoard();
    mBoardRenderer = new BoardRenderer();

    mBoardRenderer->render(this, *mBoardModel);

    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    const auto boardTip = Label::createWithSystemFont(BOARD_TIP_TEXT, FONT_NAME, TIP_FONT_SIZE);
    boardTip->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + BOARD_TIP_BOTTOM_OFFSET));
    addChild(boardTip);
}
