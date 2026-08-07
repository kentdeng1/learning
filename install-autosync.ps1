# install-autosync.ps1
# 用途：注册 Windows 任务计划，让 GR-JD 项目自动同步到 GitHub
# 用法：powershell -ExecutionPolicy Bypass -File .\install-autosync.ps1
#
# 效果：
#   - 你登录电脑时立即跑一次 sync.sh
#   - 之后每 30 分钟跑一次
#   - 没变化自动跳过（sync.sh 智能判断）
#   - K 完全不用手动操作
#
# 卸载：
#   Unregister-ScheduledTask -TaskName "GR-JD Auto Sync" -Confirm:$false

$ErrorActionPreference = "Stop"

$taskName   = "GR-JD Auto Sync"
$projectDir = "C:\Users\tenx\WorkBuddy\GR-JD"
$syncScript = "$projectDir\sync.sh"
$logPath    = "$projectDir\.workbuddy\autosync.log"

# 找 Git Bash
$bashPaths = @(
    "C:\Program Files\Git\bin\bash.exe",
    "C:\Program Files (x86)\Git\bin\bash.exe",
    "$env:LOCALAPPDATA\Programs\Git\bin\bash.exe"
)

$bashPath = $null
foreach ($p in $bashPaths) {
    if (Test-Path $p) {
        $bashPath = $p
        break
    }
}

if (-not $bashPath) {
    Write-Host "✗ 找不到 Git Bash" -ForegroundColor Red
    Write-Host "  尝试过:" -ForegroundColor Yellow
    foreach ($p in $bashPaths) { Write-Host "    $p" -ForegroundColor DarkGray }
    exit 1
}

Write-Host "→ Git Bash: $bashPath" -ForegroundColor DarkGray
Write-Host "→ 项目目录: $projectDir" -ForegroundColor DarkGray

# 删旧任务
$existing = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($existing) {
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
    Write-Host "→ 删除旧任务" -ForegroundColor DarkGray
}

# 触发器 1：登录时跑
$triggerLogon = New-ScheduledTaskTrigger -AtLogOn

# 触发器 2：每 30 分钟跑一次，持续 1 年
$triggerPeriodic = New-ScheduledTaskTrigger `
    -Once `
    -At (Get-Date).AddMinutes(1) `
    -RepetitionInterval (New-TimeSpan -Minutes 30) `
    -RepetitionDuration (New-TimeSpan -Days 365)

# 操作：用 bash 跑 sync.sh（带日志重定向）
# 注意：用 ; 不用 && —— PowerShell 5.1 解析字符串里的 && 会出错，但 bash 也支持 ;
$bashCmd = "cd '$projectDir'; bash sync.sh -Auto >> '$logPath' 2>&1"
$argument = "-c `"$bashCmd`""
$action = New-ScheduledTaskAction `
    -Execute $bashPath `
    -Argument $argument

# 注册（最高权限，避免 UAC 弹窗）
Register-ScheduledTask `
    -TaskName $taskName `
    -Action $action `
    -Trigger $triggerLogon, $triggerPeriodic `
    -RunLevel Highest `
    -Description "每 30 分钟自动同步 GR-JD 到 GitHub"

Write-Host ""
Write-Host "✓ 已注册任务: $taskName" -ForegroundColor Green
Write-Host ""
Write-Host "效果：" -ForegroundColor Cyan
Write-Host "  - 你登录电脑时立即跑一次" -ForegroundColor DarkGray
Write-Host "  - 之后每 30 分钟跑一次" -ForegroundColor DarkGray
Write-Host "  - 无文件变化自动跳过" -ForegroundColor DarkGray
Write-Host "  - 日志写到: $logPath" -ForegroundColor DarkGray
Write-Host ""
Write-Host "管理命令：" -ForegroundColor Cyan
Write-Host "  立即跑一次:   Start-ScheduledTask -TaskName '$taskName'" -ForegroundColor Yellow
Write-Host "  查看状态:     Get-ScheduledTask -TaskName '$taskName'" -ForegroundColor Yellow
Write-Host "  看日志:       Get-Content '$logPath' -Tail 20" -ForegroundColor Yellow
Write-Host "  卸载:         Unregister-ScheduledTask -TaskName '$taskName' -Confirm:`$false" -ForegroundColor Yellow