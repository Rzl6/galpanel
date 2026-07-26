# GALPANEL 工作日志

本文件记录每轮开发做了什么、为什么这么做、对后续有什么好处、如何验证以及遗留问题。

日志按时间倒序追加。完成一轮代码或硬件相关工作后，必须先更新本文件，再创建 Git 提交。

---

## 2026-07-26 - 同步 GitHub、首次云端构建并刷入 GALPANEL

### 本次完成

- 将本地 `main` 安全推送到空仓库 `https://github.com/Rzl6/galpanel`，并设置为跟踪 `origin/main`；
- 阅读 ProMicro NRF52840 商品资料，确认兼容 nice!nano、双击 RST 进入 `NICENANO` 和 UF2 拖拽烧录路线；
- 监控 GitHub Actions Run #1（ID `30197044810`），构建目标为 `nice_nano//zmk + galpanel`；
- 确认 West、Kconfig、Devicetree、ZMK 编译和产物归档全部成功，总耗时 4 分 21 秒；
- 下载最终 `firmware` 产物，并用 GitHub 页面公布的 SHA-256 校验 ZIP；
- 解压并验证 `galpanel.uf2` 的文件名、大小、SHA-256 和 UF2 魔数；
- 刷写前再次确认 E 盘卷标为 `NICENANO`、类型为可移动磁盘、Board-ID 为 `nRF52840-nicenano`；
- 备份原 `CURRENT.UF2` 后，将 `galpanel.uf2` 复制到开发板；
- 确认复制后 `NICENANO` 自动消失，Windows 枚举名为 `GALPANEL` 的复合 HID；
- 确认设备同时提供 Keyboard、Consumer Control 和 Mouse HID 接口；
- 将 ZMK 上游固定到已验证提交 `f84ef436cd8e88f9d1580a9b86bab2fae8d349c6`；
- 将商品页官方引脚图 `PCB/ProMicroNRF52840Foot.jpg` 纳入项目资料。

### 为什么这么做

首次刷写最重要的是建立可追溯链路：仓库提交、云端构建、产物摘要、Bootloader Board-ID 和实物设备必须一一对应。仅凭文件名相似就刷写，无法排除下载错误或板型错误。

第一次成功构建后固定 ZMK revision，是为了让相同配置以后仍使用同一套上游源码，避免 `main` 更新导致行为变化或突然无法编译。

### 对后续的好处

- 已建立从 `git push` 到 GitHub Actions、UF2、实物启动和 Windows HID 的完整闭环；
- 后续修改只需比较新提交和新固件，不再怀疑基础工具链是否可用；
- 原始 `CURRENT.UF2` 已备份，保留额外恢复与分析材料；
- Keyboard、Consumer Control 和 Mouse 三个 USB 接口均存在，后续可分别测试按键、媒体键和滚轮；
- 固定上游提交后，后续构建具有更好的可重复性。

### 本次验证

- GitHub Actions：<https://github.com/Rzl6/galpanel/actions/runs/30197044810>，结论 `Success`；
- 源提交：`3bb605e515a98756e3b3c968b7e4dd3dfb4592be`；
- `firmware-run-1.zip` SHA-256：`ee6b3bb7e22da461ac8674a448fc7acc3cd77c35aee1731773e1ed0be466aead`；
- `galpanel.uf2` 大小：412160 bytes；
- `galpanel.uf2` SHA-256：`6805b40de4d7214abe371d7dc0138b99bb5ab18e32165ed1b43dc92ed017f5e4`；
- 原 `CURRENT.UF2` 备份 SHA-256：`9d2ac4f29706e9bee83a9ea772e2eb658f4025eb5e2d3823e1242cbaceb44fdb`；
- USB Bus Reported Device Description：`GALPANEL`；
- USB VID/PID：`1D50:615E`；
- 复制后 `NICENANO` 自动消失，Keyboard/Consumer Control/Mouse 均为正常状态。

### 尚未完成

- 需要用户实按 D2、D3、D4、D19，确认 Ctrl、A、Enter、Esc；
- 需要实测 EC11 方向、每格触发数和快速旋转；
- 需要快速双击 RST，确认能稳定重新进入 `NICENANO`；
- 四颗侧键、Fn 层、SYS、BLE 和 LED 尚未进行实物功能测试；
- 固定 ZMK revision 后需要由下一次 Actions 构建再次确认可重复编译。

### 计划提交

```text
docs: record first GALPANEL firmware flash
```

---

## 2026-07-26 - 识别实物 Bootloader 并扩展学习教程

### 本次完成

- 在开发板通电连接电脑后，读取 Windows 可移动磁盘列表；
- 识别到开发板挂载为 `E:\`，卷标为 `NICENANO`，FAT 文件系统，容量约 32 MB；
- 只读检查 `CURRENT.UF2`、`INDEX.HTM` 和 `INFO_UF2.TXT`；
- 确认实物为 nice!nano 兼容 UF2 Bootloader：
  - UF2 Bootloader 0.6.0；
  - Board-ID `nRF52840-nicenano`；
  - SoftDevice S140 6.1.1；
  - Bootloader 日期 2021-06-19；
- 未向开发板写入任何文件，保留当前可恢复状态；
- 将 `学习日志.md` 扩展为从 Windows 环境、Git、uv/ZMK CLI、首次连接开发板、GitHub 云端构建到首次 UF2 刷写的连续教程和实操手册；
- 在 `项目认知.md` 中加入实物 Bootloader 识别结论。

### 为什么这么做

此前 `nice_nano//zmk` 是依据商品资料和 Bootloader 文件做出的合理选择。实物 `INFO_UF2.TXT` 现在提供了直接证据，可以在刷写前确认 Board target 方向正确。

用户需要的是能跟着执行的教程，而不只是概念对照。因此学习日志必须从环境检查和真实设备现象开始，每一步同时说明命令、预期输出、风险和测试记录方法。

### 对后续的好处

- 首次编译可以继续使用 `nice_nano//zmk`，不需要先创建自定义主控板定义；
- 已确认 USB 数据和 Bootloader 基本正常，首次刷写的变量更少；
- 即使后续应用固件出错，仍有明确的 UF2 和 SWD 恢复路线；
- 用户可以按章节实操，不需要一次理解完整 Zephyr 工具链；
- 后续每次出现新现象，都能直接补充到对应教程章节和测试表格。

### 本次验证

- `Get-Volume` 显示 `E: NICENANO FAT Removable Healthy`；
- `INFO_UF2.TXT` 可正常读取；
- Bootloader Model 和 Board-ID 均指向 nice!nano/nRF52840；
- 项目文档中的 Board target 与实物信息一致。

### 尚未完成

- 需要用户创建或提供 GitHub 空仓库地址；
- 尚未触发第一次 GitHub Actions 构建；
- 尚未生成或刷入 `galpanel.uf2`；
- 需要继续测试拔插行为和双击 RST 重新进入 UF2；
- 当前直接进入 U 盘可能是因为没有有效应用固件，刷入 ZMK 后需确认能自动启动应用。

### 计划提交

```text
docs: expand setup tutorial and record UF2 bootloader
```

---

## 2026-07-26 - 初始化软件工程与 Git 结构

### 本次完成

- 阅读 SuperMini nRF52840 官方资料目录；
- 确认官方资料包含 nRF52840 产品规格、nice!nano 0.6.0 Bootloader、J-Link、nRF Connect Programmer 和 nRF 命令行工具；
- 确认软件主路线使用 ZMK/Zephyr，UF2 日常烧录，J-Link/SWD 作为恢复方式；
- 创建标准 ZMK 用户配置结构；
- 创建 `galpanel` 自定义 shield；
- 写入 V2.1 的 9 路普通输入、EC11 A/B 和 5 路 LED GPIO；
- 写入基础层和 Fn 层；
- 为 QSAVE、QLOAD、Win+D 添加双击保护；
- 为 SYS 添加单击切换 BLE Profile、双击切换 USB/BLE 输出；
- 为基础层 EC11 添加鼠标滚轮，为 Fn 层添加音量控制；
- 创建 GitHub Actions 自动编译入口；
- 创建仓库结构检查和环境检查脚本；
- 创建 `README.md`、`项目规划.md` 和 `学习日志.md`；
- 添加 `.gitignore`，避免把大体积官方安装程序和构建产物提交到 Git；
- 初始化 Git 主分支并准备首个提交。

### 为什么这么做

用户已有 STM32、CubeMX 和 HAL 基础，但没有接触过 Zephyr、ZMK、West 和 GitHub Actions。如果直接安装完整本地工具链并同时实现全部功能，遇到问题时很难判断是环境、设备树、GPIO、蓝牙还是键位行为导致的。

因此先建立一套云端可构建、目录职责清晰、可以逐阶段验证的工程。GitHub Actions 负责复杂编译环境，用户初期只需要理解配置、下载 UF2 和测试硬件。

### 对后续的好处

- 所有 GPIO 与 V2.1 原理图有固定对应关系；
- 后续修改键位不需要重新写底层 USB 或 BLE 协议；
- 每个阶段都能单独验证，问题更容易定位；
- Git 可以随时比较、回退和恢复到已知可用版本；
- 大型厂商安装程序不会进入 GitHub，避免仓库过大和超过单文件限制；
- 文档与代码放在同一仓库，后续不会出现“聊天里定了，但代码里忘了”的情况；
- LED 和危险操作被刻意延后，可先完成最关键的输入与连接闭环。

### 本次验证

- 对照 V2.1 原理图检查了 D0-D21 的最终映射；
- 对照当前 ZMK 官方源码确认 `nice_nano//zmk` 板型、GPIO direct kscan、EC11、tap-dance、鼠标滚轮和可复用 GitHub Actions 的语法方向；
- 使用静态检查脚本验证项目必需文件和关键配置存在；
- 使用 PyYAML 验证 `build.yaml`、workflow、west manifest 和 shield metadata 可正常解析；
- 通过 `uvx zmk --version` 确认可按需使用 ZMK CLI 0.4.1；
- 当前电脑已安装 Git 和 uv，尚未安装 West/CMake/nrfjprog/J-Link 命令行环境，因此首轮采用 GitHub Actions 云端构建；
- 检查 Git 忽略规则，确认大型 `.exe` 和 `.tar.gz` 厂商工具不会进入版本库。

### 尚未完成

- 尚未在 GitHub Actions 中进行真正的 ZMK 编译；
- 尚未刷入 SuperMini 实物；
- LED 只有 GPIO 定义，没有状态事件监听与 PWM 动画；
- SYS 的长按状态查询和长按清除配对尚未实现；
- Fn 自动退出尚未实现；
- ZMK 目前跟随 `main`，首次成功构建后需要固定 commit；
- ADC 按用户决定暂不实现；
- 天线性能等待实物测试。

### 计划提交

```text
chore: initialize GALPANEL ZMK project
```

---

## 日志模板

复制下面内容到文件顶部，并替换字段：

```markdown
## YYYY-MM-DD - 简短标题

### 本次完成

- 

### 为什么这么做



### 对后续的好处



### 本次验证

- 

### 尚未完成

- 

### 关联提交

`提交信息或 commit hash`
```
