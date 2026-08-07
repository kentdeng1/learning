#!/bin/bash
# install-bash.sh
# 用途：在 ~/.bashrc 里加 acp 别名（指向 sync.sh）
# 使用：bash ./install-bash.sh
# 效果：之后任何 bash 窗口都能用 acp 提交
#
# 卸载：手动从 ~/.bashrc 删除 acp 相关行

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SYNC_SCRIPT="$SCRIPT_DIR/sync.sh"

# Windows 路径转换（Git Bash 环境下）
# acp 别名需要用 POSIX 路径，否则 Windows cmd 不识别
BASHRC="$HOME/.bashrc"
MARKER="# >>> gr-jd sync alias >>>"
END_MARKER="# <<< gr-jd sync alias <<<"

# 检查是否已存在
if [ -f "$BASHRC" ] && grep -qF "$MARKER" "$BASHRC"; then
    echo "→ 别名已存在，跳过添加"
    echo "  重新加载: source $BASHRC"
    exit 0
fi

# 追加别名块
cat >> "$BASHRC" <<EOF

$MARKER
alias acp='bash "$SYNC_SCRIPT"'
$END_MARKER
EOF

echo "✓ 已添加 acp 别名到 $BASHRC"
echo ""
echo "让当前 bash 立即生效："
echo "  source $BASHRC"
echo ""
echo "之后可以这样用（在项目目录下）："
echo "  acp                          # 交互式"
echo "  acp 'W1 day3 完成指针章节'    # 指定信息"
echo "  acp -Auto                    # 自动时间戳"
echo "  acp -Pull                    # 只拉不推"
