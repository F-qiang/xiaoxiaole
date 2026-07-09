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
constexpr float TITLE_TOP_OFFSET = 14.0F;
constexpr float BOARD_TIP_BOTTOM_OFFSET = 14.0F;
constexpr float STEP_LABEL_TOP_OFFSET = 44.0F;
constexpr float GOAL_LABEL_TOP_OFFSET = 72.0F;
constexpr const char* TITLE_TEXT = "Cocos2d-x 4.0 Match3 Prototype";
constexpr const char* BOARD_TIP_TEXT = "Tap a piece, then tap an adjacent piece to swap.";
constexpr const char* STEP_TEXT_PREFIX = "Steps: ";
constexpr const char* GOAL_TEXT_PREFIX = "Obstacles left: ";
constexpr const char* RESULT_FONT_NAME = "Microsoft YaHei";
constexpr const char* FONT_NAME = "Microsoft YaHei";
constexpr int TITLE_FONT_SIZE = 24;
constexpr int TIP_FONT_SIZE = 20;
constexpr int STEP_FONT_SIZE = 20;
constexpr int GOAL_FONT_SIZE = 20;
constexpr float BOARD_CELL_SIZE = 64.0F;
constexpr float BOARD_MARGIN = 4.0F;
constexpr float BOARD_SCALE = 0.50F;
constexpr float BOARD_LAYOUT_SCALE = 1.16F;
constexpr float BOARD_X_OFFSET = 0.0F;
constexpr float BOARD_Y_OFFSET = -40.0F;
constexpr float BOARD_LEFT_RIGHT_EXTRA = 0.0F;
constexpr float SWAP_ANIMATION_DURATION = 0.12F;
constexpr float CLEAR_FADE_DURATION = 0.08F;
constexpr float HIGHLIGHT_SCALE = 0.56F;
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
    createRestartControl();
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
    mStepLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.25F, visibleOrigin.y + visibleSize.height - STEP_LABEL_TOP_OFFSET));
    addChild(mStepLabel);

    mGoalLabel = cocos2d::Label::createWithSystemFont("", FONT_NAME, GOAL_FONT_SIZE);
    mGoalLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.75F, visibleOrigin.y + visibleSize.height - GOAL_LABEL_TOP_OFFSET));
    addChild(mGoalLabel);

    mResultLabel = cocos2d::Label::createWithSystemFont("", RESULT_FONT_NAME, 24);
    mResultLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + 58.0F));
    mResultLabel->setVisible(false);
    addChild(mResultLabel);

    refreshBoard(false);
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

void GameScene::setSceneState(SceneState state) {
    mSceneState = state;
}

void GameScene::runCollapseAndRefresh() {
    if (mBoardModel == nullptr || mSceneState == SceneState::Victory || mSceneState == SceneState::Failure || mSceneState == SceneState::Dropping) {
        return;
    }
    setSceneState(SceneState::Dropping);
    mBoardModel->collapseAndRefill();
    mPendingSpecialRow = -1;
    mPendingSpecialCol = -1;
    mBoardModel->clearSelection();
    playDropAnimation();
    refreshBoard(true);
    runAction(Sequence::create(DelayTime::create(0.36F), CallFunc::create([this]() {
        if (mBoardModel == nullptr || mSceneState == SceneState::Victory || mSceneState == SceneState::Failure) {
            return;
        }
        setSceneState(SceneState::Idle);
        refreshBoard(false);
        checkLevelState();
        resolveMatches();
    }), nullptr));
}

namespace {
enum class MatchSpecialType {
    None,
    Rocket,
    ColorBomb,
    Bomb,
};

struct MatchSummary {
    bool hasLine4 {false};
    bool hasFive {false};
    bool hasTOrL {false};
    int bestIndex {-1};
    MatchSpecialType specialType {MatchSpecialType::None};
};

MatchSummary analyzeMatchShape(const std::vector<Cell>& matchedCells) {
    MatchSummary summary;
    if (matchedCells.empty()) {
        return summary;
    }

    std::size_t rowCount[GameBoard::ROWS] = {};
    std::size_t colCount[GameBoard::COLS] = {};
    for (const auto& cell : matchedCells) {
        if (cell.row >= 0 && cell.row < static_cast<int>(GameBoard::ROWS)) {
            ++rowCount[static_cast<std::size_t>(cell.row)];
        }
        if (cell.col >= 0 && cell.col < static_cast<int>(GameBoard::COLS)) {
            ++colCount[static_cast<std::size_t>(cell.col)];
        }
    }

    std::size_t bestRunScore = 0;
    for (std::size_t i = 0; i < matchedCells.size(); ++i) {
        const auto& cell = matchedCells[i];
        const auto rowMatch = rowCount[static_cast<std::size_t>(cell.row)];
        const auto colMatch = colCount[static_cast<std::size_t>(cell.col)];
        const auto runScore = rowMatch + colMatch;
        if (rowMatch >= 5 || colMatch >= 5) {
            summary.hasFive = true;
            if (runScore >= bestRunScore) {
                bestRunScore = runScore;
                summary.bestIndex = static_cast<int>(i);
            }
            continue;
        }
        if (rowMatch >= 4 || colMatch >= 4) {
            summary.hasLine4 = true;
            if (runScore >= bestRunScore) {
                bestRunScore = runScore;
                summary.bestIndex = static_cast<int>(i);
            }
        }
    }

    std::size_t rowsWith3Plus = 0;
    std::size_t colsWith3Plus = 0;
    for (std::size_t i = 0; i < GameBoard::ROWS; ++i) {
        if (rowCount[i] >= 3) {
            ++rowsWith3Plus;
        }
    }
    for (std::size_t i = 0; i < GameBoard::COLS; ++i) {
        if (colCount[i] >= 3) {
            ++colsWith3Plus;
        }
    }
    summary.hasTOrL = rowsWith3Plus >= 2 && colsWith3Plus >= 2;
    if (summary.hasFive || summary.hasTOrL) {
        summary.specialType = MatchSpecialType::Bomb;
    } else if (summary.hasLine4) {
        summary.specialType = bestRunScore > 0 && rowCount[static_cast<std::size_t>(matchedCells[summary.bestIndex].row)] >= colCount[static_cast<std::size_t>(matchedCells[summary.bestIndex].col)]
            ? MatchSpecialType::Rocket
            : MatchSpecialType::ColorBomb;
    }
    if (summary.bestIndex < 0) {
        summary.bestIndex = static_cast<int>(matchedCells.size() / 2);
    }
    return summary;
}
}

bool GameScene::isLineMatch(const std::vector<Cell>& matchedCells) const {
    return analyzeMatchShape(matchedCells).hasLine4;
}

bool GameScene::isTOrLMatch(const std::vector<Cell>& matchedCells) const {
    return analyzeMatchShape(matchedCells).hasTOrL;
}

void GameScene::placeSpecialCandy(const std::vector<Cell>& matchedCells) {
    if (mBoardModel == nullptr) {
        return;
    }

    const auto summary = analyzeMatchShape(matchedCells);
    if (summary.specialType == MatchSpecialType::None) {
        return;
    }

    const int bestIndex = summary.bestIndex;
    if (bestIndex < 0 || bestIndex >= static_cast<int>(matchedCells.size())) {
        return;
    }

    const auto& center = matchedCells[static_cast<std::size_t>(bestIndex)];
    auto* boardCell = mBoardModel->getCell(center.row, center.col);
    if (boardCell == nullptr) {
        return;
    }

    boardCell->state = CellState::SpecialPiece;
    boardCell->pieceType = PieceType::Normal;
    boardCell->uid = center.uid;
    switch (summary.specialType) {
        case MatchSpecialType::Bomb:
            boardCell->effectType = EffectType::Bomb;
            break;
        case MatchSpecialType::Rocket:
            boardCell->effectType = EffectType::Rocket;
            break;
        case MatchSpecialType::ColorBomb:
            boardCell->effectType = EffectType::ColorBomb;
            break;
        case MatchSpecialType::None:
            break;
    }
}

void GameScene::resolveMatches() {
    if (mBoardModel == nullptr || mSceneState == SceneState::Victory || mSceneState == SceneState::Failure) {
        return;
    }

    std::vector<Cell> matchedCells;
    if (!mBoardModel->collectMatches(matchedCells)) {
        setSceneState(SceneState::Idle);
        refreshBoard(false);
        checkLevelState();
        return;
    }

    setSceneState(SceneState::Resolving);
    updateGoalLabel();
    clearAdjacentObstacles(matchedCells);

    const auto summary = analyzeMatchShape(matchedCells);
    const int specialIndex = summary.specialType == MatchSpecialType::None ? -1 : summary.bestIndex;
    if (specialIndex >= 0) {
        mPendingSpecialRow = matchedCells[static_cast<std::size_t>(specialIndex)].row;
        mPendingSpecialCol = matchedCells[static_cast<std::size_t>(specialIndex)].col;
    } else {
        mPendingSpecialRow = -1;
        mPendingSpecialCol = -1;
    }
    placeSpecialCandy(matchedCells);

    for (std::size_t i = 0; i < matchedCells.size(); ++i) {
        const auto& cell = matchedCells[i];
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
        if (specialIndex >= 0 && static_cast<int>(i) == specialIndex) {
            continue;
        }
        auto* boardCell = mBoardModel->getCell(cell.row, cell.col);
        if (boardCell != nullptr) {
            if (boardCell->row == mPendingSpecialRow && boardCell->col == mPendingSpecialCol) {
                continue;
            }
            if (boardCell->state == CellState::Obstacle) {
                mBoardModel->clearObstacle(*boardCell);
            } else {
                mBoardModel->clearCell(*boardCell);
            }
        }
    }
    playClearFeedback();
    refreshBoard(false);

    runAction(Sequence::create(DelayTime::create(0.10F), CallFunc::create([this]() {
        if (mBoardModel != nullptr && mSceneState != SceneState::Victory && mSceneState != SceneState::Failure) {
            mBoardModel->collapseAndRefill();
            mPendingSpecialRow = -1;
            mPendingSpecialCol = -1;
            mBoardModel->clearSelection();
            setSceneState(SceneState::Dropping);
            playDropAnimation();
            refreshBoard(true);
            runAction(Sequence::create(DelayTime::create(0.36F), CallFunc::create([this]() {
                if (mBoardModel == nullptr || mSceneState == SceneState::Victory || mSceneState == SceneState::Failure) {
                    return;
                }
                setSceneState(SceneState::Idle);
                refreshBoard(false);
                checkLevelState();
                resolveMatches();
            }), nullptr));
        }
    }), nullptr));
}

void GameScene::checkLevelState() {
    if (mSceneState == SceneState::Victory || mSceneState == SceneState::Failure) {
        return;
    }

    const auto remainingObstacles = mBoardModel != nullptr ? mBoardModel->countObstacles() : 0;
    updateGoalLabel();
    if (remainingObstacles == 0) {
        setSceneState(SceneState::Victory);
        showResultMessage("通关成功");
        return;
    }

    if (!mBoardModel->hasAnyValidMove()) {
        setSceneState(SceneState::Failure);
        showResultMessage("挑战失败");
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
                    playSpecialBurst(neighbor->row, neighbor->col, Color3B::RED, BOARD_SCALE * 0.34F, 0.16F, HIGHLIGHT_Z_ORDER + 2);
                    mBoardModel->clearObstacle(*neighbor);
                }
            }
        }
    }
}

void GameScene::playSpecialBurst(int row, int col, Color3B color, float scale, float duration, int zOrder) {
    auto* effect = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
    if (effect == nullptr) {
        return;
    }
    effect->setPosition(cellToWorld(row, col));
    effect->setScale(scale);
    effect->setColor(color);
    effect->setOpacity(220);
    effect->setLocalZOrder(zOrder);
    addChild(effect);
    effect->runAction(Sequence::create(Spawn::create(ScaleTo::create(duration, scale * 1.25F), FadeOut::create(duration), nullptr), RemoveSelf::create(), nullptr));
}

void GameScene::clearLineAt(int row, int col, bool vertical) {
    if (mBoardModel == nullptr) {
        return;
    }
    playSpecialBurst(row, col, vertical ? Color3B::BLUE : Color3B::GREEN, BOARD_SCALE * 0.24F, 0.16F, HIGHLIGHT_Z_ORDER + 4);
    for (std::size_t index = 0; index < (vertical ? GameBoard::ROWS : GameBoard::COLS); ++index) {
        const int targetRow = vertical ? static_cast<int>(index) : row;
        const int targetCol = vertical ? col : static_cast<int>(index);
        auto* target = mBoardModel->getCell(targetRow, targetCol);
        if (target == nullptr) {
            continue;
        }
        if (target->state == CellState::Obstacle) {
            mBoardModel->clearObstacle(*target);
        } else if (target->state != CellState::EmptyCell) {
            mBoardModel->clearCell(*target);
        }
    }
}

void GameScene::clearCrossAt(int row, int col) {
    playSpecialBurst(row, col, Color3B::RED, BOARD_SCALE * 0.26F, 0.18F, HIGHLIGHT_Z_ORDER + 5);
    if (mBoardModel == nullptr) {
        return;
    }
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            auto* target = mBoardModel->getCell(row + dr, col + dc);
            if (target == nullptr) {
                continue;
            }
            if (target->state == CellState::Obstacle) {
                mBoardModel->clearObstacle(*target);
            } else if (target->state != CellState::EmptyCell) {
                mBoardModel->clearCell(*target);
            }
        }
    }
}

void GameScene::triggerSpecialCombo(Cell& first, Cell& second) {
    if (mBoardModel == nullptr) {
        return;
    }

    auto playComboBurst = [this](int row, int col, Color3B color, float scale, float duration, int zOrder) {
        auto* effect = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
        if (effect == nullptr) {
            return;
        }
        effect->setPosition(cellToWorld(row, col));
        effect->setScale(scale);
        effect->setColor(color);
        effect->setOpacity(220);
        effect->setLocalZOrder(zOrder);
        addChild(effect);
        effect->runAction(Sequence::create(Spawn::create(ScaleTo::create(duration, scale * 1.25F), FadeOut::create(duration), nullptr), RemoveSelf::create(), nullptr));
    };

    const bool firstBomb = GameBoard::isBombCandy(first);
    const bool secondBomb = GameBoard::isBombCandy(second);
    const bool firstVertical = GameBoard::isVerticalClearCandy(first);
    const bool secondVertical = GameBoard::isVerticalClearCandy(second);
    const bool firstHorizontal = GameBoard::isHorizontalClearCandy(first);
    const bool secondHorizontal = GameBoard::isHorizontalClearCandy(second);

    playComboBurst(first.row, first.col, Color3B::WHITE, BOARD_SCALE * 0.26F, 0.12F, HIGHLIGHT_Z_ORDER + 6);
    playComboBurst(second.row, second.col, Color3B::WHITE, BOARD_SCALE * 0.26F, 0.12F, HIGHLIGHT_Z_ORDER + 6);

    if (firstBomb || secondBomb) {
        clearCrossAt(first.row, first.col);
        clearCrossAt(second.row, second.col);
    } else if ((firstVertical && secondHorizontal) || (firstHorizontal && secondVertical)) {
        clearCrossAt(first.row, first.col);
        clearCrossAt(second.row, second.col);
        clearLineAt(first.row, first.col, true);
        clearLineAt(second.row, second.col, false);
    } else if (firstVertical || secondVertical) {
        clearLineAt(firstVertical ? first.row : second.row, firstVertical ? first.col : second.col, true);
    } else if (firstHorizontal || secondHorizontal) {
        clearLineAt(firstHorizontal ? first.row : second.row, firstHorizontal ? first.col : second.col, false);
    }

    mBoardModel->clearCell(first);
    mBoardModel->clearCell(second);
    runCollapseAndRefresh();
}

void GameScene::triggerSpecialCandy(Cell& specialCell) {
    if (mBoardModel == nullptr) {
        return;
    }

    playSpecialBurst(specialCell.row, specialCell.col, Color3B::WHITE, BOARD_SCALE * 0.24F, 0.14F, HIGHLIGHT_Z_ORDER + 4);

    if (GameBoard::isBombCandy(specialCell)) {
        clearCrossAt(specialCell.row, specialCell.col);
    } else if (GameBoard::isVerticalClearCandy(specialCell)) {
        clearLineAt(specialCell.row, specialCell.col, true);
    } else if (GameBoard::isHorizontalClearCandy(specialCell)) {
        clearLineAt(specialCell.row, specialCell.col, false);
    }
    mBoardModel->clearCell(specialCell);
    runCollapseAndRefresh();
}

void GameScene::showResultMessage(const char* message) {
    if (mResultLabel == nullptr) {
        return;
    }
    mResultLabel->setString(message == nullptr ? "" : message);
    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    mResultLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + 58.0F));
    mResultLabel->setVisible(true);
}

void GameScene::hideResultMessage() {
    if (mResultLabel != nullptr) {
        mResultLabel->setVisible(false);
        mResultLabel->setString("");
    }
}

void GameScene::updateStepLabel() {
    if (mStepLabel != nullptr) {
        mStepLabel->setString(std::string(STEP_TEXT_PREFIX) + std::to_string(mStepCount));
    }
    if (mSceneState == SceneState::Idle && mBoardModel != nullptr) {
        setSceneState(SceneState::Idle);
    }
}

void GameScene::updateGoalLabel() {
    if (mGoalLabel != nullptr) {
        const auto remainingObstacles = mBoardModel != nullptr ? mBoardModel->countObstacles() : 0;
        mGoalLabel->setString(std::string(GOAL_TEXT_PREFIX) + std::to_string(remainingObstacles) + " obstacle(s)");
    }
}

void GameScene::createRestartControl() {
    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    mRestartLabel = cocos2d::Label::createWithSystemFont("Restart", "Arial", 20);
    if (mRestartLabel == nullptr) {
        return;
    }
    mRestartLabel->setPosition(Vec2(visibleOrigin.x + visibleSize.width * 0.5F, visibleOrigin.y + visibleSize.height - 104.0F));
    mRestartLabel->setColor(Color3B::YELLOW);
    addChild(mRestartLabel, 20);
}

void GameScene::restartLevel() {
    mStepCount = 0;
    mClearedCount = 0;
    mPendingSpecialRow = -1;
    mPendingSpecialCol = -1;
    mSceneState = SceneState::Idle;
    if (mBoardModel != nullptr) {
        mBoardModel->reset();
        mBoardModel->clearSelection();
    }
    hideResultMessage();
    updateStepLabel();
    updateGoalLabel();
    refreshBoard(false);
}

bool GameScene::isRestartButtonTouched(const Vec2& point) const {
    if (mRestartLabel == nullptr) {
        return false;
    }
    const auto size = mRestartLabel->getContentSize();
    const auto pos = mRestartLabel->getPosition();
    const float halfWidth = size.width * 0.5F + 20.0F;
    const float halfHeight = size.height * 0.5F + 10.0F;
    return point.x >= pos.x - halfWidth && point.x <= pos.x + halfWidth && point.y >= pos.y - halfHeight && point.y <= pos.y + halfHeight;
}

void GameScene::onRestartClicked(cocos2d::Ref*) {
    restartLevel();
}

Vec2 GameScene::cellToWorld(int row, int col) const {
    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    const float boardWidth = (9 * BOARD_CELL_SIZE + 8 * BOARD_MARGIN) * BOARD_LAYOUT_SCALE;
    const float boardHeight = (8 * BOARD_CELL_SIZE + 7 * BOARD_MARGIN) * BOARD_LAYOUT_SCALE;
    const float cellPitch = (BOARD_CELL_SIZE + BOARD_MARGIN) * BOARD_LAYOUT_SCALE;
    const float startX = visibleOrigin.x + (visibleSize.width - boardWidth) * 0.5F + BOARD_X_OFFSET;
    const float startY = visibleOrigin.y + (visibleSize.height - boardHeight) * 0.5F + BOARD_Y_OFFSET;
    const float x = startX + col * cellPitch + BOARD_CELL_SIZE * 0.5F * BOARD_LAYOUT_SCALE;
    const float y = startY + (7 - row) * cellPitch + BOARD_CELL_SIZE * 0.5F * BOARD_LAYOUT_SCALE;
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
    if (mBoardModel == nullptr || mBoardRenderer == nullptr || mSceneState == SceneState::Victory || mSceneState == SceneState::Failure || mSceneState == SceneState::Swapping || mSceneState == SceneState::Resolving || mSceneState == SceneState::Dropping) {
        return false;
    }

    const auto visibleSize = Director::getInstance()->getVisibleSize();
    const auto visibleOrigin = Director::getInstance()->getVisibleOrigin();
    const float boardWidth = (9 * BOARD_CELL_SIZE + 8 * BOARD_MARGIN) * BOARD_LAYOUT_SCALE;
    const float boardHeight = (8 * BOARD_CELL_SIZE + 7 * BOARD_MARGIN) * BOARD_LAYOUT_SCALE;
    const float cellPitch = (BOARD_CELL_SIZE + BOARD_MARGIN) * BOARD_LAYOUT_SCALE;
    const float startX = visibleOrigin.x + (visibleSize.width - boardWidth) * 0.5F + BOARD_X_OFFSET;
    const float startY = visibleOrigin.y + (visibleSize.height - boardHeight) * 0.5F + BOARD_Y_OFFSET;
    const Vec2 location = touch->getLocation();

    if (isRestartButtonTouched(location)) {
        restartLevel();
        return true;
    }

    const float relativeX = location.x - startX;
    const float relativeY = location.y - startY;
    if (relativeX < 0.0F || relativeY < 0.0F || relativeX > boardWidth || relativeY > boardHeight) {
        mBoardModel->clearSelection();
        refreshBoard(false);
        return true;
    }

    const int col = static_cast<int>(relativeX / cellPitch);
    const int row = 7 - static_cast<int>(relativeY / cellPitch);
    auto* cell = mBoardModel->getCell(row, col);
    if (cell == nullptr || cell->state == CellState::Obstacle) {
        mBoardModel->clearSelection();
        refreshBoard(false);
        return true;
    }

    auto* selected = mBoardModel->getSelectedCell();
    if (selected == nullptr) {
        mBoardModel->toggleSelection(*cell);
        refreshBoard(false);
        return true;
    }

    if (selected == cell) {
        mBoardModel->clearSelection();
        refreshBoard(false);
        return true;
    }

    if (!GameBoard::isAdjacent(*selected, *cell)) {
        mBoardModel->clearSelection();
        mBoardModel->toggleSelection(*cell);
        refreshBoard(false);
        return true;
    }

    const Vec2 selectedPos = cellToWorld(selected->row, selected->col);
    const Vec2 currentPos = cellToWorld(cell->row, cell->col);
    if (mBoardModel->swapCells(*selected, *cell)) {
        playSwapFeedback(selectedPos, currentPos);
        const bool selectedSpecial = GameBoard::isSpecialCandy(*selected);
        const bool cellSpecial = GameBoard::isSpecialCandy(*cell);
        const bool hasMatchAfterSwap = mBoardModel->hasMatchAt(selected->row, selected->col) || mBoardModel->hasMatchAt(cell->row, cell->col);
        if (selectedSpecial || cellSpecial || hasMatchAfterSwap) {
            ++mStepCount;
            updateStepLabel();
            if (selectedSpecial && cellSpecial) {
                triggerSpecialCombo(*selected, *cell);
                refreshBoard(false);
                runCollapseAndRefresh();
            } else if (selectedSpecial || cellSpecial) {
                if (selectedSpecial) {
                    triggerSpecialCandy(*selected);
                }
                if (cellSpecial) {
                    triggerSpecialCandy(*cell);
                }
                refreshBoard(false);
                runCollapseAndRefresh();
            } else {
                resolveMatches();
            }
        } else {
            mBoardModel->swapCells(*selected, *cell);
            playSwapFeedback(currentPos, selectedPos);
        }
    }

    mBoardModel->clearSelection();
    refreshBoard(false);
    return true;
}
