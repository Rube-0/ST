# GitHub 基本操作指南

## 一、Git 初始配置

```bash
# 设置用户名
git config --global user.name "你的用户名"

# 设置邮箱
git config --global user.email "你的邮箱@example.com"

# 查看配置
git config --list
```

## 二、仓库操作

### 1. 创建本地仓库

```bash
# 初始化新仓库
git init

# 克隆远程仓库（SSH 方式，推荐）
git clone git@github.com:用户名/仓库名.git

# 克隆远程仓库（HTTPS 方式）
git clone https://github.com/用户名/仓库名.git
```

### 2. 连接远程仓库

```bash
# 添加远程仓库（SSH 方式，推荐）
git remote add origin git@github.com:用户名/仓库名.git

# 添加远程仓库（HTTPS 方式）
git remote add origin https://github.com/用户名/仓库名.git

# 查看远程仓库
git remote -v

# 将 HTTPS 改为 SSH
git remote set-url origin git@github.com:用户名/仓库名.git

# 删除远程仓库连接
git remote remove origin
```

## 三、基本工作流程

### 1. 查看状态

```bash
# 查看当前状态
git status

# 查看简洁状态
git status -s
```

### 2. 添加文件到暂存区

```bash
# 添加单个文件
git add 文件名

# 添加所有文件
git add .

# 添加所有修改和删除的文件
git add -A
```

### 3. 提交更改

```bash
# 提交并添加描述
git commit -m "提交说明"

# 修改上次提交信息
git commit --amend -m "新的提交说明"
```

### 4. 推送到远程

```bash
# 首次推送（设置上游分支）
git push -u origin main

# 后续推送
git push

# 强制推送（谨慎使用）
git push -f
```

### 5. 拉取更新

```bash
# 拉取并合并
git pull

# 仅获取不合并
git fetch
```

## 四、分支操作

### 1. 分支管理

```bash
# 查看本地分支
git branch

# 查看所有分支（包括远程）
git branch -a

# 创建新分支
git branch 分支名

# 切换分支
git checkout 分支名

# 创建并切换分支
git checkout -b 分支名

# 删除本地分支
git branch -d 分支名

# 强制删除分支
git branch -D 分支名

# 删除远程分支
git push origin --delete 分支名
```

### 2. 合并分支

```bash
# 合并指定分支到当前分支
git merge 分支名

# 变基合并
git rebase 分支名
```

## 五、版本回退

```bash
# 查看提交历史
git log

# 简洁查看历史
git log --oneline

# 回退到上一版本
git reset --hard HEAD^

# 回退到指定版本
git reset --hard commit_id

# 撤销工作区修改
git checkout -- 文件名

# 撤销暂存区文件
git reset HEAD 文件名
```

## 六、暂存工作

```bash
# 暂存当前工作
git stash

# 暂存并添加描述
git stash save "描述信息"

# 查看暂存列表
git stash list

# 恢复最近暂存
git stash pop

# 恢复指定暂存
git stash apply stash@{n}

# 删除暂存
git stash drop stash@{n}
```

## 七、标签管理

```bash
# 创建标签
git tag v1.0.0

# 创建带注释的标签
git tag -a v1.0.0 -m "版本说明"

# 查看所有标签
git tag

# 推送标签到远程
git push origin v1.0.0

# 推送所有标签
git push origin --tags

# 删除本地标签
git tag -d v1.0.0

# 删除远程标签
git push origin --delete v1.0.0
```

## 八、.gitignore 文件

创建 `.gitignore` 文件来忽略不需要跟踪的文件：

```
# 忽略所有 .log 文件
*.log

# 忽略 node_modules 目录
node_modules/

# 忽略编译输出
build/
dist/

# 忽略环境配置文件
.env

# 忽略 IDE 配置
.idea/
.vscode/
```

## 九、常见问题解决

### 1. 解决合并冲突

```bash
# 查看冲突文件
git status

# 手动编辑冲突文件，解决冲突后
git add .
git commit -m "解决冲突"
```

### 2. 取消合并

```bash
git merge --abort
```

### 3. 撤销已推送的提交

```bash
git revert commit_id
git push
```

### 4. git pull 失败：本地没有提交历史

**错误场景**：本地 `git init` 后直接执行 `git pull`，报错失败。

**原因**：本地仓库没有任何提交记录，无法与远程仓库合并。

**解决方法**：

```bash
# 1. 先添加并提交本地文件
git add .
git commit -m "初始提交"

# 2. 拉取远程内容（允许合并不相关的历史）
git pull origin main --rebase --allow-unrelated-histories

# 3. 推送到远程
git push origin main
```

### 5. 拒绝合并不相关的历史

**错误信息**：`fatal: refusing to merge unrelated histories`

**原因**：本地和远程仓库是独立创建的，没有共同的提交历史。

**解决方法**：

```bash
# 添加 --allow-unrelated-histories 参数
git pull origin main --allow-unrelated-histories

# 或使用 rebase 方式
git pull origin main --rebase --allow-unrelated-histories
```

### 6. push 被拒绝：远程有更新

**错误信息**：`rejected - non-fast-forward`

**解决方法**：

```bash
# 先拉取远程更新
git pull origin main

# 解决可能的冲突后再推送
git push origin main
```

### 7. SSH 连接失败

**错误信息**：`Permission denied (publickey)`

**解决方法**：

```bash
# 1. 确保 ssh-agent 运行
eval $(ssh-agent -s)

# 2. 添加私钥
ssh-add ~/.ssh/id_rsa

# 3. 测试连接
ssh -T git@github.com

# 4. 如仍失败，检查公钥是否已添加到 GitHub
cat ~/.ssh/id_rsa.pub
# 复制内容到 GitHub: Settings → SSH and GPG keys → New SSH key
```

## 十、SSH 密钥配置

```bash
# 生成 SSH 密钥
ssh-keygen -t rsa -C "你的邮箱@example.com"

# 查看公钥
cat ~/.ssh/id_rsa.pub

# 启动 ssh-agent
eval $(ssh-agent -s)

# 添加私钥到 ssh-agent
ssh-add ~/.ssh/id_rsa

# 查看已添加的密钥
ssh-add -l

# 删除所有已添加的密钥
ssh-add -D

# 测试连接
ssh -T git@github.com
```

将公钥添加到 GitHub：Settings → SSH and GPG keys → New SSH key

---

> 💡 **提示**：建议多练习这些命令，熟能生巧！
