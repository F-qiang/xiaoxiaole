#pragma once

#include <array>
#include <cstddef>
#include <vector>

/**
 * 棋盘状态枚举。
 *
 * 当前阶段先保留最小必要状态，后续可扩展为更复杂的棋子与特效体系。
 */
enum class CellState {
    EmptyCell,
    NormalPiece,
    SpecialPiece,
    Obstacle
};

/**
 * 单个棋子类型。
 */
enum class PieceType {
    Normal,
    Obstacle
};

/**
 * 特效类型。
 */
enum class EffectType {
    None,
    Rocket,
    Bomb,
    ColorBomb,
    BombBarrel
};

/**
 * 单个格子数据。
 *
 * 这个结构体是棋盘逻辑的最小数据单元，后续所有交换、消除、下落和补充都围绕它展开。
 */
struct Cell {
    int row {0};
    int col {0};
    CellState state {CellState::EmptyCell};
    PieceType pieceType {PieceType::Normal};
    EffectType effectType {EffectType::None};
    int colorType {0};
    bool hasObstacle {false};
    bool isSelected {false};
    int uid {0};
};

/**
 * 棋盘核心数据结构。
 *
 * 当前 Demo 目标固定为 8 行 × 9 列满格棋盘。
 * 逻辑层会在这个类里完成数据存取、交换、消除和下落的基础骨架。
 */
class GameBoard final {
public:
    /** 棋盘行数。 */
    static constexpr std::size_t ROWS = 8;
    /** 棋盘列数。 */
    static constexpr std::size_t COLS = 9;

    GameBoard();

    void initialize();
    void reset();

    Cell* getCell(int row, int col);
    const Cell* getCell(int row, int col) const;

    static bool isAdjacent(const Cell& lhs, const Cell& rhs);
    bool swapCells(Cell& lhs, Cell& rhs);
    void clearCell(Cell& cell);
    void toggleSelection(Cell& cell);
    void clearSelection();
    Cell* getSelectedCell();
    const Cell* getSelectedCell() const;
    bool hasMatchAt(int row, int col) const;
    bool hasAnyMatch() const;
    bool collectMatches(std::vector<Cell>& matchedCells) const;
    bool hasAnyValidMove() const;
    void clearObstacle(Cell& cell);
    std::size_t countObstacles() const;
    void collapseAndRefill();
    int getCellUid(int row, int col) const;

    static bool isBombCandy(const Cell& cell);
    static bool isVerticalClearCandy(const Cell& cell);
    static bool isHorizontalClearCandy(const Cell& cell);
    static bool isSpecialCandy(const Cell& cell);
    static bool canMergeToSpecial(const Cell& lhs, const Cell& rhs);

private:
    static constexpr int SPECIAL_DROP_PROBABILITY = 12;
    std::array<std::array<Cell, COLS>, ROWS> mCells;
    Cell* mSelectedCell {nullptr};

    void rebuildBoard();
    static bool isValidPosition(int row, int col);
};