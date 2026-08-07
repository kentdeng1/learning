# sync.ps1
# 用途：简化的 git 同步（pull + add + commit + push）
#
# 用法（在项目目录下）：
#   .\sync.ps1                    # 交互式：问你提交信息
#   .\sync.ps1 "W1 day3 完成"     # 直接给信息
#   .\sync.ps1 -Auto              # 自动模式：用时间戳
#   .\sync.ps1 -Pull              # 只 pull 不 commit
#
# 配合 install-alias.ps1，可以用：
#   acp "W1 day3"

param(
    [Parameter(Position=0)]
    [string]$Message = "",

    [switch]$Auto,

    [switch]$PullOnly
)

$ErrorActionPreference = "Stop"

function Step($m) { Write-Host "→ $m" -ForegroundColor Cyan }
function OK($m)   { Write-Host "✓ $m" -ForegroundColor Green }
function Err($m)  { Write-Host "✗ $m" -ForegroundColor Red }
function Warn($m) { Write-Host "⚠ $m" -ForegroundColor Yellow }
function Dim($m)  { Write-Host "  $m" -ForegroundColor DarkGray }

# 前置检查
$projectDir = (Get-Location).Path
if (-not (Test-Path "$projectDir\.git")) {
    Err "当前目录不是 git 仓库: $projectDir"
    exit 1
}

$remote = git remote get-url origin 2>$null
if (-not $remote) {
    Err "未设置 remote origin"
    Warn "先执行: git remote add origin <url>"
    exit 1
}
Dim "仓库: $remote"

# 1. Pull
Step "拉取最新..."
git pull --rebase --autostash 2>&1 | ForEach-Object { Dim $_ }

if ($PullOnly) {
    OK "Pull 完成"
    exit 0
}

# 2. 检查变化
$status = git status --porcelain
if (-not $status) {
    Warn "无变化，跳过提交"
    exit 0
}

# 3. Add
Step "添加文件..."
git add .

# 4. 决定 commit 信息
if ($Auto) {
    $commitMsg = "auto: sync $(Get-Date -Format 'yyyy-MM-dd HH:mm')"
} elseif ($Message) {
    $commitMsg = $Message
} else {
    $commitMsg = Read-Host "提交信息"
}

if ([string]::IsNullOrWhiteSpace($commitMsg)) {
    Err "提交信息不能为空"
    exit 1
}

# 5. Commit
Step "提交: $commitMsg"
git commit -m $commitMsg

# 6. Push
Step "推送到 origin..."
$pushResult = git push 2>&1
if ($LASTEXITCODE -eq 0) {
    OK "完成: $commitMsg"
} else {
    Err "Push 失败"
    Warn "错误信息: $pushResult"
    Warn "如果提示需要先 pull: 先跑 .\sync.ps1 -Pull"
    exit 1
}
