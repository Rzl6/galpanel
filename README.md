# GALPANEL

GALPANEL 是一款基于 SuperMini/ProMicro nRF52840 的 GALGAME 专用 USB + BLE 双模控制器。

本仓库同时保存硬件资料、项目认知和 ZMK 固件配置。当前软件阶段已经建立首版 ZMK 工程骨架，目标板为：

```text
board:  nice_nano//zmk
shield: galpanel
```

## 从这里开始

如果你有 STM32 + CubeMX + HAL 基础，但没有接触过 ZMK，建议按这个顺序阅读：

1. `项目认知.md`：先理解硬件和最终键位；
2. `项目规划.md`：了解软件会分哪些阶段完成；
3. `学习日志.md`：建立 STM32 与 Zephyr/ZMK 的知识映射；
4. `config/boards/shields/galpanel/galpanel.overlay`：看 GPIO 和 EC11；
5. `config/boards/shields/galpanel/galpanel.keymap`：看按键行为；
6. `galpanel.log.md`：查看每次具体改动和原因。

## 当前软件能力

首版配置已经描述：

- 3 颗机械键；
- 4 颗侧键；
- SYS 按键；
- EC11 旋转和按压；
- 基础层和 Fn 层；
- 鼠标滚轮、音量键、BLE Profile 和 USB/BLE 输出切换；
- QSAVE、QLOAD、Win+D 的双击保护；
- 5 路 LED 的硬件 GPIO 定义。

LED 的状态动画、自检、Fn 超时退出和 SYS 长按清除配对仍属于后续阶段。

## 最省事的编译方式

当前仓库已经配置 GitHub Actions。将仓库推送到 GitHub 后，每次 `push` 都会自动编译：

1. 打开仓库的 `Actions` 页面；
2. 进入最新的 `Build ZMK firmware`；
3. 下载 `firmware` 构建产物；
4. 双击两次 RST，使 SuperMini 出现 UF2 U 盘；
5. 将 `galpanel.uf2` 复制进去；
6. 先测试 USB，再测试 BLE。

首次启用鼠标滚轮后，BLE HID 描述会发生变化，需要在 Windows 中删除旧配对并重新配对一次。

## 本地检查

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\validate-project.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\check-environment.ps1
```

这两个脚本不会修改系统：第一个检查仓库结构和关键配置，第二个只显示当前电脑已经安装了哪些开发工具。

## 开发纪律

每次完成一轮工作后必须：

1. 验证代码或文档；
2. 更新 `galpanel.log.md`；
3. 新增知识时更新 `学习日志.md`；
4. 检查 `git diff` 和 `git status`；
5. 创建内容明确的 Git 提交。

