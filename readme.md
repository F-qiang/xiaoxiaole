# 消消乐（Cocos + C++）

一个基于 **Cocos + C++** 的 2D 消除类 Demo 项目，包含棋盘交换、三消、障碍物清除、连锁结算、重置与结算界面等核心玩法。

## 项目定位

本项目用于实现一个规则明确、结构清晰、便于扩展的消除类小游戏 Demo，适合作为：

- Cocos + C++ 项目入门实践
- 2D 消除玩法原型验证
- 规则驱动型游戏架构参考
- 后续关卡与特效扩展基础

## 核心玩法

- 8 × 9 棋盘
- 4 种普通棋子
- 1 种障碍棋子
- 普通棋子交换
- 横向 / 纵向三连及以上消除
- 下落与顶部补充
- 连锁消除
- 障碍物清除
- 胜负判定与结算
- 重新开始与重置

## 文档结构

- `docs/design/spec.md`：设计规范，描述规则、架构与边界
- `docs/design/development_order.md`：由规范推导出的开发顺序清单
- `docs/plan/PR1-board-base.md ~ PR5-ui-reset-config-cleanup.md`：具体任务拆分
- 资源目录统一使用 `resourses/`
- `docs/game_rule.md`：玩法规则说明书

## 开发顺序

建议按以下顺序实现：

1. 棋盘底座与基础显示
2. 基础交互与交换逻辑
3. 三消、下落与连锁
4. 障碍物清除与结算逻辑
5. UI、重置、配置与工程化收尾

## 目录说明

```text
project/
├─ docs/
│  ├─ design/
│  │  ├─ spec.md
│  │  └─ development_order.md
│  ├─ plan/
│  │  ├─ PR1-board-base.md
│  │  ├─ PR2-interaction-swap.md
│  │  ├─ PR3-clear-fall-refill.md
│  │  ├─ PR4-effects-obstacles-result.md
│  │  └─ PR5-ui-reset-config-cleanup.md
│  └─ game_rule.md
└─ readme.md
```

## 技术选择

- **引擎**：Cocos
- **核心逻辑**：C++
- **配置文件**：JSON
- **本地存档**：先按需求决定，Demo 阶段优先保持轻量

## 当前实现目标

本项目当前阶段重点是完成一个可稳定运行的 Demo，确保：

- 棋盘能够正确显示
- 交换与消除流程可跑通
- 障碍物清除与胜负规则清晰
- 结算与重置可用
- 文档、计划与实现保持一致

## 后续扩展方向

- 更多关卡配置
- 更多障碍物类型
- 更多特效组合
- 关卡编辑与导出工具
- 本地存档与进度系统

## 备注

- 该项目以文档驱动开发为主，先按 `spec.md` 与 `development_order.md` 完成基础架构，再逐步实现 `plan/` 中的任务。
- 若后续需要增加关卡、道具或更复杂的结算规则，先更新设计规范，再同步调整开发计划。
