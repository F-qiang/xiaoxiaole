#pragma once

#include "2d/CCLabel.h"
#include "2d/CCScene.h"
#include <vector>

class BoardRenderer;
class GameBoard;
struct Cell;

class GameScene final : public cocos2d::Scene {
public:
    ~GameScene() override;
    static cocos2d::Scene* createScene();
    bool init() override;

    CREATE_FUNC(GameScene);

private:
    void createBoardPlaceholder();
    void updateStepLabel();
    void updateGoalLabel();
    void refreshBoard(bool animateDrop = false);
    void resolveMatches();
    void clearAdjacentObstacles(const std::vector<Cell>& matchedCells);
    void placeSpecialCandy(const std::vector<Cell>& matchedCells);
    bool isLineMatch(const std::vector<Cell>& matchedCells) const;
    bool isTOrLMatch(const std::vector<Cell>& matchedCells) const;
    void playSpecialBurst(int row, int col, cocos2d::Color3B color, float scale, float duration, int zOrder);
    void triggerSpecialCandy(Cell& specialCell);
    void triggerSpecialCombo(Cell& first, Cell& second);
    void clearLineAt(int row, int col, bool vertical);
    void clearCrossAt(int row, int col);
    void checkLevelState();
    void showResultMessage(const char* message);
    void playSwapFeedback(const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    void playClearFeedback();
    void playDropAnimation();
    void runCollapseAndRefresh();
    cocos2d::Vec2 cellToWorld(int row, int col) const;
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    BoardRenderer* mBoardRenderer {nullptr};
    GameBoard* mBoardModel {nullptr};
    cocos2d::Label* mStepLabel {nullptr};
    cocos2d::Label* mGoalLabel {nullptr};
    cocos2d::Label* mResultLabel {nullptr};
    int mStepCount {0};
    int mClearedCount {0};
    int mTargetClearCount {20};
    int mMaxStepCount {12};
    int mPendingSpecialRow {-1};
    int mPendingSpecialCol {-1};
    bool mIsAnimating {false};
    bool mLevelFinished {false};
};
