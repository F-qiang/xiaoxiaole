#include "GameBoard.h"

#include <algorithm>
#include <cstdlib>
#include <random>

namespace {
constexpr int NORMAL_COLOR_COUNT = 4;

/**
 * 普通棋子颜色生成器。
 *
 * 这个小对象只负责为初始棋盘分配普通棋子颜色，并确保不会在横向或纵向
 * 直接生成三个连续同色棋子，从而避免初始化盘面自带可消除组合。
 */
class PieceGenerator final {
public:
    /**
     * 根据当前已生成的棋盘状态，返回一个合规的普通棋子颜色。
     *
     * @param cells 当前棋盘二维数组。
     * @param row 当前格子所在行。
     * @param col 当前格子所在列。
     * @return 可直接使用的颜色编号。
     */
    int nextColor(const std::array<std::array<Cell, GameBoard::COLS>, GameBoard::ROWS>& cells, std::size_t row, std::size_t col) {
        for (;;) {
            const int candidate = mDistribution(mRng);

            const bool hasLeftMatch = col >= 2
                && cells[row][col - 1].state == CellState::NormalPiece
                && cells[row][col - 2].state == CellState::NormalPiece
                && cells[row][col - 1].colorType == candidate
                && cells[row][col - 2].colorType == candidate;
            if (hasLeftMatch) {
                continue;
            }

            const bool hasUpMatch = row >= 2
                && cells[row - 1][col].state == CellState::NormalPiece
                && cells[row - 2][col].state == CellState::NormalPiece
                && cells[row - 1][col].colorType == candidate
                && cells[row - 2][col].colorType == candidate;
            if (hasUpMatch) {
                continue;
            }

            return candidate;
        }
    }

private:
    /**
     * 随机数引擎，用于生成普通棋子的颜色编号。
     */
    std::mt19937 mRng{std::random_device{}()};

    /**
     * 颜色编号分布范围，当前对应 4 种普通棋子颜色。
     */
    std::uniform_int_distribution<int> mDistribution{0, NORMAL_COLOR_COUNT - 1};
};
}

GameBoard::GameBoard() {
    initialize();
}

void GameBoard::initialize() {
    // 初始化时先构建一个满格棋盘，后续会接入具体关卡配置。
    rebuildBoard();
}

void GameBoard::reset() {
    // 重置逻辑先简单回到初始满格状态。
    rebuildBoard();
}

Cell* GameBoard::getCell(int row, int col) {
    if (!isValidPosition(row, col)) {
        return nullptr;
    }
    return &mCells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
}

const Cell* GameBoard::getCell(int row, int col) const {
    if (!isValidPosition(row, col)) {
        return nullptr;
    }
    return &mCells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
}

bool GameBoard::isAdjacent(const Cell& lhs, const Cell& rhs) {
    const int rowDelta = std::abs(lhs.row - rhs.row);
    const int colDelta = std::abs(lhs.col - rhs.col);
    return (rowDelta == 1 && colDelta == 0) || (rowDelta == 0 && colDelta == 1);
}

bool GameBoard::swapCells(Cell& lhs, Cell& rhs) {
    if (!isAdjacent(lhs, rhs)) {
        return false;
    }

    // 当前阶段只交换格子内容，后续扩展时可加入动画状态和交换锁。
    std::swap(lhs.state, rhs.state);
    std::swap(lhs.pieceType, rhs.pieceType);
    std::swap(lhs.effectType, rhs.effectType);
    std::swap(lhs.colorType, rhs.colorType);
    std::swap(lhs.hasObstacle, rhs.hasObstacle);
    return true;
}

void GameBoard::clearCell(Cell& cell) {
    // 清除格子时一并移除选中状态与障碍标记。
    cell.state = CellState::EmptyCell;
    cell.pieceType = PieceType::Normal;
    cell.effectType = EffectType::None;
    cell.colorType = 0;
    cell.hasObstacle = false;
    cell.isSelected = false;
}

void GameBoard::rebuildBoard() {
    PieceGenerator generator;

    for (std::size_t row = 0; row < ROWS; ++row) {
        for (std::size_t col = 0; col < COLS; ++col) {
            auto& cell = mCells[row][col];
            cell.row = static_cast<int>(row);
            cell.col = static_cast<int>(col);
            cell.effectType = EffectType::None;
            cell.isSelected = false;

            const bool isBottomFourRows = row >= ROWS - 4;
            const bool isMiddleOpeningRow = row == ROWS - 4 && col >= 3 && col <= 5;

            if (isBottomFourRows && !isMiddleOpeningRow) {
                cell.state = CellState::Obstacle;
                cell.pieceType = PieceType::Obstacle;
                cell.colorType = 0;
                cell.hasObstacle = true;
            } else {
                cell.state = CellState::NormalPiece;
                cell.pieceType = PieceType::Normal;
                cell.colorType = generator.nextColor(mCells, row, col);
                cell.hasObstacle = false;
            }
        }
    }
}

bool GameBoard::isValidPosition(int row, int col) {
    return row >= 0 && row < static_cast<int>(ROWS) && col >= 0 && col < static_cast<int>(COLS);
}
