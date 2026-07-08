#include "GameBoard.h"

#include <cstdlib>

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
    for (std::size_t row = 0; row < ROWS; ++row) {
        for (std::size_t col = 0; col < COLS; ++col) {
            auto& cell = mCells[row][col];
            cell.row = static_cast<int>(row);
            cell.col = static_cast<int>(col);
            cell.state = CellState::NormalPiece;
            cell.pieceType = PieceType::Normal;
            cell.effectType = EffectType::None;
            cell.colorType = static_cast<int>((row + col) % 5);
            cell.hasObstacle = false;
            cell.isSelected = false;
        }
    }
}

bool GameBoard::isValidPosition(int row, int col) {
    return row >= 0 && row < static_cast<int>(ROWS) && col >= 0 && col < static_cast<int>(COLS);
}
