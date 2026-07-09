#include "BoardRenderer.h"

#include "GameBoard.h"

#include <algorithm>

#include "2d/CCActionInterval.h"
#include "2d/CCActionEase.h"
#include "2d/CCLabel.h"
#include "2d/CCSprite.h"
#include "base/CCDirector.h"

using namespace cocos2d;

BoardRenderer::BoardRenderer()
    : mNormalPieceFiles{
          "picture/img_game_common/goal_Animal_1_0.png",
          "picture/img_game_common/goal_Animal_2_0.png",
          "picture/img_game_common/goal_Animal_3_0.png",
          "picture/img_game_common/goal_Animal_4_0.png"}
    , mObstaclePieceFile("picture/img_game_common/goal_Animalice.png")
    , mBombPieceFile("picture/img_ig_candy/boost_candy_bomb.png")
    , mVerticalPieceFile("picture/img_ig_candy/boost_candy_hv.png")
    , mHorizontalPieceFile("picture/img_ig_candy/royal_leaves_heng.png") {
    for (auto& row : mPreviousUids) {
        row.fill(0);
    }
}

void BoardRenderer::render(Node* parent, const GameBoard& board, bool animateDrop) {
    if (parent == nullptr) {
        return;
    }

    auto existingBoard = parent->getChildByName("board-layer");
    if (existingBoard != nullptr) {
        existingBoard->removeFromParent();
    }

    auto winSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    const float boardWidth = (BOARD_COLS * CELL_SIZE + (BOARD_COLS - 1) * BOARD_MARGIN) * BOARD_SCALE_FACTOR;
    const float boardHeight = (BOARD_ROWS * CELL_SIZE + (BOARD_ROWS - 1) * BOARD_MARGIN) * BOARD_SCALE_FACTOR;
    const float startX = origin.x + (winSize.width - boardWidth) * 0.5F;
    const float startY = origin.y + (winSize.height - boardHeight) * 0.5F + BOARD_Y_OFFSET;

    std::array<std::array<int, BOARD_COLS>, BOARD_ROWS> previousSnapshot = mPreviousUids;
    bool hasSnapshot = false;
    for (const auto& rowSnapshot : previousSnapshot) {
        for (const auto uid : rowSnapshot) {
            if (uid != 0) {
                hasSnapshot = true;
                break;
            }
        }
        if (hasSnapshot) {
            break;
        }
    }

    auto boardNode = Node::create();
    boardNode->setName("board-layer");
    boardNode->setLocalZOrder(0);
    parent->addChild(boardNode);

    auto bgNode = Node::create();
    bgNode->setName("board-bg-layer");
    bgNode->setLocalZOrder(-20);
    parent->addChild(bgNode);

    const float bgInset = 1.5F;
    const float pieceInset = 4.0F;
    const float pieceBaseScale = PIECE_SCALE;
    for (int row = 0; row < BOARD_ROWS; ++row) {
        for (int col = 0; col < BOARD_COLS; ++col) {
            const float x = startX + col * (CELL_SIZE + BOARD_MARGIN) * BOARD_SCALE_FACTOR + CELL_SIZE * 0.5F * BOARD_SCALE_FACTOR;
            const float y = startY + (BOARD_ROWS - 1 - row) * (CELL_SIZE + BOARD_MARGIN) * BOARD_SCALE_FACTOR + CELL_SIZE * 0.5F * BOARD_SCALE_FACTOR;

            auto bgSprite = Sprite::create(BOARD_BG_FILE);
            if (bgSprite != nullptr) {
                const float bgScaleX = ((CELL_SIZE - bgInset * 2.0F) * BOARD_SCALE_FACTOR) / bgSprite->getContentSize().width;
                const float bgScaleY = ((CELL_SIZE - bgInset * 2.0F) * BOARD_SCALE_FACTOR) / bgSprite->getContentSize().height;
                bgSprite->setPosition(Vec2(x, y));
                bgSprite->setScaleX(bgScaleX);
                bgSprite->setScaleY(bgScaleY);
                bgSprite->setOpacity(250);
                bgNode->addChild(bgSprite);
            }

            const auto* cell = board.getCell(row, col);
            if (cell == nullptr || cell->state == CellState::EmptyCell) {
                continue;
            }

            const char* resource = mNormalPieceFiles[static_cast<std::size_t>(std::max(0, cell->colorType)) % mNormalPieceFiles.size()];
            if (cell->state == CellState::SpecialPiece) {
                if (cell->effectType == EffectType::Bomb) {
                    resource = mBombPieceFile;
                } else if (cell->effectType == EffectType::Rocket) {
                    resource = mVerticalPieceFile;
                } else if (cell->effectType == EffectType::ColorBomb) {
                    resource = mHorizontalPieceFile;
                }
            } else if (cell->state == CellState::Obstacle) {
                resource = mObstaclePieceFile;
            }

            auto piece = Sprite::create(resource);
            if (piece != nullptr) {
                const float scaledSize = (CELL_SIZE - pieceInset * 2.0F) * BOARD_SCALE_FACTOR;
                const float pieceScale = std::min(pieceBaseScale, scaledSize / std::max(piece->getContentSize().width, piece->getContentSize().height));
                piece->setScale(pieceScale);
                const bool isObstacle = cell->state == CellState::Obstacle;
                const bool shouldAnimate = animateDrop && hasSnapshot && !isObstacle && previousSnapshot[row][col] != cell->uid;
                if (shouldAnimate) {
                    int sourceRow = -1;
                    int sourceCol = col;
                    for (int prevRow = 0; prevRow < BOARD_ROWS; ++prevRow) {
                        for (int prevCol = 0; prevCol < BOARD_COLS; ++prevCol) {
                            if (previousSnapshot[prevRow][prevCol] == cell->uid) {
                                sourceRow = prevRow;
                                sourceCol = prevCol;
                                break;
                            }
                        }
                        if (sourceRow >= 0) {
                            break;
                        }
                    }

                    const int dropRows = sourceRow >= 0 ? std::max(1, sourceRow - row) : (row + 2);
                    const float dropOffset = 26.0F + static_cast<float>(dropRows) * 28.0F;
                    const float delay = static_cast<float>(sourceCol) * 0.022F + static_cast<float>(BOARD_ROWS - 1 - row) * 0.018F;
                    piece->setPosition(Vec2(x, y + dropOffset));
                    piece->runAction(Sequence::create(DelayTime::create(delay), EaseBackOut::create(MoveTo::create(0.26F, Vec2(x, y))), nullptr));
                } else {
                    piece->setPosition(Vec2(x, y));
                }
                if (isObstacle) {
                    piece->setColor(Color3B::GRAY);
                    piece->setOpacity(220);
                }
                if (cell->state == CellState::SpecialPiece) {
                    piece->setColor(Color3B::WHITE);
                    piece->setOpacity(255);
                }
                if (cell->isSelected) {
                    piece->setOpacity(180);
                    piece->setColor(Color3B::YELLOW);
                }
                boardNode->addChild(piece);
            }
            mPreviousUids[row][col] = cell->uid;
        }
    }
}
