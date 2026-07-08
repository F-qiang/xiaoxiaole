#include "BoardRenderer.h"

#include "GameBoard.h"

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
    , mObstaclePieceFile("picture/img_game_common/goal_Animalice.png") {
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

    const float boardWidth = BOARD_COLS * CELL_SIZE + (BOARD_COLS - 1) * BOARD_MARGIN;
    const float boardHeight = BOARD_ROWS * CELL_SIZE + (BOARD_ROWS - 1) * BOARD_MARGIN;
    const float startX = origin.x + (winSize.width - boardWidth) * 0.5F;
    const float startY = origin.y + (winSize.height - boardHeight) * 0.5F;

    std::array<std::array<int, BOARD_COLS>, BOARD_ROWS> previousSnapshot = mPreviousUids;

    auto boardNode = Node::create();
    boardNode->setName("board-layer");
    parent->addChild(boardNode);

    for (int row = 0; row < BOARD_ROWS; ++row) {
        for (int col = 0; col < BOARD_COLS; ++col) {
            const float x = startX + col * (CELL_SIZE + BOARD_MARGIN) + CELL_SIZE * 0.5F;
            const float y = startY + (BOARD_ROWS - 1 - row) * (CELL_SIZE + BOARD_MARGIN) + CELL_SIZE * 0.5F;

            const auto* cell = board.getCell(row, col);
            if (cell == nullptr || cell->state == CellState::EmptyCell) {
                continue;
            }

            const bool isObstacle = cell->state == CellState::Obstacle;
            const std::size_t fileIndex = static_cast<std::size_t>(cell->colorType < 0 ? 0 : cell->colorType) % mNormalPieceFiles.size();
            auto piece = Sprite::create(isObstacle ? mObstaclePieceFile : mNormalPieceFiles[fileIndex]);
            if (piece != nullptr) {
                piece->setScale(PIECE_SCALE);
                const bool shouldAnimate = animateDrop && !isObstacle && previousSnapshot[row][col] != cell->uid;
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
