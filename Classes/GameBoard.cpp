#include "GameBoard.h"

#include <algorithm>
#include <cstdlib>
#include <random>
#include <vector>

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
    rebuildBoard();
}

void GameBoard::reset() {
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

    std::swap(lhs.state, rhs.state);
    std::swap(lhs.pieceType, rhs.pieceType);
    std::swap(lhs.effectType, rhs.effectType);
    std::swap(lhs.colorType, rhs.colorType);
    std::swap(lhs.hasObstacle, rhs.hasObstacle);
    std::swap(lhs.isSelected, rhs.isSelected);
    std::swap(lhs.uid, rhs.uid);
    return true;
}

void GameBoard::clearCell(Cell& cell) {
    cell.state = CellState::EmptyCell;
    cell.pieceType = PieceType::Normal;
    cell.effectType = EffectType::None;
    cell.colorType = 0;
    cell.hasObstacle = false;
    cell.isSelected = false;
    cell.uid = 0;
}

void GameBoard::toggleSelection(Cell& cell) {
    if (mSelectedCell != nullptr && mSelectedCell != &cell) {
        mSelectedCell->isSelected = false;
    }

    cell.isSelected = !cell.isSelected;
    mSelectedCell = cell.isSelected ? &cell : nullptr;
}

void GameBoard::clearSelection() {
    if (mSelectedCell != nullptr) {
        mSelectedCell->isSelected = false;
        mSelectedCell = nullptr;
    }
}

Cell* GameBoard::getSelectedCell() {
    return mSelectedCell;
}

const Cell* GameBoard::getSelectedCell() const {
    return mSelectedCell;
}

bool GameBoard::hasMatchAt(int row, int col) const {
    const auto* cell = getCell(row, col);
    if (cell == nullptr || cell->state != CellState::NormalPiece) {
        return false;
    }

    const int color = cell->colorType;
    int horizontal = 1;
    for (int offset = 1; ; ++offset) {
        const auto* left = getCell(row, col - offset);
        if (left == nullptr || left->state != CellState::NormalPiece || left->colorType != color) {
            break;
        }
        ++horizontal;
    }
    for (int offset = 1; ; ++offset) {
        const auto* right = getCell(row, col + offset);
        if (right == nullptr || right->state != CellState::NormalPiece || right->colorType != color) {
            break;
        }
        ++horizontal;
    }
    if (horizontal >= 3) {
        return true;
    }

    int vertical = 1;
    for (int offset = 1; ; ++offset) {
        const auto* up = getCell(row - offset, col);
        if (up == nullptr || up->state != CellState::NormalPiece || up->colorType != color) {
            break;
        }
        ++vertical;
    }
    for (int offset = 1; ; ++offset) {
        const auto* down = getCell(row + offset, col);
        if (down == nullptr || down->state != CellState::NormalPiece || down->colorType != color) {
            break;
        }
        ++vertical;
    }
    return vertical >= 3;
}

bool GameBoard::hasAnyMatch() const {
    std::vector<Cell> matchedCells;
    return collectMatches(matchedCells);
}

bool GameBoard::collectMatches(std::vector<Cell>& matchedCells) const {
    matchedCells.clear();

    bool visited[ROWS][COLS] = {};

    for (std::size_t row = 0; row < ROWS; ++row) {
        for (std::size_t col = 0; col < COLS; ++col) {
            const auto* cell = getCell(static_cast<int>(row), static_cast<int>(col));
            if (cell == nullptr || cell->state != CellState::NormalPiece) {
                continue;
            }

            const int color = cell->colorType;
            int left = static_cast<int>(col);
            while (left > 0) {
                const auto* prev = getCell(static_cast<int>(row), left - 1);
                if (prev == nullptr || prev->state != CellState::NormalPiece || prev->colorType != color) {
                    break;
                }
                --left;
            }
            int right = static_cast<int>(col);
            while (right + 1 < static_cast<int>(COLS)) {
                const auto* next = getCell(static_cast<int>(row), right + 1);
                if (next == nullptr || next->state != CellState::NormalPiece || next->colorType != color) {
                    break;
                }
                ++right;
            }
            if (right - left + 1 >= 3) {
                for (int c = left; c <= right; ++c) {
                    if (!visited[row][static_cast<std::size_t>(c)]) {
                        visited[row][static_cast<std::size_t>(c)] = true;
                        matchedCells.push_back(*getCell(static_cast<int>(row), c));
                    }
                }
            }

            int up = static_cast<int>(row);
            while (up > 0) {
                const auto* prev = getCell(up - 1, static_cast<int>(col));
                if (prev == nullptr || prev->state != CellState::NormalPiece || prev->colorType != color) {
                    break;
                }
                --up;
            }
            int down = static_cast<int>(row);
            while (down + 1 < static_cast<int>(ROWS)) {
                const auto* next = getCell(down + 1, static_cast<int>(col));
                if (next == nullptr || next->state != CellState::NormalPiece || next->colorType != color) {
                    break;
                }
                ++down;
            }
            if (down - up + 1 >= 3) {
                for (int r = up; r <= down; ++r) {
                    if (!visited[static_cast<std::size_t>(r)][col]) {
                        visited[static_cast<std::size_t>(r)][col] = true;
                        matchedCells.push_back(*getCell(r, static_cast<int>(col)));
                    }
                }
            }
        }
    }

    return !matchedCells.empty();
}

void GameBoard::collapseAndRefill() {
    for (std::size_t col = 0; col < COLS; ++col) {
        int writeRow = static_cast<int>(ROWS) - 1;
        for (int row = static_cast<int>(ROWS) - 1; row >= 0; --row) {
            auto& cell = mCells[static_cast<std::size_t>(row)][col];
            if (cell.state == CellState::Obstacle) {
                cell.row = row;
                cell.col = static_cast<int>(col);
                writeRow = row - 1;
                continue;
            }
            if (cell.state == CellState::NormalPiece) {
                if (writeRow != row) {
                    mCells[static_cast<std::size_t>(writeRow)][col] = cell;
                    mCells[static_cast<std::size_t>(writeRow)][col].row = writeRow;
                    mCells[static_cast<std::size_t>(writeRow)][col].col = static_cast<int>(col);
                    clearCell(cell);
                }
                --writeRow;
            }
        }

        for (int row = writeRow; row >= 0; --row) {
            auto& cell = mCells[static_cast<std::size_t>(row)][col];
            if (cell.state == CellState::Obstacle) {
                continue;
            }
            cell.row = row;
            cell.col = static_cast<int>(col);
            cell.state = CellState::NormalPiece;
            cell.pieceType = PieceType::Normal;
            cell.effectType = EffectType::None;
            cell.colorType = std::rand() % NORMAL_COLOR_COUNT;
            cell.hasObstacle = false;
            cell.isSelected = false;
            cell.uid = static_cast<int>((row + 1) * 100 + col + 1);
        }
    }
}

int GameBoard::getCellUid(int row, int col) const {
    const auto* cell = getCell(row, col);
    return cell == nullptr ? 0 : cell->uid;
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
                cell.uid = static_cast<int>((row + 1) * 100 + col + 1);
            } else {
                cell.state = CellState::NormalPiece;
                cell.pieceType = PieceType::Normal;
                cell.colorType = generator.nextColor(mCells, row, col);
                cell.hasObstacle = false;
                cell.uid = static_cast<int>((row + 1) * 100 + col + 1);
            }
        }
    }
}

bool GameBoard::isValidPosition(int row, int col) {
    return row >= 0 && row < static_cast<int>(ROWS) && col >= 0 && col < static_cast<int>(COLS);
}
