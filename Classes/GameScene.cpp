#include "GameScene.h"

#include "BoardRenderer.h"
#include "GameBoard.h"

#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerTouch.h"
#include "2d/CCLabel.h"
#include "base/CCDirector.h"
#include "2d/CCSprite.h"
#include "base/CCScheduler.h"

using namespace cocos2d;

namespace {
constexpr float TITLE_TOP_OFFSET = 40.0F;
constexpr float BOARD_TIP_BOTTOM_OFFSET = 70.0F;
constexpr float STEP_LABEL_TOP_OFFSET = 80.0F;
constexpr const char* TITLE_TEXT = "Cocos2d-x 4.0 Match3 Prototype";
constexpr const char* BOARD_TIP_TEXT = "Tap a piece, then tap an adjacent piece to swap";
constexpr const char* STEP_TEXT_PREFIX = "Steps: ";
constexpr const char* FONT_NAME = "Arial";
constexpr int TITLE_FONT_SIZE = 24;
constexpr int TIP_FONT_SIZE = 20;
constexpr int STEP_FONT_SIZE = 20;
constexpr float BOARD_CELL_SIZE = 64.0F;
constexpr float BOARD_MARGIN = 8.0F;
constexpr float BOARD_SCALE = 0.42F;
constexpr float SWAP_ANIMATION_DURATION = 0.12F;
constexpr float HIGHLIGHT_SCALE = 0.48F;
constexpr int HIGHLIGHT_Z_ORDER = 10;
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
    title->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + visibleSize.height - TITLE_TOP_OFFSET));
    addChild(title);

    createBoardPlaceholder();
    updateStepLabel();

    auto touchListener = cocos2d::EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);
    return true;
}

void GameScene::createBoardPlaceholder() {
    mBoardModel = new GameBoard();
    mBoardRenderer = new BoardRenderer();

    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    const auto boardTip = cocos2d::Label::createWithSystemFont(BOARD_TIP_TEXT, FONT_NAME, TIP_FONT_SIZE);
    boardTip->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + BOARD_TIP_BOTTOM_OFFSET));
    addChild(boardTip);

    mStepLabel = cocos2d::Label::createWithSystemFont("", FONT_NAME, STEP_FONT_SIZE);
    mStepLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + visibleSize.height - STEP_LABEL_TOP_OFFSET));
    addChild(mStepLabel);

    refreshBoard();
}

void GameScene::refreshBoard() {
    if (mBoardRenderer != nullptr && mBoardModel != nullptr) {
        mBoardRenderer->render(this, *mBoardModel);
    }
}

void GameScene::updateStepLabel() {
    if (mStepLabel != nullptr) {
        mStepLabel->setString(std::string(STEP_TEXT_PREFIX) + std::to_string(mStepCount));
    }
}

Vec2 GameScene::cellToWorld(int row, int col) const {
    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    const float boardWidth = 9 * BOARD_CELL_SIZE + 8 * BOARD_MARGIN;
    const float boardHeight = 8 * BOARD_CELL_SIZE + 7 * BOARD_MARGIN;
    const float startX = visibleOrigin.x + (visibleSize.width - boardWidth) * 0.5F;
    const float startY = visibleOrigin.y + (visibleSize.height - boardHeight) * 0.5F;
    const float x = startX + col * (BOARD_CELL_SIZE + BOARD_MARGIN) + BOARD_CELL_SIZE * 0.5F;
    const float y = startY + (7 - row) * (BOARD_CELL_SIZE + BOARD_MARGIN) + BOARD_CELL_SIZE * 0.5F;
    return Vec2(x, y);
}

void GameScene::playSwapFeedback(const Vec2& from, const Vec2& to) {
    auto highlightA = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
    auto highlightB = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
    if (highlightA == nullptr || highlightB == nullptr) {
        return;
    }

    highlightA->setColor(Color3B::YELLOW);
    highlightB->setColor(Color3B::YELLOW);
    highlightA->setOpacity(180);
    highlightB->setOpacity(180);
    highlightA->setScale(BOARD_SCALE * HIGHLIGHT_SCALE);
    highlightB->setScale(BOARD_SCALE * HIGHLIGHT_SCALE);
    highlightA->setPosition(from);
    highlightB->setPosition(to);
    highlightA->setLocalZOrder(HIGHLIGHT_Z_ORDER);
    highlightB->setLocalZOrder(HIGHLIGHT_Z_ORDER);
    addChild(highlightA);
    addChild(highlightB);
}

bool GameScene::onTouchBegan(Touch* touch, Event* event) {
    CC_UNUSED_PARAM(event);
    if (mBoardModel == nullptr || mBoardRenderer == nullptr) {
        return false;
    }

    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    const float boardWidth = 9 * BOARD_CELL_SIZE + 8 * BOARD_MARGIN;
    const float boardHeight = 8 * BOARD_CELL_SIZE + 7 * BOARD_MARGIN;
    const float startX = visibleOrigin.x + (visibleSize.width - boardWidth) * 0.5F;
    const float startY = visibleOrigin.y + (visibleSize.height - boardHeight) * 0.5F;
    const Vec2 location = touch->getLocation();

    const float relativeX = location.x - startX;
    const float relativeY = location.y - startY;
    if (relativeX < 0.0F || relativeY < 0.0F || relativeX > boardWidth || relativeY > boardHeight) {
        mBoardModel->clearSelection();
        refreshBoard();
        return true;
    }

    const int col = static_cast<int>(relativeX / (BOARD_CELL_SIZE + BOARD_MARGIN));
    const int row = 7 - static_cast<int>(relativeY / (BOARD_CELL_SIZE + BOARD_MARGIN));
    auto* cell = mBoardModel->getCell(row, col);
    if (cell == nullptr || cell->state == CellState::Obstacle) {
        mBoardModel->clearSelection();
        refreshBoard();
        return true;
    }

    auto* selected = mBoardModel->getSelectedCell();
    if (selected == nullptr) {
        mBoardModel->toggleSelection(*cell);
        refreshBoard();
        return true;
    }

    if (selected == cell) {
        mBoardModel->clearSelection();
        refreshBoard();
        return true;
    }

    if (!GameBoard::isAdjacent(*selected, *cell)) {
        mBoardModel->clearSelection();
        mBoardModel->toggleSelection(*cell);
        refreshBoard();
        return true;
    }

    const Vec2 selectedPos = cellToWorld(selected->row, selected->col);
    const Vec2 currentPos = cellToWorld(cell->row, cell->col);
    if (mBoardModel->swapCells(*selected, *cell)) {
        playSwapFeedback(selectedPos, currentPos);
        const bool hasMatchAfterSwap = mBoardModel->hasMatchAt(selected->row, selected->col) || mBoardModel->hasMatchAt(cell->row, cell->col);
        if (hasMatchAfterSwap) {
            ++mStepCount;
            updateStepLabel();
        } else {
            mBoardModel->swapCells(*selected, *cell);
            playSwapFeedback(currentPos, selectedPos);
        }
    }

    mBoardModel->clearSelection();
    refreshBoard();
    return true;
}
