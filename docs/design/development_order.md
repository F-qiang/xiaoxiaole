# Cocos + C++ 消除游戏开发顺序清单

本文档基于 `docs/design/spec.md` 整理，目标是把规范中的需求拆成**可直接开工的代码实现顺序**，并作为 `docs/plan/PR1-board-base.md ~ PR5-ui-reset-config-cleanup.md` 的上游依据，用于指导 C++ 类、接口、UI 节点和资源的开发次序。

---

## 一、总体开发原则

1. **先数据后表现**：先把 C++ 核心数据结构和结算逻辑做稳，再补 Cocos 视图层。
2. **先闭环后扩展**：先完成“交换→消除→下落→连锁→胜负”的最小闭环，再做特效、障碍物和结算 UI。
3. **先单局后配置化**：Demo 阶段优先支持单固定关卡，JSON 配置即可满足需求，暂不优先接 SQLite。
4. **逻辑与动画解耦**：C++ 只负责状态和规则，Cocos 只负责节点表现和动画。
5. **所有操作都要可回退、可锁定、可重置**：避免后期状态混乱。

---

## 二、推荐开发顺序总览

### 阶段 1：棋盘底座与基础显示

先做这些：

1. `GameBoard` 基础类
2. `Cell` / `Piece` / `Obstacle` 数据结构
3. 棋盘初始化接口
4. Cocos 棋盘节点容器
5. 棋子基础显示节点
6. 固定关卡 JSON 读取

### 阶段 2：基础交互与交换

再做这些：

1. 选中状态管理
2. 相邻判断接口
3. 交换接口
4. 无效交换回退
5. 步数统计
6. 输入锁定状态

### 阶段 3：三消、下落、连锁

接着做这些：

1. 消除扫描器 `MatchFinder`
2. 消除标记与执行
3. 下落与补充接口
4. 连锁检测循环
5. 消除动画和下落动画

### 阶段 4：特效与障碍物

然后做这些：

1. 特效生成规则
2. 特效触发器 `EffectResolver`
3. 炸药桶主动触发
4. 螺旋炸弹（Color Bomb）生成与触发
5. 标准组合技处理（火箭 + 炸弹、炸弹 + 炸弹、螺旋 + 任意）
6. 特效连锁扩散
7. 障碍物清除和计数
8. 胜负判定

### 阶段 5：UI、重置与收尾

最后做这些：

1. 顶部信息区
2. 结算弹窗
3. 重新开始
4. 完整对局测试
5. 资源管理和稳定性收尾

---

## 三、C++ 类开发顺序

### 1. 第一批：核心数据类

先创建以下类和枚举：

#### `Cell`
负责单个格子状态。

建议字段：
- `row`
- `col`
- `pieceType`
- `colorType`
- `effectType`
- `hasObstacle`
- `isEmpty`
- `isSelected`

#### `Piece`
负责棋子基础属性。

建议字段：
- `pieceType`
- `colorType`
- `effectType`
- `isMatched`
- `isMovable`

#### `Obstacle`
负责障碍物基础状态。

建议字段：
- `isDestroyed`
- `hp`（先保留扩展位，Demo 可固定为 1）
- `type`

#### 枚举建议
- `CellState`：`EmptyCell`、`NormalPiece`、`SpecialPiece`、`Obstacle`
- `PieceType`：普通棋子、特效棋子
- `EffectType`：无特效、范围爆炸、行清除、列清除、炸药桶、螺旋炸弹
- `GameState`：`Idle`、`Selecting`、`Swapping`、`Resolving`、`Falling`、`Checking`、`Win`、`Lose`

---

### 2. 第二批：棋盘主控类

#### `GameBoard`
这是最先实现的核心类。

职责：
- 管理 8×9 棋盘二维数组
- 提供格子读取与写入
- 提供交换、清除、下落、补充接口
- 判断盘面是否稳定

接口：
- `InitializeBoard()`：初始化棋盘
- `ResetBoard()`：重置棋盘
- `GetCell(row, col)`：获取格子
- `SwapCells(a, b)`：交换格子内容
- `ClearCells(list)`：清除格子
- `ApplyGravity()`：执行下落
- `RefillBoard()`：顶部补充新棋子
- `IsStable()`：判断盘面稳定

---

### 3. 第三批：消除检测类

#### `MatchFinder`
负责全局扫描消除组合。

接口：
- `FindMatches()`：全局扫描消除
- `FindHorizontalMatches()`：横向检测
- `FindVerticalMatches()`：纵向检测
- `FindSquareMatches()`：2×2 检测
- `FindTShapeMatches()`：T 型检测
- `FindFiveLineMatches()`：5 连检测

说明：
- 先实现三连检测
- 再扩展 4 连、5 连、T 型
- 特效生成也建议放到这里输出候选结果

---

### 4. 第四批：特效解析类

#### `EffectResolver`
负责特效触发、扩散、范围计算。

接口：
- `ResolveAutoEffects()`：处理自动特效
- `ResolveBombBarrel()`：处理炸药桶
- `ResolveColorBomb()`：处理螺旋炸弹 / Color Bomb
- `BuildEffectRange()`：计算特效作用范围
- `EnqueueLinkedEffects()`：加入连锁特效队列
- `ApplyEffectMarks()`：统一打标
- `ResolveStandardCombos()`：处理标准组合技

说明：
- 这里建议使用队列式广度优先扩散
- 炸药桶不进入自动连锁队列
- 炸药桶由玩家主动交换直接触发
- 螺旋炸弹由直线 5 连生成，触发后清除全棋盘同色
- 标准组合技需单独处理，不能仅当作普通扩散

---

### 5. 第五批：回合与关卡类

#### `TurnSystem`
负责一次玩家操作从选中到结算的全过程。

接口：
- `HandleSelect()`：处理选中
- `HandleSwap()`：处理交换
- `HandleInvalidSwap()`：处理无效回退
- `LockInput()`：锁定输入
- `UnlockInput()`：解锁输入
- `AddStep()`：步数加一
- `ExecuteTurn()`：执行完整回合流程

#### `LevelSystem`
负责关卡状态、障碍物数量、胜负判定和重置。

接口：
- `LoadLevel()`：加载关卡
- `ResetLevel()`：重置关卡
- `GetRemainingObstacleCount()`：获取剩余障碍物数量
- `CheckWin()`：胜利判定
- `CheckLose()`：失败判定

---

### 6. 第六批：视图与动画桥接类

#### `BoardView`
负责把 C++ 状态变化变成 Cocos 节点变化。

接口：
- `CreateBoardNodes()`：创建棋盘节点
- `CreatePieceNode()`：创建棋子节点
- `PlaySwapAnimation()`：播放交换动画
- `PlayClearAnimation()`：播放消除动画
- `PlayFallAnimation()`：播放下落动画
- `PlayEffectAnimation()`：播放特效动画
- `UpdateBoardView()`：同步棋盘显示

#### `HudView`
负责顶部信息区。

接口：
- `UpdateObstacleCount()`：更新剩余障碍物数量
- `UpdateStepCount()`：更新步数

#### `ResultPopup`
负责结算弹窗。

接口：
- `ShowResult()`：显示结果
- `SetWin()`：设置胜利状态
- `SetLose()`：设置失败状态
- `BindRestart()`：绑定重新开始按钮

---

## 四、UI 节点开发顺序

### 1. 场景根节点
先建立一个主场景，包含以下节点：

- `Canvas`
- `BoardRoot`
- `HudRoot`
- `PopupRoot`
- `EffectRoot`

### 2. 棋盘节点
`BoardRoot` 包含：

- `BoardBackground`
- `PieceLayer`
- `ObstacleLayer`
- `EffectLayer`
- `SelectionLayer`

### 3. 顶部 UI
`HudRoot` 包含：

- `ObstacleCountText`
- `StepCountText`

### 4. 结算弹窗
`PopupRoot` 包含：

- `ResultPanel`
- `ResultTitle`
- `ResultObstacleCountText`
- `ResultStepCountText`
- `RestartButton`

---

## 五、资源开发顺序

### 第一批资源
- 棋子基础图片
- 棋盘底板
- 障碍物图片

### 第二批资源
- 选中高亮效果
- 消除特效
- 下落表现
- 交换动画素材

### 第三批资源
- 范围爆炸特效
- 行清除特效
- 列清除特效
- 炸药桶爆炸特效
- 螺旋炸弹 / Color Bomb 特效
- 标准组合技特效

### 第四批资源
- 结算弹窗背景
- 胜利 / 失败图标
- 按钮资源

---

## 六、按 PR 的具体落地顺序

### PR1（`docs/plan/PR1-board-base.md`）对应开发顺序
1. 建 `Cell` / `Piece` / `Obstacle` 枚举和结构体
2. 建 `GameBoard`
3. 做棋盘初始化
4. 做固定关卡 JSON 读取
5. 做棋盘基础渲染

### PR2（`docs/plan/PR2-interaction-swap.md`）对应开发顺序
1. 做选中状态
2. 做相邻交换
3. 做无效回退
4. 做步数统计
5. 做输入锁定

### PR3（`docs/plan/PR3-clear-fall-refill.md`）对应开发顺序
1. 做三连检测
2. 做消除标记
3. 做消除动画
4. 做下落与补充
5. 做连锁检测
6. 做顶部障碍物阻挡补充

### PR4（`docs/plan/PR4-effects-obstacles-result.md`）对应开发顺序
1. 做特效生成
2. 做特效触发
3. 做特效连锁扩散
4. 做障碍物清除
5. 做胜负判定
6. 做螺旋炸弹与组合技

### PR5（`docs/plan/PR5-ui-reset-config-cleanup.md`）对应开发顺序
1. 做顶部信息区
2. 做结算弹窗
3. 做重置逻辑
4. 做 JSON 配置文件化
5. 做资源管理和完整联调

---

## 七、建议的接口依赖关系

### 先实现的底层依赖

1. `Cell`
2. `Piece`
3. `Obstacle`
4. `GameBoard`
5. `MatchFinder`
6. `EffectResolver`
7. `TurnSystem`
8. `LevelSystem`
9. `BoardView`
10. `HudView`
11. `ResultPopup`

### 调用关系

- `TurnSystem` 调用 `GameBoard`、`MatchFinder`、`EffectResolver`、`LevelSystem`
- `BoardView` 订阅 `TurnSystem` 和 `GameBoard` 的状态变化
- `HudView` 订阅 `LevelSystem` 和 `TurnSystem`
- `ResultPopup` 订阅 `LevelSystem`

---

## 八、最小可玩版本的代码落地顺序

如果只追求最快跑通 Demo，顺序实现：

1. `GameBoard`
2. `BoardView`
3. `TurnSystem`
4. `MatchFinder`
5. `EffectResolver`
6. `LevelSystem`
7. `HudView`
8. `ResultPopup`

这条顺序的原则是：

- 先能看见棋盘
- 再能点选交换
- 再能消除下落
- 再补特效和障碍物
- 最后补 UI 和配置化

---

## 九、结论

这套项目的最佳开发顺序是：

**数据结构 → 棋盘主控 → 交互交换 → 三消结算 → 特效障碍物 → UI 结算 → 配置与收尾**

如果按这个顺序推进，最不容易出现“逻辑、UI、规则互相打架”的情况，也最适合当前这个 2D 消除 Demo 的开发节奏。
