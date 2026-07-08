#include "GameScene.h"

#include "BoardRenderer.h"
#include "GameBoard.h"

#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerTouch.h"
#include "2d/CCLabel.h"
#include "base/CCDirector.h"
#include "2d/CCLayer.h"
#include "2d/CCActionInterval.h"
#include "2d/CCActionInstant.h"
#include "2d/CCSprite.h"
#include <vector>

using namespace cocos2d;

namespace {
constexpr float TITLE_TOP_OFFSET = 40.0F;
constexpr float BOARD_TIP_BOTTOM_OFFSET = 70.0F;
constexpr float STEP_LABEL_TOP_OFFSET = 80.0F;
constexpr float GOAL_LABEL_TOP_OFFSET = 110.0F;
constexpr const char* TITLE_TEXT = "Cocos2d-x 4.0 Match3 Prototype";
constexpr const char* BOARD_TIP_TEXT = "\u70B9\u51FB\u68CB\u5B50\u540E\uFF0C\u518D\u70B9\u51FB\u76F8\u90BB\u68CB\u5B50\u8FDB\u884C\u4EA4\u6362";
constexpr const char* STEP_TEXT_PREFIX = "\u6B65\u6570\uFF1A";
constexpr const char* GOAL_TEXT_PREFIX = "\u5269\u4F59\u969C\u788D\uFF1A";
constexpr const char* RESULT_FONT_NAME = "Arial";
constexpr const char* FONT_NAME = "Arial";
constexpr int TITLE_FONT_SIZE = 24;
constexpr int TIP_FONT_SIZE = 20;
constexpr int STEP_FONT_SIZE = 20;
constexpr int GOAL_FONT_SIZE = 20;
constexpr float BOARD_CELL_SIZE = 64.0F;
constexpr float BOARD_MARGIN = 8.0F;
constexpr float BOARD_SCALE = 0.42F;
constexpr float SWAP_ANIMATION_DURATION = 0.12F;
constexpr float CLEAR_FADE_DURATION = 0.08F;
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
    updateGoalLabel();

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

    mGoalLabel = cocos2d::Label::createWithSystemFont("", FONT_NAME, GOAL_FONT_SIZE);
    mGoalLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + visibleSize.height - GOAL_LABEL_TOP_OFFSET));
    addChild(mGoalLabel);

    mResultLabel = cocos2d::Label::createWithSystemFont("", RESULT_FONT_NAME, 32);
    mResultLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + visibleSize.height * 0.5F));
    mResultLabel->setVisible(false);
    addChild(mResultLabel);

    refreshBoard();
}

void GameScene::refreshBoard(bool animateDrop) {
    if (mBoardRenderer != nullptr && mBoardModel != nullptr) {
        mBoardRenderer->render(this, *mBoardModel, animateDrop);
    }
}

void GameScene::playClearFeedback() {
    auto flash = Layer::create();
    if (flash == nullptr) {
        return;
    }
    flash->setLocalZOrder(100);
    addChild(flash);
    flash->runAction(Sequence::create(DelayTime::create(0.04F), FadeOut::create(0.08F), RemoveSelf::create(), nullptr));
}

void GameScene::playDropAnimation() {
    auto root = Node::create();
    if (root == nullptr) {
        return;
    }
    root->setName("drop-root");
    root->setLocalZOrder(50);
    addChild(root);

    root->runAction(Sequence::create(DelayTime::create(0.03F), FadeOut::create(0.0F), RemoveSelf::create(), nullptr));
}

void GameScene::resolveMatches() {
    if (mBoardModel == nullptr || mLevelFinished) {
        return;
    }

    std::vector<Cell> matchedCells;
    if (!mBoardModel->collectMatches(matchedCells)) {
        mIsAnimating = false;
        refreshBoard(false);
        checkLevelState();
        return;
    }

    mIsAnimating = true;
    updateGoalLabel();
    clearAdjacentObstacles(matchedCells);

    for (const auto& cell : matchedCells) {
        auto* flash = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
        if (flash != nullptr) {
            flash->setPosition(cellToWorld(cell.row, cell.col));
            flash->setScale(BOARD_SCALE * 0.42F);
            flash->setColor(Color3B::WHITE);
            flash->setOpacity(220);
            flash->setLocalZOrder(HIGHLIGHT_Z_ORDER + 1);
            addChild(flash);
            flash->runAction(Sequence::create(ScaleTo::create(CLEAR_FADE_DURATION, BOARD_SCALE * 0.15F), FadeOut::create(CLEAR_FADE_DURATION), RemoveSelf::create(), nullptr));
        }
        auto* boardCell = mBoardModel->getCell(cell.row, cell.col);
        if (boardCell != nullptr) {
            mBoardModel->clearCell(*boardCell);
        }
    }
    playClearFeedback();
    refreshBoard(false);

    runAction(Sequence::create(DelayTime::create(0.10F), CallFunc::create([this]() {
        if (mBoardModel != nullptr && !mLevelFinished) {
            mBoardModel->collapseAndRefill();
            playDropAnimation();
            refreshBoard(true);
            runAction(Sequence::create(DelayTime::create(0.36F), CallFunc::create([this]() {
                mIsAnimating = false;
                checkLevelState();
                resolveMatches();
            }), nullptr));
        }
    }), nullptr));
}

void GameScene::checkLevelState() {
    if (mLevelFinished) {
        return;
    }

    const auto remainingObstacles = mBoardModel != nullptr ? mBoardModel->countObstacles() : 0;
    if (remainingObstacles == 0) {
        mLevelFinished = true;
        showResultMessage("\u901A\u5173\u6210\u529F");
        return;
    }

    if (!mBoardModel->hasAnyValidMove()) {
        mLevelFinished = true;
        showResultMessage("\u95EF\u5173\u5931\u8D25");
    }
}

void GameScene::clearAdjacentObstacles(const std::vector<Cell>& matchedCells) {
    if (mBoardModel == nullptr) {
        return;
    }

    for (const auto& cell : matchedCells) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (std::abs(dr) + std::abs(dc) != 1) {
                    continue;
                }
                auto* neighbor = mBoardModel->getCell(cell.row + dr, cell.col + dc);
                if (neighbor != nullptr && neighbor->state == CellState::Obstacle) {
                    auto* shatter = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
                    if (shatter != nullptr) {
                        shatter->setPosition(cellToWorld(neighbor->row, neighbor->col));
                        shatter->setScale(BOARD_SCALE * 0.34F);
                        shatter->setColor(Color3B::RED);
                        shatter->setOpacity(240);
                        shatter->setLocalZOrder(HIGHLIGHT_Z_ORDER + 2);
                        addChild(shatter);

                        auto* burst = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
                        if (burst != nullptr) {
                            burst->setPosition(cellToWorld(neighbor->row, neighbor->col));
                            burst->setScale(BOARD_SCALE * 0.20F);
                            burst->setColor(Color3B::WHITE);
                            burst->setOpacity(220);
                            burst->setLocalZOrder(HIGHLIGHT_Z_ORDER + 3);
                            addChild(burst);
                            burst->runAction(Sequence::create(Spawn::create(ScaleTo::create(0.12F, BOARD_SCALE * 0.92F), FadeOut::create(0.12F), nullptr), RemoveSelf::create(), nullptr));
                        }

                        shatter->runAction(Sequence::create(Spawn::create(ScaleTo::create(0.16F, BOARD_SCALE * 0.68F), FadeOut::create(0.16F), nullptr), RemoveSelf::create(), nullptr));
                    }
                    if (shatter != nullptr) {
                        shatter->runAction(Sequence::create(DelayTime::create(0.12F), CallFunc::create([this, neighbor]() {
                            if (neighbor != nullptr && mBoardModel != nullptr) {
                                mBoardModel->clearObstacle(*neighbor);
                            }
                        }), nullptr));
                    } else if (neighbor != nullptr && mBoardModel != nullptr) {
                        mBoardModel->clearObstacle(*neighbor);
                    }
                }
            }
        }
    }
}

void GameScene::showResultMessage(const char* message) {
    if (mResultLabel == nullptr) {
        return;
    }
    mResultLabel->setString(message == nullptr ? "" : message);
    mResultLabel->setVisible(true);
}

void GameScene::updateStepLabel() {
    if (mStepLabel != nullptr) {
        mStepLabel->setString(std::string(STEP_TEXT_PREFIX) + std::to_string(mStepCount) + "/" + std::to_string(mMaxStepCount));
    }
}

void GameScene::updateGoalLabel() {
    if (mGoalLabel != nullptr) {
        const auto remainingObstacles = mBoardModel != nullptr ? mBoardModel->countObstacles() : 0;
        mGoalLabel->setString(std::string(GOAL_TEXT_PREFIX) + std::to_string(remainingObstacles) + " obstacle(s)");
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
    highlightA->runAction(Sequence::create(ScaleTo::create(SWAP_ANIMATION_DURATION, BOARD_SCALE * HIGHLIGHT_SCALE * 1.2F), FadeOut::create(SWAP_ANIMATION_DURATION), RemoveSelf::create(), nullptr));
    highlightB->runAction(Sequence::create(ScaleTo::create(SWAP_ANIMATION_DURATION, BOARD_SCALE * HIGHLIGHT_SCALE * 1.2F), FadeOut::create(SWAP_ANIMATION_DURATION), RemoveSelf::create(), nullptr));
}

bool GameScene::onTouchBegan(Touch* touch, Event* event) {
    CC_UNUSED_PARAM(event);
    if (mBoardModel == nullptr || mBoardRenderer == nullptr || mLevelFinished) {
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
            resolveMatches();
        } else {
            mBoardModel->swapCells(*selected, *cell);
            playSwapFeedback(currentPos, selectedPos);
        }
    }

    mBoardModel->clearSelection();
    refreshBoard();
    return true;
}
