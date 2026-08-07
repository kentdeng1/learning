# K 的嵌入式转型学习仓库

> 从 8 位 MCU FAE 到嵌入式软件工程师，6 个月计划。
> 详细计划见 [`plan-v1.md`](./plan-v1.md)。

## 目录结构

| 目录 | 用途 |
|------|------|
| [`notes/`](./notes/) | 学习笔记：C 语言、STM32、FreeRTOS、通信协议等知识点总结 |
| [`code/`](./code/) | 练习代码：知识点验证、demo、算法练习 |
| [`projects/`](./projects/) | 项目实战：简历要写的 2-3 个完整项目 |
| [`resume/`](./resume/) | 简历、面试题库、项目讲解稿 |

## 自动化

- 每 30 分钟自动 `git add` + `commit` + `push` 到 GitHub（任务计划 `GR-JD Auto Sync`）
- 急用时手动 `acp "msg"` 或 `Start-ScheduledTask -TaskName "GR-JD Auto Sync"`
- 详细见 `sync.sh` 和 `install-autosync.ps1`

## 进度跟踪

- 6 个月计划：[`plan-v1.md`](./plan-v1.md)
- 学习日志：`.workbuddy/memory/`（自动维护）