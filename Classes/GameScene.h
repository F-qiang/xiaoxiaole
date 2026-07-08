#pragma once

#include "2d/CCLabel.h"
#include "2d/CCScene.h"

class BoardRenderer;
class GameBoard;

class GameScene final : public cocos2d::Scene {
public:
    ~GameScene() override;
    static cocos2d::Scene* createScene();
    bool init() override;

    CREATE_FUNC(GameScene);

private:
    void createBoardPlaceholder();
    void updateStepLabel();
    void refreshBoard(bool animateDrop = false);
    void resolveMatches();
    void playSwapFeedback(const cocos2d::Vec2& from, const cocos2d::Vec2& to);
    void playClearFeedback();
    void playDropAnimation();
    cocos2d::Vec2 cellToWorld(int row, int col) const;
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

    BoardRenderer* mBoardRenderer {nullptr};
    GameBoard* mBoardModel {nullptr};
    cocos2d::Label* mStepLabel {nullptr};
    int mStepCount {0};
    bool mIsAnimating {false};
};