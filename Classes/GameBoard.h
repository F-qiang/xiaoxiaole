#pragma once

#include <array>
#include <cstddef>

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

    /**
     * 创建棋盘对象，并在构造时完成初始盘面构建。
     */
    GameBoard();

    /**
     * 初始化棋盘内容。
     *
     * 当前版本先生成一个基础满格棋盘，后续再接入关卡配置。
     */
    void initialize();

    /**
     * 重置棋盘到初始状态。
     */
    void reset();

    /**
     * 读取指定坐标的格子。
     *
     * 若坐标越界，返回 nullptr。
     */
    Cell* getCell(int row, int col);
    const Cell* getCell(int row, int col) const;

    /**
     * 判断两个格子是否相邻。
     * 规则仅允许上下左右四邻域。
     */
    static bool isAdjacent(const Cell& lhs, const Cell& rhs);

    /**
     * 交换两个格子的内容。
     * 返回 true 表示交换成功。
     */
    bool swapCells(Cell& lhs, Cell& rhs);

    /**
     * 将指定格子清空。
     */
    void clearCell(Cell& cell);

private:
    /**
     * 棋盘二维缓存，保存当前所有格子的运行时状态。
     */
    std::array<std::array<Cell, COLS>, ROWS> mCells;

    /**
     * 重新构建完整棋盘。
     */
    void rebuildBoard();

    /**
     * 判断坐标是否在棋盘范围内。
     */
    static bool isValidPosition(int row, int col);
};