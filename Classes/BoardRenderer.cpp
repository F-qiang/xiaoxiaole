#include "BoardRenderer.h"

#include "GameBoard.h"

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
    , mObstaclePieceFile("picture/img_game_common/goal_Animalice.png") {}

void BoardRenderer::render(Node* parent, const GameBoard& board) {
    if (parent == nullptr) {
        return;
    }

    auto winSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    const float boardWidth = BOARD_COLS * CELL_SIZE + (BOARD_COLS - 1) * BOARD_MARGIN;
    const float boardHeight = BOARD_ROWS * CELL_SIZE + (BOARD_ROWS - 1) * BOARD_MARGIN;
    const float startX = origin.x + (winSize.width - boardWidth) * 0.5F;
    const float startY = origin.y + (winSize.height - boardHeight) * 0.5F;

    auto boardNode = Node::create();
    parent->addChild(boardNode);

    for (int row = 0; row < BOARD_ROWS; ++row) {
        for (int col = 0; col < BOARD_COLS; ++col) {
            const float x = startX + col * (CELL_SIZE + BOARD_MARGIN) + CELL_SIZE * 0.5F;
            const float y = startY + (BOARD_ROWS - 1 - row) * (CELL_SIZE + BOARD_MARGIN) + CELL_SIZE * 0.5F;

            const auto* cell = board.getCell(row, col);
            if (cell == nullptr) {
                continue;
            }

            auto piece = Sprite::create(cell->state == CellState::Obstacle
                ? mObstaclePieceFile
                : mNormalPieceFiles[static_cast<std::size_t>(cell->colorType % mNormalPieceFiles.size())]);
            if (piece != nullptr) {
                piece->setPosition(Vec2(x, y));
                piece->setScale(PIECE_SCALE);
                boardNode->addChild(piece);
            }
        }
    }
}
