# Git 工作流规范

**版本：** v1.0
**日期：** 2026-06-17

---

## 1. 分支策略

### 1.1 多人协作模式（5+ 人）

```
main (保护分支，发布版本)
  │
  └── develop (开发集成分支)
        │
        ├── feature/<模块>-<功能>      # 功能开发
        ├── bugfix/<模块>-<问题>       # 缺陷修复
        └── release/v<版本号>          # 发布准备
```

### 1.2 单人开发模式（简化）

```
main (发布版本)
  │
  └── develop (日常开发)
        │
        └── 可选：feature/<功能>        # 大功能隔离
```

---

## 2. 分支命名规范

| 分支类型 | 命名格式 | 示例 |
|----------|----------|------|
| 功能 | `feature/<模块>-<功能简述>` | `feature/metering-self-learning` |
| 修复 | `bugfix/<模块>-<问题简述>` | `bugfix/valve-open-timeout` |
| 发布 | `release/v<主>.<次>.<修订>` | `release/v1.2.0` |
| 热修复 | `hotfix/v<版本>-<问题>` | `hotfix/v1.0.1-critical-leak` |

---

## 3. 分支保护规则

### 3.1 多人协作模式

| 分支 | 保护规则 |
|------|----------|
| `main` | 禁止直接 push，必须通过 PR 合并 |
| `develop` | 禁止直接 push，必须通过 PR 合并 |
| 功能分支 | 从 `develop` 检出，完成后 PR 回 `develop` |

### 3.2 单人开发模式

| 分支 | 保护规则 |
|------|----------|
| `main` | 建议通过 merge 方式更新 |
| `develop` | 允许直接 push |

---

## 4. 提交消息格式

### 4.1 格式规范

```
<类型>(<范围>): <简短描述>

[可选正文]

[可选脚注]
```

### 4.2 类型列表

| 类型 | 用途 | 示例 |
|------|------|------|
| `feat` | 新功能 | `feat(metering): add self-learning algorithm` |
| `fix` | 缺陷修复 | `fix(valve): correct open timeout logic` |
| `docs` | 文档更新 | `docs: update coding standard` |
| `refactor` | 重构（不改变功能） | `refactor(billing): simplify tier calculation` |
| `test` | 测试相关 | `test(metering): add ring buffer tests` |
| `style` | 格式调整 | `style: fix indentation` |
| `chore` | 构建/工具/配置 | `chore: update Makefile` |

### 4.3 范围

范围通常是模块名：`metering`, `billing`, `valve`, `power`, `comm`, `hmi` 等。

### 4.4 提交消息示例

```
feat(metering): add ring buffer for ADC samples

Implement a 16-element ring buffer for ISR→main-loop data transfer.
The buffer provides O(1) push/pop operations and handles overflow
by dropping the oldest sample.

Refs: spec §14.1.1
```

---

## 5. 工作流程

### 5.1 多人协作：开发新功能

```bash
# 1. 更新本地 develop
git checkout develop
git pull origin develop

# 2. 创建功能分支
git checkout -b feature/metering-optical-detection

# 3. 开发并提交
# ... 编码 ...
git add <files>
git commit -m "feat(metering): add optical detection algorithm"

# 4. 推送到远程
git push origin feature/metering-optical-detection

# 5. 创建 Pull Request
# 在 GitHub/GitLab 上创建 PR，目标分支：develop

# 6. Code Review
# 等待 Review 通过

# 7. 合并 PR
# Squash merge 或普通 merge

# 8. 清理分支
git checkout develop
git pull origin develop
git branch -d feature/metering-optical-detection
```

### 5.2 多人协作：修复 Bug

```bash
# 1. 从 develop 创建修复分支
git checkout develop
git pull origin develop
git checkout -b bugfix/valve-open-timeout

# 2. 修复并提交
# ... 编码 ...
git commit -m "fix(valve): correct open timeout calculation"

# 3. 推送并创建 PR
git push origin bugfix/valve-open-timeout

# 4. Code Review + 合并
```

### 5.3 发布版本

```bash
# 1. 从 develop 创建发布分支
git checkout develop
git checkout -b release/v1.0.0

# 2. 在发布分支上只做版本更新和 Bug 修复
git commit -m "chore: bump version to 1.0.0"

# 3. 测试通过后，合并到 main
git checkout main
git merge release/v1.0.0
git tag -a v1.0.0 -m "Release v1.0.0"

# 4. 同时合并回 develop
git checkout develop
git merge release/v1.0.0

# 5. 推送
git push origin main --tags
git push origin develop

# 6. 清理
git branch -d release/v1.0.0
```

### 5.4 单人开发：简化流程

```bash
# 日常开发在 develop 分支
git checkout develop

# 编码后直接提交
git add <files>
git commit -m "feat(metering): add new feature"

# 定期推送备份
git push origin develop

# 发布时合并到 main
git checkout main
git merge develop
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin main --tags
```

---

## 6. 不合规操作

| 不合规操作 | 后果 | 正确做法 |
|------------|------|----------|
| 直接 push 到 `main` | 绕过 Review | 通过 PR 合并 |
| 功能分支不从最新 `develop` 创建 | 合并冲突 | 先 `git pull` |
| 一个 PR 包含多个无关功能 | Review 困难 | 一个功能一个 PR |
| PR 超过 500 行 | Review 质量下降 | 拆分小 PR |
| 不填 PR 描述 | Review 缺乏上下文 | 填写完整描述 |

---

## 修订历史

| 版本 | 日期 | 描述 |
|------|------|------|
| v1.0 | 2026-06-17 | 初始版本 |