# Cocos + C++ 消除游戏架构方案

## 目录

- [1. 目标与范围（Scope and Goals）](#1-目标与范围scope-and-goals)
- [2. 总体架构（Overall Architecture）](#2-总体架构overall-architecture)
  - [2.1 分层原则（Layering Principles）](#21-分层原则layering-principles)
  - [2.2 分层总结（Layering Summary）](#22-分层总结layering-summary)
- [3. 技术选型（Technology Choices）](#3-技术选型technology-choices)
  - [3.1 配置与存档边界（Config and Save Scope）](#31-配置与存档边界config-and-save-scope)
- [4. 核心模块设计（Core Modules）](#4-核心模块设计core-modules)
  - [4.1 棋盘管理器（GameBoard / BoardManager）](#41-棋盘管理器gameboard--boardmanager)
  - [4.2 消除检测器（MatchFinder / MatchDetector）](#42-消除检测器matchfinder--matchdetector)
  - [4.3 特效解析器（EffectResolver / EffectManager）](#43-特效解析器effectresolver--effectmanager)
  - [4.4 回合系统（TurnSystem / TurnManager）](#44-回合系统turnsystem--turnmanager)
  - [4.5 关卡系统（LevelSystem / LevelManager）](#45-关卡系统levelsystem--levelmanager)
  - [4.6 棋盘视图（BoardView / BoardRenderer）](#46-棋盘视图boardview--boardrenderer)
- [5. 数据层设计（Data Layer Design）](#5-数据层设计data-layer-design)
  - [5.1 运行时数据（Runtime Data）](#51-运行时数据runtime-data)
  - [5.2 配置数据（Config Data）](#52-配置数据config-data)
  - [5.3 存档数据（Save Data）](#53-存档数据save-data)
- [6. 棋盘模型设计（Board Model Design）](#6-棋盘模型设计board-model-design)
  - [6.1 盘面表示（Board Representation）](#61-盘面表示board-representation)
  - [6.2 格子对象字段（Cell Fields）](#62-格子对象字段cell-fields)
  - [6.3 状态建议（State Recommendations）](#63-状态建议state-recommendations)
  - [6.4 下落与补充（Fall and Refill）](#64-下落与补充fall-and-refill)
- [7. 核心结算流程（Core Resolution Flow）](#7-核心结算流程core-resolution-flow)
- [8. 状态机设计（State Machine Design）](#8-状态机设计state-machine-design)
- [9. 特效扩散设计（Effect Propagation Design）](#9-特效扩散设计effect-propagation-design)
- [10. 障碍物设计（Obstacle Design）](#10-障碍物设计obstacle-design)
- [11. 胜负判定设计（Win/Lose Design）](#11-胜负判定设计winlose-design)
- [12. 与 Cocos 的对接方式（Cocos Integration）](#12-与-cocos-的对接方式cocos-integration)
- [13. 推荐目录结构（Recommended Directory Structure）](#13-推荐目录结构recommended-directory-structure)
- [14. 实现优先级（Implementation Priority）](#14-实现优先级implementation-priority)

---

## 1. 目标与范围（Scope and Goals）

本文档用于说明一个基于 **Cocos + C++** 的 2D 消除类游戏架构方案，重点覆盖：

- 棋盘数据建模
- 消除与连锁结算流程
- 特效棋子与障碍物规则
- 视图层与逻辑层分离
- 配置文件与存档边界
- 关卡配置、存档与运行时数据的划分

---

## 2. 总体架构（Overall Architecture）

建议采用“三层分离”结构：

1. **数据层**：维护棋盘状态、关卡配置、存档数据。
2. **规则层**：处理交换、消除、连锁、特效、胜负判定等核心逻辑。
3. **表现层**：负责场景节点、动画、特效、UI、音效。

### 2.1 分层原则（Layering Principles）

- **逻辑先于表现**：所有规则先在 C++ 中计算完成，再驱动 Cocos 节点表现。
- **数据与视图解耦**：棋盘状态不直接依赖 UI 节点，UI 仅订阅状态变化事件。
- **结算统一处理**：每次操作完成后，按固定流水线执行消除、下落、连锁、胜负判定。

### 2.2 分层总结（Layering Summary）

- **运行时逻辑**放在 C++ 内存结构中
- **关卡配置与存档**放到配置文件中，Demo 阶段优先使用 JSON
- **动画、UI、音效**放在 Cocos 表现层

这样可以兼顾：

- 规则可控性
- 后续可扩展性
- 开发稳定性
- 配置修改成本

如果后续规则变复杂，这套架构也比较容易继续扩展。

---

## 3. 技术选型（Technology Choices）

### 3.1 配置与存档边界（Config and Save Scope）

对于当前一测 Demo，只有 1 个固定关卡时，**JSON 配置文件优先于 SQLite**：

- JSON 可以直接管理关卡布局、棋子颜色配置和初始参数
- 工程复杂度更低，接入成本更小
- 便于快速调试与频繁调整
- 后续若关卡数量增加、需要复杂查询或本地持久化，再升级 SQLite 即可

配置建议：

- **关卡配置**：JSON
- **棋子颜色权重**：JSON
- **初始参数**：JSON
- **本地存档**：可先用 JSON，后续再视需求切换 SQLite

不建议在 Demo 阶段直接引入数据库作为刚需依赖，以免挤占核心玩法开发时间。

---

## 4. 核心模块设计（Core Modules）

### 4.1 棋盘管理器（GameBoard / BoardManager）

`GameBoard` / 棋盘管理器 是棋盘数据中心，负责维护 8×9 盘面状态。

#### 职责

- 存储每个格子的内容
- 提供交换、清除、下落、补充接口
- 判断边界与占位状态
- 维护当前盘面是否处于稳定状态

#### 数据

- 棋子类型
- 棋子颜色
- 特效类型
- 障碍物状态
- 当前格子是否为空

### 4.2 消除检测器（MatchFinder / MatchDetector）

`MatchFinder` / 消除检测器 负责全局扫描可消除组合。

#### 职责

- 横向三连及以上检测
- 竖向三连及以上检测
- 2×2 方块检测
- 4 连、5 连、T 型等特效生成条件识别
- 连锁后再次扫描

### 4.3 特效解析器（EffectResolver / EffectManager）

`EffectResolver` / 特效解析器 负责特效触发与扩散。

#### 职责

- 范围爆炸
- 行清除
- 列清除
- 炸药桶触发
- 螺旋炸弹（Color Bomb）触发
- 自动特效连锁扩散
- 标准组合技处理

### 4.4 回合系统（TurnSystem / TurnManager）

`TurnSystem` / 回合系统 负责一次玩家操作的完整流程控制。

#### 职责

- 选中与交换
- 有效/无效交换判断
- 回弹处理
- 步数统计
- 操作锁定与解锁
- 操作结算驱动

### 4.5 关卡系统（LevelSystem / LevelManager）

`LevelSystem` / 关卡系统 负责关卡状态、胜负条件与重置。

#### 职责

- 初始化关卡数据
- 加载障碍物布局
- 统计剩余障碍物数量
- 胜利/失败判定
- 重置关卡

### 4.6 棋盘视图（BoardView / BoardRenderer）

`BoardView` / 棋盘视图 是表现层桥梁，负责把逻辑层变化转成视觉效果。

#### 职责

- 创建与销毁棋子节点
- 播放交换动画
- 播放消除动画
- 播放下落动画
- 播放特效动画
- 更新顶部信息区与结算弹窗

---

## 5. 数据层设计（Data Layer Design）

### 5.1 运行时数据（Runtime Data）

运行时数据应全部存放在 C++ 内存结构中。

#### 典型内容

- 棋盘二维数组
- 当前选中格
- 当前操作状态
- 当前步数
- 当前剩余障碍物数量
- 当前连锁结算队列

### 5.2 配置数据（Config Data）

Demo 阶段建议使用 JSON 配置文件。

#### 典型内容

- 关卡表
- 初始障碍物坐标
- 棋子颜色权重
- 特效生成规则
- 失败条件配置
- UI 默认参数

### 5.3 存档数据（Save Data）

Demo 阶段可先使用 JSON 保存；如后续需要复杂持久化，再切换 SQLite。

#### 内容

- 已解锁关卡
- 最高分
- 最高步数记录
- 音效开关
- 当前进度

---

## 6. 棋盘模型设计（Board Model Design）

### 6.1 盘面表示（Board Representation）

建议使用固定大小二维数组表示棋盘：

- 行数 / Rows：8
- 列数 / Columns：9

每个格子保存一个格子对象（Cell），记录其当前状态。

### 6.2 格子对象字段（Cell Fields）

建议字段如下：

- `row`：行索引
- `col`：列索引
- `pieceType`：棋子类型
- `colorType`：颜色类型
- `effectType`：特效类型
- `hasObstacle`：是否有障碍物
- `isEmpty`：是否为空格
- `isSelected`：是否被选中

### 6.3 状态建议（State Recommendations）

每个格子至少支持以下状态：

- `EmptyCell`：空格
- `NormalPiece`：普通棋子
- `SpecialPiece`：特效棋子
- `Obstacle`：障碍物

#### 当前 PR1 棋子资源约定

- 普通棋子共 4 种，对应资源：
  - `picture/img_game_common/goal_Animal_1_0.png`
  - `picture/img_game_common/goal_Animal_2_0.png`
  - `picture/img_game_common/goal_Animal_3_0.png`
  - `picture/img_game_common/goal_Animal_4_0.png`
- 障碍棋子资源：
  - `picture/img_game_common/goal_Animalice.png`

当前阶段先以这 4 种普通棋子 + 1 种障碍棋子的最小集合完成底座，后续再扩展更多内容资源。

如果未来扩展多层障碍物，可以在障碍物内部增加血量或层级字段。

### 6.4 下落与补充（Fall and Refill）

- 让上方棋子下落
- 顶部补充新棋子
- 当顶部有障碍物方块无法直接下落时，需要从隔壁补充

---

## 7. 核心结算流程（Core Resolution Flow）

单次玩家有效交换后，按以下顺序执行：

1. **交换校验**
   - 完成两枚棋子交换
   - 若不构成有效消除，则回弹并结束本次操作

2. **炸药桶判定**
   - 若交换双方包含炸药桶，直接进入炸药桶触发流程

3. **首轮消除扫描**
   - 全局扫描所有普通消除组合

4. **自动特效扩散**
   - 收集被波及的自动触发类特效
   - 使用队列逐层扩散
   - 标记所有受影响格子

5. **统一清除执行**
   - 一次性清除所有已标记普通棋子与障碍物
   - 同步触发对应动画

6. **特效生成**
   - 在原消除组合中心位置生成对应特效棋子

7. **下落与补充**
   - 让上方棋子下落
   - 顶部补充新棋子
   - 当顶部有障碍物方块无法直接下落时，需要从隔壁补充

8. **连锁检测**
   - 下落后再次扫描全局
   - 若仍存在可消除组合，则回到步骤 3

9. **盘面稳定与判定**
   - 无可消除组合后结束本轮操作
   - 解锁玩家输入
   - 执行胜负判定

---

## 8. 状态机设计（State Machine Design）

在 `TurnSystem` 中使用有限状态机控制整个操作过程。

### 8.1 推荐状态（Recommended States）

- `Idle`：空闲
- `Selecting`：选中中
- `Swapping`：交换中
- `Resolving`：结算中
- `Falling`：下落中
- `Checking`：检测中
- `Win`：胜利
- `Lose`：失败

### 8.2 状态切换原则（State Transition Rules）

- 任意动画或结算过程中禁止进入新的交换
- 只有在盘面稳定后才能恢复到 `Idle`
- `Win` 和 `Lose` 状态下锁定所有输入

---

## 9. 特效扩散设计（Effect Propagation Design）

### 9.1 自动触发类特效（Auto Trigger Effects）

包括：

- 范围爆炸特效
- 行清除特效
- 列清除特效（开发补充）
- 螺旋炸弹 / Color Bomb（直线 5 连生成）

#### 扩散方式

建议使用队列进行广度优先扩散：

- 先将首轮被波及的自动特效入队
- 每次取出一个特效，计算其作用范围
- 若范围内包含新的自动特效，则继续入队
- 直到队列为空

### 9.2 炸药桶特效（Bomb Barrel Effect）

炸药桶属于手动触发类特效。

#### 规则

- 仅在玩家主动交换时触发
- 不参与自动连锁扩散队列
- 与其他特效相邻时可以引发进一步连锁

### 9.3 标准组合技（Standard Combos）

以下组合技需单独定义，不应仅归类为普通连锁扩散：

- **火箭 + 炸弹**：同时触发行清除与范围爆炸，按两者作用范围合并处理
- **炸弹 + 炸弹**：扩大范围清除效果，按更大范围执行
- **螺旋炸弹 + 任意特效**：触发全棋盘同色清除，并叠加目标特效效果

#### 螺旋炸弹 / Color Bomb

- 直线连续 5 枚同色普通棋子消除时生成
- 触发后清除全棋盘所有同色棋子
- 可与任意特效或棋子交换触发组合技

---

## 10. 障碍物设计（Obstacle Design）

### 10.1 当前版本（Current Version）

当前 Demo 先实现为：

- 单层
- 单类型
- 单次清除即销毁

### 10.2 清除方式（Removal Methods）

- 被特效范围覆盖时清除
- 与相邻普通棋子发生消除联动时清除

### 10.3 扩展预留（Future Extensions）

后续可扩展：

- 多血量障碍物
- 冰块类障碍物
- 锁链类障碍物
- 需要多次触发才消失的目标

---

## 11. 胜负判定设计（Win/Lose Design）

### 11.1 胜利条件（Win Condition）

当剩余障碍物数量为 0 时，判定胜利。

建议做法：

- 先标记胜利状态
- 等待当前连锁流程完全结束
- 再弹出胜利结算界面

### 11.2 失败条件（Lose Condition）

当盘面稳定后，且不存在任何一次普通棋子交换可形成消除时，判定失败。

#### 注意

- 仅统计普通棋子的一步交换
- 不把主动交换炸药桶后的爆炸纳入无解检测
- 失败判定只在完整操作循环结束后执行一次

---

## 12. 与 Cocos 的对接方式（Cocos Integration）

### 12.1 数据驱动视图（Data-Driven View）

C++ 逻辑层输出事件，Cocos 表现层订阅事件。

#### 典型事件 / Typical Events

- `OnSwapStart`：交换开始 / 交换开始事件
- `OnSwapFinished`：交换完成 / 交换完成事件
- `OnPieceRemoved`：棋子移除 / 棋子消除事件
- `OnPieceFall`：棋子下落 / 棋子下落事件
- `OnEffectTriggered`：特效触发 / 特效触发事件
- `OnObstacleCountChanged`：障碍物数量变化 / 剩余障碍物数量变化事件
- `OnTurnFinished`：回合结束 / 本次操作结束事件
- `OnWin`：胜利 / 胜利事件
- `OnLose`：失败 / 失败事件

### 12.2 表现层职责（View Responsibilities）

Cocos 只负责：

- 节点创建与销毁
- 动画播放
- 特效展示
- 音效播放
- UI 更新

不承担核心规则判断。

---

## 13. 推荐目录结构（Recommended Directory Structure）

```text
project/
├─ cocos/
├─ native/
│  ├─ core/
│  ├─ data/
│  ├─ config/
│  └─ utils/
├─ assets/
│  ├─ prefabs/
│  ├─ sprites/
│  ├─ animations/
│  └─ audio/
├─ docs/
│  ├─ rule.md
│  └─ design.md
└─ config/
   ├─ levels.json
   └─ game_balance.json
```

---

## 14. 实现优先级（Implementation Priority）

### 第一阶段：最小可玩版本

- 棋盘初始化
- 点击选中与交换
- 三连消除
- 下落与补充
- 障碍物清除
- 胜负判定

### 第二阶段：规则完善

- 4 连、5 连、T 型组合
- 特效棋子生成
- 特效触发与扩散
- 步数统计

### 第三阶段：内容扩展

- 多关卡
- 存档
- JSON 配置化
- 更丰富的视觉特效和音效

### 第四阶段：工程化增强

- 如后续存在多关卡与复杂存档需求，再将 JSON 存档升级为 SQLite
- 引入更完善的数据校验、热更新或编辑器导出流程
