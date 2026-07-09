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
#include "audio/include/AudioEngine.h"
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
constexpr float CLEAR_FADE_DURATION = 0.36F;
constexpr float HIGHLIGHT_SCALE = 0.56F;
constexpr int HIGHLIGHT_Z_ORDER = 10;
constexpr const char* SOUND_ROCKET = "music/rorcket.mp3";
constexpr const char* SOUND_WIN = "music/win.mp3";
constexpr const char* SOUND_BOMB = "music/baozha.mp3";
constexpr const char* SOUND_COMMON = "music/common.mp3";
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

    preloadSoundEffects();

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

void GameScene::preloadSoundEffects() {
    cocos2d::AudioEngine::preload(SOUND_ROCKET, nullptr);
    cocos2d::AudioEngine::preload(SOUND_WIN, nullptr);
    cocos2d::AudioEngine::preload(SOUND_BOMB, nullptr);
    cocos2d::AudioEngine::preload(SOUND_COMMON, nullptr);
}

void GameScene::playSoundEffect(const char* filePath) {
    if (filePath == nullptr || *filePath == '\0') {
        return;
    }
    cocos2d::AudioEngine::play2d(filePath, false, 1.0F);
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
        const auto position = cellToWorld(cell.row, cell.col);
        auto* burst = Sprite::create("picture/img_ig_candy/efx_Rainbowefx.png");
        if (burst != nullptr) {
            burst->setPosition(position);
            burst->setScale(BOARD_SCALE * 0.34F);
            burst->setOpacity(255);
            burst->setLocalZOrder(HIGHLIGHT_Z_ORDER + 4);
            addChild(burst);
            burst->runAction(Sequence::create(Spawn::create(ScaleTo::create(CLEAR_FADE_DURATION, BOARD_SCALE * 1.20F), RotateBy::create(CLEAR_FADE_DURATION, 120.0F), FadeOut::create(CLEAR_FADE_DURATION), nullptr), RemoveSelf::create(), nullptr));
        }

        auto* lightning = Sprite::create("picture/img_ig_candy/efx_lightning0.png");
        if (lightning != nullptr) {
            lightning->setPosition(position);
            lightning->setScale(BOARD_SCALE * 0.82F);
            lightning->setOpacity(240);
            lightning->setLocalZOrder(HIGHLIGHT_Z_ORDER + 5);
            addChild(lightning);
            lightning->runAction(Sequence::create(Spawn::create(ScaleTo::create(CLEAR_FADE_DURATION, BOARD_SCALE * 1.05F), FadeOut::create(CLEAR_FADE_DURATION), nullptr), RemoveSelf::create(), nullptr));
        }

        static constexpr const char* PARTICLE_FILES[] = {
            "picture/img_ig_candy/particle_die_candy_1_1_01.png",
            "picture/img_ig_candy/particle_die_candy_1_2_01.png",
            "picture/img_ig_candy/particle_die_candy_1_3_01.png",
            "picture/img_ig_candy/particle_die_candy_1_4_01.png",
            "picture/img_ig_candy/particle_die_candy_1_5_01.png",
            "picture/img_ig_candy/particle_die_candy_1_6_01.png",
        };
        for (int particleIndex = 0; particleIndex < 6; ++particleIndex) {
            auto* shard = Sprite::create(PARTICLE_FILES[particleIndex]);
            if (shard == nullptr) {
                continue;
            }
            const float directionX = particleIndex % 3 == 0 ? -1.0F : particleIndex % 3 == 1 ? 0.0F : 1.0F;
            const float directionY = particleIndex < 3 ? 1.0F : -1.0F;
            const float distance = 26.0F + static_cast<float>(particleIndex) * 3.0F;
            shard->setPosition(position);
            shard->setScale(BOARD_SCALE * 0.42F);
            shard->setOpacity(255);
            shard->setLocalZOrder(HIGHLIGHT_Z_ORDER + 6);
            addChild(shard);
            shard->runAction(Sequence::create(Spawn::create(MoveBy::create(CLEAR_FADE_DURATION, Vec2(directionX * distance, directionY * distance)), ScaleTo::create(CLEAR_FADE_DURATION, BOARD_SCALE * 0.16F), RotateBy::create(CLEAR_FADE_DURATION, directionX * 180.0F), FadeOut::create(CLEAR_FADE_DURATION), nullptr), RemoveSelf::create(), nullptr));
        }

        auto* ring = Sprite::create("picture/img_ig_candy/particle_other_dot_candy.png");
        if (ring != nullptr) {
            ring->setPosition(position);
            ring->setScale(BOARD_SCALE * 0.28F);
            ring->setOpacity(255);
            ring->setLocalZOrder(HIGHLIGHT_Z_ORDER + 3);
            addChild(ring);
            ring->runAction(Sequence::create(Spawn::create(ScaleTo::create(CLEAR_FADE_DURATION, BOARD_SCALE * 1.25F), FadeOut::create(CLEAR_FADE_DURATION), nullptr), RemoveSelf::create(), nullptr));
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
    playSoundEffect(SOUND_COMMON);
    refreshBoard(false);

    runAction(Sequence::create(DelayTime::create(0.36F), CallFunc::create([this]() {
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
        playSoundEffect(SOUND_WIN);
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
    auto* center = Sprite::create("picture/img_ig_candy/efx_Rainbowefx.png");
    if (center == nullptr) {
        center = Sprite::create("picture/img_game_common/goal_Animal_1_0.png");
    }
    if (center == nullptr) {
        return;
    }
    const auto position = cellToWorld(row, col);
    center->setPosition(position);
    center->setScale(scale * 0.72F);
    center->setColor(color);
    center->setOpacity(255);
    center->setLocalZOrder(zOrder);
    addChild(center);
    center->runAction(Sequence::create(Spawn::create(ScaleTo::create(duration, scale * 1.65F), RotateBy::create(duration, 240.0F), FadeOut::create(duration), nullptr), RemoveSelf::create(), nullptr));

    auto* flash = Sprite::create("picture/img_ig_candy/efx_lightning0.png");
    if (flash != nullptr) {
        flash->setPosition(position);
        flash->setScale(scale * 0.95F);
        flash->setOpacity(255);
        flash->setColor(color);
        flash->setLocalZOrder(zOrder + 1);
        addChild(flash);
        flash->runAction(Sequence::create(Spawn::create(ScaleTo::create(duration * 0.85F, scale * 2.0F), FadeOut::create(duration * 0.85F), nullptr), RemoveSelf::create(), nullptr));
    }

    auto* ring = Sprite::create("picture/img_ig_candy/particle_other_dot_candy.png");
    if (ring != nullptr) {
        ring->setPosition(position);
        ring->setScale(scale * 0.35F);
        ring->setOpacity(240);
        ring->setColor(Color3B::WHITE);
        ring->setLocalZOrder(zOrder + 2);
        addChild(ring);
        ring->runAction(Sequence::create(Spawn::create(ScaleTo::create(duration, scale * 2.4F), FadeOut::create(duration), nullptr), RemoveSelf::create(), nullptr));
    }
}

void GameScene::playRocketCharge(int row, int col, bool vertical) {
    const auto position = cellToWorld(row, col);
    auto* glow = Sprite::create("picture/img_ig_candy/efx_Rainbowefx.png");
    if (glow == nullptr) {
        return;
    }
    glow->setPosition(position);
    glow->setScale(BOARD_SCALE * 0.38F);
    glow->setOpacity(220);
    glow->setColor(Color3B::YELLOW);
    glow->setLocalZOrder(HIGHLIGHT_Z_ORDER + 12);
    glow->setRotation(vertical ? 90.0F : 0.0F);
    addChild(glow);
    glow->runAction(Sequence::create(Repeat::create(Sequence::create(FadeTo::create(0.14F, 255), FadeTo::create(0.14F, 100), nullptr), 3), RemoveSelf::create(), nullptr));

    auto* piece = mBoardModel != nullptr ? mBoardModel->getCell(row, col) : nullptr;
    if (piece != nullptr && piece->state == CellState::SpecialPiece) {
        auto* jump = Sprite::create("picture/img_game_common/efx_arrow_1.png");
        if (jump != nullptr) {
            jump->setPosition(position);
            jump->setScale(BOARD_SCALE * 0.16F);
            jump->setRotation(vertical ? 90.0F : 0.0F);
            jump->setOpacity(255);
            jump->setColor(Color3B::WHITE);
            jump->setLocalZOrder(HIGHLIGHT_Z_ORDER + 13);
            addChild(jump);
            jump->runAction(Sequence::create(Spawn::create(MoveBy::create(0.22F, Vec2(0.0F, 8.0F)), ScaleTo::create(0.22F, BOARD_SCALE * 0.20F), FadeOut::create(0.22F), nullptr), RemoveSelf::create(), nullptr));
        }
    }
}

void GameScene::playRocketTrail(int row, int col, bool vertical) {
    const auto base = cellToWorld(row, col);
    const float travel = vertical ? static_cast<float>(GameBoard::ROWS - 1) : static_cast<float>(GameBoard::COLS - 1);
    const float distance = travel * (BOARD_CELL_SIZE + BOARD_MARGIN) * BOARD_LAYOUT_SCALE * 1.35F;
    const Vec2 direction = vertical ? Vec2(0.0F, 1.0F) : Vec2(1.0F, 0.0F);
    const float rotation = vertical ? 90.0F : 0.0F;

    auto spawnArrow = [this, base, direction, rotation, distance](float offset, float scale, float duration, int zOrder, float opacity, bool brightTip) {
        auto* arrow = Sprite::create("picture/img_game_common/efx_arrow_1.png");
        if (arrow == nullptr) {
            return;
        }
        arrow->setPosition(base - direction * offset);
        arrow->setRotation(rotation);
        arrow->setScale(scale);
        arrow->setOpacity(static_cast<unsigned char>(opacity));
        arrow->setColor(brightTip ? Color3B::YELLOW : Color3B::WHITE);
        arrow->setLocalZOrder(zOrder);
        addChild(arrow);
        arrow->runAction(Sequence::create(Spawn::create(MoveBy::create(duration, direction * (offset + 16.0F)), ScaleTo::create(duration, scale * 1.18F), FadeOut::create(duration), nullptr), RemoveSelf::create(), nullptr));

        if (brightTip) {
            auto* tipGlow = Sprite::create("picture/img_game_common/efx_arrow_1.png");
            if (tipGlow != nullptr) {
                tipGlow->setPosition(base + direction * 22.0F);
                tipGlow->setRotation(rotation);
                tipGlow->setScale(scale * 0.92F);
                tipGlow->setOpacity(255);
                tipGlow->setColor(Color3B::WHITE);
                tipGlow->setLocalZOrder(zOrder + 2);
                addChild(tipGlow);
                tipGlow->runAction(Sequence::create(Spawn::create(MoveBy::create(duration, direction * (distance * 0.28F)), ScaleTo::create(duration, scale * 0.72F), FadeOut::create(duration), nullptr), RemoveSelf::create(), nullptr));
            }

            auto* tip = Sprite::create("picture/img_game_common/efx_arrow_1.png");
            if (tip != nullptr) {
                tip->setPosition(base + direction * 26.0F);
                tip->setRotation(rotation);
                tip->setScale(scale * 0.72F);
                tip->setOpacity(255);
                tip->setColor(Color3B::WHITE);
                tip->setLocalZOrder(zOrder + 1);
                addChild(tip);
                tip->runAction(Sequence::create(Spawn::create(MoveBy::create(duration, direction * distance), ScaleTo::create(duration, scale * 0.58F), FadeOut::create(duration), nullptr), RemoveSelf::create(), nullptr));
            }
        }
    };

    spawnArrow(180.0F, BOARD_SCALE * 0.30F, 0.95F, HIGHLIGHT_Z_ORDER + 8, 255.0F, false);
    spawnArrow(120.0F, BOARD_SCALE * 0.26F, 0.82F, HIGHLIGHT_Z_ORDER + 9, 240.0F, false);
    spawnArrow(60.0F, BOARD_SCALE * 0.22F, 0.70F, HIGHLIGHT_Z_ORDER + 10, 220.0F, true);

    auto* streak = Sprite::create("picture/img_ig_candy/efx_arrow_1.png");
    if (streak != nullptr) {
        streak->setPosition(base - direction * 120.0F);
        streak->setRotation(rotation);
        streak->setScale(BOARD_SCALE * 0.22F);
        streak->setOpacity(245);
        streak->setColor(Color3B::YELLOW);
        streak->setLocalZOrder(HIGHLIGHT_Z_ORDER + 11);
        addChild(streak);
        streak->runAction(Sequence::create(Spawn::create(MoveBy::create(0.78F, direction * (distance + 36.0F)), ScaleTo::create(0.78F, BOARD_SCALE * 0.34F), FadeOut::create(0.78F), nullptr), RemoveSelf::create(), nullptr));
    }
}

void GameScene::playBombCharge(int row, int col) {
    const auto position = cellToWorld(row, col);
    auto* glow = Sprite::create("picture/img_ig_candy/efx_Rainbowefx.png");
    if (glow == nullptr) {
        return;
    }
    glow->setPosition(position);
    glow->setScale(BOARD_SCALE * 0.42F);
    glow->setOpacity(240);
    glow->setColor(Color3B::RED);
    glow->setLocalZOrder(HIGHLIGHT_Z_ORDER + 12);
    addChild(glow);
    glow->runAction(Sequence::create(Repeat::create(Sequence::create(ScaleTo::create(0.12F, BOARD_SCALE * 0.58F), ScaleTo::create(0.12F, BOARD_SCALE * 0.42F), nullptr), 3), RemoveSelf::create(), nullptr));
}

void GameScene::playBombExplosion(int row, int col) {
    const auto position = cellToWorld(row, col);
    auto* core = Sprite::create("picture/img_ig_candy/efx_Rainbowefx.png");
    if (core != nullptr) {
        core->setPosition(position);
        core->setScale(BOARD_SCALE * 0.52F);
        core->setOpacity(255);
        core->setColor(Color3B::RED);
        core->setLocalZOrder(HIGHLIGHT_Z_ORDER + 9);
        addChild(core);
        core->runAction(Sequence::create(Spawn::create(ScaleTo::create(0.24F, BOARD_SCALE * 1.55F), RotateBy::create(0.24F, 480.0F), FadeOut::create(0.24F), nullptr), RemoveSelf::create(), nullptr));
    }

    auto* shock = Sprite::create("picture/img_ig_candy/particle_other_dot_candy.png");
    if (shock != nullptr) {
        shock->setPosition(position);
        shock->setScale(BOARD_SCALE * 0.28F);
        shock->setOpacity(255);
        shock->setColor(Color3B::WHITE);
        shock->setLocalZOrder(HIGHLIGHT_Z_ORDER + 10);
        addChild(shock);
        shock->runAction(Sequence::create(Spawn::create(ScaleTo::create(0.42F, BOARD_SCALE * 2.6F), FadeOut::create(0.42F), nullptr), RemoveSelf::create(), nullptr));
    }

    auto* shock2 = Sprite::create("picture/img_ig_candy/efx_lightning0.png");
    if (shock2 != nullptr) {
        shock2->setPosition(position);
        shock2->setScale(BOARD_SCALE * 0.36F);
        shock2->setOpacity(230);
        shock2->setColor(Color3B::WHITE);
        shock2->setLocalZOrder(HIGHLIGHT_Z_ORDER + 11);
        addChild(shock2);
        shock2->runAction(Sequence::create(Spawn::create(ScaleTo::create(0.34F, BOARD_SCALE * 2.0F), FadeOut::create(0.34F), nullptr), RemoveSelf::create(), nullptr));
    }

    static constexpr const char* BURST_FILES[] = {
        "picture/img_ig_candy/particle_die_candy_1_1_01.png",
        "picture/img_ig_candy/particle_die_candy_1_2_01.png",
        "picture/img_ig_candy/particle_die_candy_1_3_01.png",
        "picture/img_ig_candy/particle_die_candy_1_4_01.png",
        "picture/img_ig_candy/particle_die_candy_1_5_01.png",
        "picture/img_ig_candy/particle_die_candy_1_6_01.png",
    };
    for (int i = 0; i < 6; ++i) {
        auto* spark = Sprite::create(BURST_FILES[i]);
        if (spark == nullptr) {
            continue;
        }
        const float angle = static_cast<float>(i) * 60.0F;
        const float rad = angle * 3.1415926F / 180.0F;
        spark->setPosition(position);
        spark->setScale(BOARD_SCALE * 0.20F);
        spark->setOpacity(255);
        spark->setColor(Color3B::WHITE);
        spark->setRotation(angle);
        spark->setLocalZOrder(HIGHLIGHT_Z_ORDER + 13);
        addChild(spark);
        spark->runAction(Sequence::create(Spawn::create(MoveBy::create(0.34F, Vec2(std::cos(rad) * 48.0F, std::sin(rad) * 48.0F)), ScaleTo::create(0.34F, BOARD_SCALE * 0.08F), FadeOut::create(0.34F), nullptr), RemoveSelf::create(), nullptr));
    }
}

void GameScene::shakeScene(float intensity, float duration) {
    auto* actionTarget = getActionManager() != nullptr ? this : nullptr;
    if (actionTarget == nullptr) {
        return;
    }
    const Vec2 original = getPosition();
    auto* shake = Sequence::create(MoveBy::create(0.05F, Vec2(-intensity, 0.0F)), MoveBy::create(0.05F, Vec2(intensity * 2.0F, 0.0F)), MoveBy::create(0.05F, Vec2(-intensity * 1.5F, 0.0F)), MoveBy::create(0.05F, Vec2(0.0F, intensity)), MoveBy::create(0.05F, Vec2(0.0F, -intensity * 2.0F)), MoveBy::create(0.05F, Vec2(0.0F, intensity)), MoveTo::create(0.05F, original), nullptr);
    runAction(shake);
}

void GameScene::clearLineAt(int row, int col, bool vertical) {
    if (mBoardModel == nullptr) {
        return;
    }
    playRocketCharge(row, col, vertical);
    runAction(Sequence::create(DelayTime::create(0.22F), CallFunc::create([this]() {
        shakeScene(1.8F, 0.22F);
    }), nullptr));
    playRocketTrail(row, col, vertical);
    playSoundEffect(SOUND_ROCKET);
    playSpecialBurst(row, col, vertical ? Color3B::BLUE : Color3B::GREEN, BOARD_SCALE * 0.24F, 0.28F, HIGHLIGHT_Z_ORDER + 4);
    runAction(Sequence::create(DelayTime::create(0.95F), CallFunc::create([this]() {
        shakeScene(1.0F, 0.10F);
    }), nullptr));
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
    playBombCharge(row, col);
    runAction(Sequence::create(DelayTime::create(0.20F), CallFunc::create([this]() {
        shakeScene(4.5F, 0.24F);
    }), nullptr));
    playBombExplosion(row, col);
    playSoundEffect(SOUND_BOMB);
    playSpecialBurst(row, col, Color3B::RED, BOARD_SCALE * 0.26F, 0.30F, HIGHLIGHT_Z_ORDER + 5);
    if (mBoardModel == nullptr) {
        return;
    }
    for (int dr = -2; dr <= 2; ++dr) {
        for (int dc = -2; dc <= 2; ++dc) {
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

    playComboBurst(first.row, first.col, Color3B::WHITE, BOARD_SCALE * 0.34F, 0.22F, HIGHLIGHT_Z_ORDER + 6);
    playComboBurst(second.row, second.col, Color3B::WHITE, BOARD_SCALE * 0.34F, 0.22F, HIGHLIGHT_Z_ORDER + 6);

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

    const bool isBomb = GameBoard::isBombCandy(specialCell);
    const bool isVertical = GameBoard::isVerticalClearCandy(specialCell);
    const bool isHorizontal = GameBoard::isHorizontalClearCandy(specialCell);
    playSpecialBurst(specialCell.row, specialCell.col, Color3B::WHITE, BOARD_SCALE * 0.24F, 0.22F, HIGHLIGHT_Z_ORDER + 4);

    if (isBomb) {
        clearCrossAt(specialCell.row, specialCell.col);
    } else if (isVertical) {
        clearLineAt(specialCell.row, specialCell.col, true);
    } else if (isHorizontal) {
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
