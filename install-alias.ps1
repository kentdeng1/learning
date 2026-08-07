# install-alias.ps1
# 用途：在 PowerShell profile 里添加 acp 别名
# 使用：.\install-alias.ps1
# 效果：之后可以在任何 PowerShell 窗口里直接用 acp 代替 .\sync.ps1
#
# 卸载：手动从 $PROFILE 删除 acp 相关行

$ErrorActionPreference = "Stop"

# 脚本绝对路径
$scriptPath = (Resolve-Path "$PSScriptRoot\sync.ps1").Path
$profilePath = $PROFILE.CurrentUserAllHosts
$profileDir = Split-Path $profilePath -Parent

# 创建 profile 目录
if (-not (Test-Path $profileDir)) {
    New-Item -ItemType Directory -Path $profileDir -Force | Out-Null
    Write-Host "✓ 创建 profile 目录: $profileDir" -ForegroundColor Green
}

# 创建 profile 文件
if (-not (Test-Path $profilePath)) {
    New-Item -ItemType File -Path $profilePath -Force | Out-Null
    Write-Host "✓ 创建 profile: $profilePath" -ForegroundColor Green
}

# 检查是否已存在
$marker = "# >>> gr-jd sync alias >>>"
$endMarker = "# <<< gr-jd sync alias <<<"
$existing = Get-Content $profilePath -ErrorAction SilentlyContinue

if ($existing -and ($existing -match [regex]::Escape($marker))) {
    Write-Host "→ 别名已存在，跳过添加" -ForegroundColor DarkGray
    Write-Host "  重新加载: . `$PROFILE" -ForegroundColor Cyan
    exit 0
}

# 添加别名块
$aliasBlock = @"

$marker
function global:acp {
    & "$scriptPath" @args
}
Set-Alias -Name acp -Value acp -Force -Scope Global
$endMarker
"@

Add-Content -Path $profilePath -Value $aliasBlock -Encoding UTF8

Write-Host "✓ 已添加 acp 别名" -ForegroundColor Green
Write-Host ""
Write-Host "让当前 PowerShell 立即生效：" -ForegroundColor Cyan
Write-Host "  . `$PROFILE" -ForegroundColor Yellow
Write-Host ""
Write-Host "之后可以这样用：" -ForegroundColor Cyan
Write-Host "  acp                          # 交互式" -ForegroundColor Yellow
Write-Host "  acp 'W1 day3 完成指针章节'    # 指定信息" -ForegroundColor Yellow
Write-Host "  acp -Auto                    # 自动时间戳" -ForegroundColor Yellow
Write-Host "  acp -Pull                    # 只拉不推" -ForegroundColor Yellow
