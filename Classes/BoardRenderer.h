#pragma once

#include <array>

#include "2d/CCNode.h"

class GameBoard;

/**
 * 棋盘渲染器。
 *
 * 负责把 `GameBoard` 里的逻辑数据转换成可视节点，并把普通棋子与障碍棋子
 * 按照固定布局绘制到场景中。
 */
class BoardRenderer final {
public:
    /**
     * 创建棋盘渲染器。
     */
    BoardRenderer();

    /**
     * 渲染整张棋盘。
     *
     * @param parent 父节点，渲染出来的棋盘内容会挂到这个节点下。
     * @param board 棋盘逻辑数据。
     */
    void render(cocos2d::Node* parent, const GameBoard& board, bool animateDrop = false);

private:
    /** 棋盘行数。 */
    static constexpr int BOARD_ROWS = 8;
    /** 棋盘列数。 */
    static constexpr int BOARD_COLS = 9;
    /** 单个格子的显示尺寸。 */
    static constexpr float CELL_SIZE = 64.0F;
    /** 格子之间的显示间距。 */
    static constexpr float BOARD_MARGIN = 4.0F;
    /** 棋子缩放比例。 */
    static constexpr float PIECE_SCALE = 0.58F;
    /** 背景底图资源路径。 */
    static constexpr const char* BOARD_BG_FILE = "picture/img_ig_candy/tileA.png";
    /** 棋盘整体下移偏移。 */
    static constexpr float BOARD_Y_OFFSET = -40.0F;
    /** 棋盘等比例缩放。 */
    static constexpr float BOARD_SCALE_FACTOR = 1.16F;

    /** 普通棋子资源路径表。 */
    std::array<const char*, 4> mNormalPieceFiles;
    /** 障碍棋子的资源路径。 */
    const char* mObstaclePieceFile;
    /** 炸弹棋子资源路径。 */
    const char* mBombPieceFile;
    /** 上下消除棋子资源路径。 */
    const char* mVerticalPieceFile;
    /** 左右消除棋子资源路径。 */
    const char* mHorizontalPieceFile;
    /** 上一次渲染时各格棋子的 UID 快照。 */
    std::array<std::array<int, BOARD_COLS>, BOARD_ROWS> mPreviousUids{};
};
