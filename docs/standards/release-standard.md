# 版本发布规范

**版本：** v1.0
**日期：** 2026-06-17

---

## 1. 版本号规则

采用语义化版本：`v<主版本>.<次版本>.<修订号>`

### 1.1 版本号变化规则

| 变更类型 | 版本号变化 | 示例 |
|----------|------------|------|
| 架构重大变更、不兼容更新 | 主版本 +1 | v1.0.0 → v2.0.0 |
| 新增功能（向后兼容） | 次版本 +1 | v1.0.0 → v1.1.0 |
| Bug 修复（向后兼容） | 修订号 +1 | v1.0.0 → v1.0.1 |

### 1.2 预发布版本

| 类型 | 命名 | 示例 |
|------|------|------|
| 内部测试 | `vX.Y.Z-alpha.N` | v1.0.0-alpha.1 |
| 外部测试 | `vX.Y.Z-beta.N` | v1.0.0-beta.1 |
| 候选版本 | `vX.Y.Z-rc.N` | v1.0.0-rc.1 |

---

## 2. 发布流程

### 2.1 发布准备

```bash
# 1. 从 develop 创建发布分支
git checkout develop
git checkout -b release/v1.0.0

# 2. 在发布分支上：
#    - 更新版本号（代码中 + 文档中）
#    - 更新 CHANGELOG.md
#    - 只做 Bug 修复，不添加新功能

# 3. 运行全量测试
make test-all

# 4. 目标板验证（RL78 + HDSC）
#    - 下载到目标板
#    - 执行功能验证
#    - 执行计量精度验证
```

### 2.2 正式发布

```bash
# 1. 合并到 main
git checkout main
git merge release/v1.0.0

# 2. 创建标签
git tag -a v1.0.0 -m "Release v1.0.0

功能:
- 计量组件完整实现
- 阀门控制完整实现
- 通信功能完整实现

修复:
- 采样异常误触发问题

已知问题:
- 无
"

# 3. 合并回 develop
git checkout develop
git merge release/v1.0.0

# 4. 推送
git push origin main --tags
git push origin develop

# 5. 清理发布分支
git branch -d release/v1.0.0
```

---

## 3. 发布检查清单

```markdown
## Release Checklist

**版本号:** v1.0.0
**发布日期:** 2026-06-17
**发布负责人:** 姓名

---

### 代码质量
- [ ] 所有单元测试通过
- [ ] 所有集成测试通过
- [ ] MISRA-C 强制规则无违规
- [ ] 代码已通过 Code Review

### 功能验证
- [ ] 计量功能验证通过
- [ ] 阀门控制验证通过
- [ ] 通信功能验证通过
- [ ] 安全检测功能验证通过

### 目标板验证
- [ ] RL78 平台编译通过
- [ ] RL78 平台功能验证通过
- [ ] HDSC 平台编译通过
- [ ] HDSC 平台功能验证通过

### 文档
- [ ] 版本号已更新（代码 + 文档）
- [ ] CHANGELOG.md 已更新
- [ ] README.md 已更新（如有必要）

### 发布
- [ ] Git tag 已创建
- [ ] 已推送到远程仓库
```

---

## 4. 热修复流程

当生产环境发现严重 Bug 时：

```bash
# 1. 从 main 创建热修复分支
git checkout main
git checkout -b hotfix/v1.0.1-critical-leak

# 2. 修复 Bug
git commit -m "fix(valve): correct critical leak detection"

# 3. 合并到 main 和 develop
git checkout main
git merge hotfix/v1.0.1-critical-leak
git tag -a v1.0.1 -m "Hotfix v1.0.1"

git checkout develop
git merge hotfix/v1.0.1-critical-leak

# 4. 推送
git push origin main --tags
git push origin develop
```

---

## 修订历史

| 版本 | 日期 | 描述 |
|------|------|------|
| v1.0 | 2026-06-17 | 初始版本 |