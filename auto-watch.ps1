# auto-watch.ps1
# 用途：监视项目目录，文件变化 30 秒后自动提交
# 用法：.\auto-watch.ps1
# 停止：Ctrl + C
#
# 推荐用法：
#   1. 开一个独立的 PowerShell 窗口跑这个脚本
#   2. 窗口最小化挂着
#   3. 任何时候有文件变化 30 秒后自动 commit + push
#
# 适合场景：
#   - WorkBuddy 改完文件后自动存档
#   - VSCode 写完代码自动存档
#   - 你自己改文件后自动存档
#
# 注意：
#   - 30 秒防抖：连续编辑时只在停止后提交一次
#   - 不要在大量编译/构建时启用（会触发很多次提交）

$ErrorActionPreference = "Stop"

$projectDir = (Get-Location).Path
$syncScript = "$projectDir\sync.ps1"
$debounceSeconds = 30

if (-not (Test-Path "$projectDir\.git")) {
    Write-Host "✗ 当前目录不是 git 仓库" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  自动同步已启动" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  监视目录: $projectDir" -ForegroundColor DarkGray
Write-Host "  防抖时间: $debounceSeconds 秒" -ForegroundColor DarkGray
Write-Host "  停止方式: Ctrl + C" -ForegroundColor DarkGray
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 防抖状态
$script:lastTrigger = (Get-Date).AddSeconds(-$debounceSeconds * 2)

$action = {
    $now = Get-Date
    $elapsed = ($now - $script:lastTrigger).TotalSeconds

    if ($elapsed -lt $debounceSeconds) { return }
    $script:lastTrigger = $now

    Write-Host "[$($now.ToString('HH:mm:ss'))] 检测到文件变化，等待稳定..." -ForegroundColor Yellow
    Start-Sleep -Seconds 5

    # 再次检查（避免合并中状态）
    Set-Location $projectDir
    $status = git status --porcelain 2>$null
    if (-not $status) { return }

    & $script:syncScript -Auto
    Write-Host ""
}

# 创建监视器
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $projectDir
$watcher.IncludeSubdirectories = $true
$watcher.NotifyFilter = [System.IO.NotifyFilters]::FileName, `
                        [System.IO.NotifyFilters]::LastWrite, `
                        [System.IO.NotifyFilters]::DirectoryName
$watcher.EnableRaisingEvents = $true

# 注册事件
$eventIds = @()
$eventIds += Register-ObjectEvent $watcher "Changed" -Action $action
$eventIds += Register-ObjectEvent $watcher "Created" -Action $action
$eventIds += Register-ObjectEvent $watcher "Deleted" -Action $action
$eventIds += Register-ObjectEvent $watcher "Renamed" -Action $action

# 启动时先做一次同步
& $syncScript -Auto
Write-Host ""

# 保持运行
try {
    while ($true) {
        Start-Sleep -Seconds 1
    }
} finally {
    # 清理
    $watcher.EnableRaisingEvents = $false
    $watcher.Dispose()
    foreach ($id in $eventIds) {
        Unregister-Event -SourceIdentifier $id -ErrorAction SilentlyContinue
    }
    Write-Host ""
    Write-Host "自动同步已停止" -ForegroundColor Yellow
}
