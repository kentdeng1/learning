#!/bin/bash
# sync.sh
# 用途：bash 版本的 git 同步（pull + add + commit + push）
#
# 用法：
#   ./sync.sh                    # 交互式：问你提交信息
#   ./sync.sh "W1 day3 完成"     # 直接给信息
#   ./sync.sh -Auto              # 自动模式：用时间戳
#   ./sync.sh -Pull              # 只 pull 不 commit
#
# 配合 install-bash.sh，可以用：
#   acp "W1 day3"

set -e

# 颜色（如果终端不支持会自动 fallback）
if [ -t 1 ]; then
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    YELLOW='\033[0;33m'
    CYAN='\033[0;36m'
    GRAY='\033[0;90m'
    NC='\033[0m'
else
    GREEN='' RED='' YELLOW='' CYAN='' GRAY='' NC=''
fi

step() { echo -e "${CYAN}→ $1${NC}"; }
ok()   { echo -e "${GREEN}✓ $1${NC}"; }
err()  { echo -e "${RED}✗ $1${NC}"; }
warn() { echo -e "${YELLOW}⚠ $1${NC}"; }
dim()  { echo -e "${GRAY}  $1${NC}"; }

# 参数解析
MESSAGE=""
AUTO=false
PULL_ONLY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -Auto|-auto) AUTO=true; shift ;;
        -Pull|-pull) PULL_ONLY=true; shift ;;
        -*)
            err "未知参数: $1"
            exit 1
            ;;
        *)
            MESSAGE="$1"
            shift
            ;;
    esac
done

# 前置检查
if [ ! -d ".git" ]; then
    err "当前目录不是 git 仓库: $(pwd)"
    exit 1
fi

REMOTE=$(git remote get-url origin 2>/dev/null || true)
if [ -z "$REMOTE" ]; then
    err "未设置 remote origin"
    warn "先执行: git remote add origin <url>"
    exit 1
fi
dim "仓库: $REMOTE"

# 1. Pull
step "拉取最新..."
git pull --rebase --autostash 2>&1 | while IFS= read -r line; do dim "$line"; done

# Pull only
if [ "$PULL_ONLY" = true ]; then
    ok "Pull 完成"
    exit 0
fi

# 2. 检查变化
if [ -z "$(git status --porcelain)" ]; then
    warn "无变化，跳过提交"
    exit 0
fi

# 3. Add
step "添加文件..."
git add .

# 4. 决定 commit 信息
if [ "$AUTO" = true ]; then
    COMMIT_MSG="auto: sync $(date +'%Y-%m-%d %H:%M')"
elif [ -n "$MESSAGE" ]; then
    COMMIT_MSG="$MESSAGE"
else
    read -p "提交信息: " COMMIT_MSG
fi

if [ -z "$COMMIT_MSG" ]; then
    err "提交信息不能为空"
    exit 1
fi

# 5. Commit
step "提交: $COMMIT_MSG"
git commit -m "$COMMIT_MSG"

# 6. Push
step "推送到 origin..."
PUSH_OUTPUT=$(git push 2>&1)
PUSH_EXIT=$?

if [ $PUSH_EXIT -eq 0 ]; then
    ok "完成: $COMMIT_MSG"
else
    err "Push 失败"
    echo ""
    dim "$PUSH_OUTPUT"
    echo ""

    # 检测常见的失败原因
    if echo "$PUSH_OUTPUT" | grep -q "non-fast-forward"; then
        warn "原因：远端有本地没有的 commit（通常是仓库建时 GitHub 自动生成的 Initial commit）"
        warn ""
        warn "解决方法（二选一）："
        warn "  强制推送（覆盖远端）:  git push -u origin main --force"
        warn "  合并后推送（保留远端）:  git pull origin main --rebase && git push -u origin main"
    elif echo "$PUSH_OUTPUT" | grep -q "Permission denied\|authentication"; then
        warn "原因：认证失败"
        warn "解决方法：配置 SSH key 或用 Personal Access Token（不是密码）"
    elif echo "$PUSH_OUTPUT" | grep -q "Could not resolve host\|failed to connect"; then
        warn "原因：网络问题"
        warn "解决方法：检查网络后重试"
    fi

    exit 1
fi
