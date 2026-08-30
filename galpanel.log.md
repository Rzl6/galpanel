# GALPANEL 工作日志

本文件记录每轮开发做了什么、为什么这么做、对后续有什么好处、如何验证以及遗留问题。

日志按时间倒序追加。完成一轮代码或硬件相关工作后，必须先更新本文件，再创建 Git 提交。

---

## 2026-08-30 - 编写 LED_TEST 全亮诊断固件

### 本次完成

- 在 `build.yaml` 中新增独立构建目标 `galpanel_led_test`，产物名为 `galpanel-led-test`；
- 新增 LED_TEST shield、keymap、conf 和 metadata，不覆盖正式 `galpanel` 配置；
- 使用 Zephyr GPIO hog 在 GPIO 控制器初始化时直接把五路 LED 输出拉高；
- 禁用 LED_TEST 中无关的 EC11 sensor 和 ZMK sleep，减少测试变量并保持灯持续可观察；
- 五路测试引脚为 INFO D9/P1.06、LINK D6/P1.00、FN D15/P1.13、WARN D1/P0.06、AUX D0/P0.08；
- 扩展项目检查脚本，要求 LED_TEST 文件、构建目标和 `output-high` 配置存在；
- 确认开发板当前挂载为 `E:\ NICENANO`，Board-ID 为 `nRF52840-nicenano`，但在云端构建和 UF2 校验完成前未写入。

### 为什么这么做

当前首先要回答的是 LED 电气链路是否成立，而不是状态动画是否正确。GPIO hog 不依赖 BLE、键位行为或自定义事件监听，可以在 Zephyr 初始化 GPIO 时直接输出高电平，从而把软件变量压缩到最少。

测试固件使用独立 shield 和 artifact 名称，避免把诊断配置误当成正式 GALPANEL 固件，也便于随时刷回正式版本。

### 对后续的好处

- 五灯全部亮可快速确认 `R_LED_EN`、LED_GND、极性和主要焊接回路；
- 部分不亮时可直接按 D9/D6/D15/D1/D0 定位；
- 全亮测试通过后，后续 LED 状态模块只需关注事件和动画，不再怀疑电气层；
- 正式 USB/BLE 固件没有被诊断行为污染。

### 本次验证

- PowerShell 7 项目结构检查：通过；
- `build.yaml`、workflow、west manifest 和两份 shield metadata 的 YAML 解析：通过；
- `E:\INFO_UF2.TXT`：nice!nano / nRF52840-nicenano；
- 尚待 GitHub Actions 进行真实 Devicetree/Kconfig/ZMK 编译。

### 尚未完成

- LED_TEST 云端构建；
- 下载和校验 `galpanel-led-test.uf2`；
- 实物烧录与五灯观察；
- 根据结果逐灯或公共回路诊断；
- 测试结束后刷回正式 `galpanel.uf2`。

### 计划提交

```text
test: add all-on LED diagnostic firmware
```

---

## 2026-08-30 - 建立整板焊接后的分阶段测试计划

### 本次完成

- 根据“电池以外已经全部焊接完成”的状态，新建《测试计划.md》；
- 将整板验收拆为 T0～T8：断电预检、LED 电气、九路输入、EC11、USB、BLE、Profile/输出、USB 供电安全、RF/机械；
- 设计独立 LED_TEST、IO_TEST 和正式 GALPANEL 三类固件的切换策略；
- 固定最新原理图 LED 颜色：INFO 蓝、LINK 绿、FN 白、WARN 红、AUX 蓝；
- 明确电池未焊接，充电、开关、电池电流和续航暂不验收；
- 更新项目规划，把当前阶段从裸板 BLE 推进到整板 IO 与 LED bring-up。

### 为什么这么做

整板焊接后，故障来源会同时增加到 LED 公共回路、按键焊点、EC11 RC、USB、BLE 和机械应力。如果直接刷正式固件并一次性测试所有宏，失败时无法判断是硬件、电气还是行为层问题。独立测试固件能把每一层的责任边界固定下来。

### 对后续的好处

- LED 全灭可以先区分 `R_LED_EN/LED_GND` 公共回路和软件驱动问题；
- IO_TEST 将 D3/D4 和侧键变成可见数字，避免 F11、Win+D、Ctrl 等不可见或有副作用的键值；
- 正式固件只在测试固件通过后恢复，减少误触宏和误清配对风险；
- 电池相关测试被明确隔离，避免在没有电池保护和测量条件时误上电；
- 每个阶段都有可重复的通过标准和日志字段。

### 本次验证

- 对照当前 `galpanel.overlay` 核对九路输入、EC11 和五路 LED GPIO；
- 对照用户提供的最新焊接原理图核对 LED 颜色与 `LED_GND → R_LED_EN(0 Ω) → GND` 回路；
- 项目结构检查和文档差异检查待提交前执行。

### 尚未完成

- LED_TEST 固件代码与构建产物；
- IO_TEST 固件代码与构建产物；
- T0 断电电气预检实测；
- T1～T8 整板实测记录。

### 计划提交

```text
docs: add full-board test plan [skip ci]
```

---

## 2026-08-30 - 校正 GALPANEL LED 颜色并区分板载指示灯

### 本次完成

- 根据用户提供的最新焊接原理图，确认颜色映射：`INFO=蓝`、`LINK=绿`、`FN=白`、`WARN=红`、`AUX=蓝`；
- 修正 `项目认知.md` 中此前把 INFO/LINK 颜色写反的问题；
- 为设备树 LED 标签补充颜色信息；
- 明确 SuperMini 板载红/蓝灯与 GALPANEL 五颗 LED 是两套独立电路；
- 明确当前固件仅声明 `gpio-leds`，尚未实现状态事件监听或 LED API 驱动，因此 GALPANEL LED 全部不亮属于当前软件预期；
- 记录 `LED_GND -> R_LED_EN(0 Ω) -> GND` 公共回路为硬件排查重点。

### 为什么这么做

如果把板载蓝灯闪烁误认为 GALPANEL 的 LINK 灯，或者把 GALPANEL 灯不亮误认为 BLE 失败，排查方向会完全偏离。先固定网络、颜色和软件现状，才能设计可靠的 LED bring-up 测试。

### 对后续的好处

- 装配、测试和代码中的颜色命名保持一致；
- BLE 故障与 LED 公共回路故障可以分开定位；
- 后续可先做“全灯点亮”诊断，再接入 BLE/Profile/Fn/WARN 状态动画；
- `R_LED_EN` 缺件或公共地断路时，可以优先检查一个节点而不是五颗 LED 逐个返工。

### 本次验证

- 对照用户提供的 V2.1 焊接原理图核对 D9/D6/D15/D1/D0 五路网络；
- 对照 `galpanel.overlay` 确认五路 GPIO 和 `GPIO_ACTIVE_HIGH` 一致；
- 对照当前固件注释确认尚未实现 LED 状态驱动；
- ZMK 项目结构检查继续通过。

### 尚未完成

- 全灯点亮诊断固件；
- `R_LED_EN` 和 `LED_GND` 实物通断确认；
- BLE 广播、连接状态与 LINK 绿灯绑定；
- USB/操作反馈与 INFO 蓝灯绑定；
- Fn 白灯、WARN 红灯、AUX 蓝灯的实际行为。

### 计划提交

```text
docs: correct GALPANEL LED color mapping [skip ci]
```

---

## 2026-07-26 - Windows BLE 配对与无线 Ctrl 验证通过

### 本次完成

- Windows 成功搜索到并配对 `GALPANEL`；
- 用户确认 D2/P0.17 对应的左 Ctrl 在 BLE 模式下可以正常使用；
- 将 M3 的 B0 广播/配对和 B1 BLE 输入标记为通过；
- 更新教程，明确当前烧录的是同一份 USB + BLE 双模 ZMK 固件，而不是两份独立固件。

### 为什么这么做

“显示已配对”只能证明控制链路建立，实际 Ctrl 成功才证明 BLE HID 报告从 GPIO 扫描、ZMK 行为层、蓝牙协议栈一直到 Windows 输入系统的完整数据通路正常。

### 对后续的好处

- nRF52840、Bootloader、ZMK BLE 和 Windows 主机兼容性得到实物证明；
- USB 与 BLE 已能复用同一套 keymap；
- 后续蓝牙问题可以集中在重连、Profile、输出路由和 RF 环境，而不再怀疑基本 BLE HID 能力。

### 本次验证

- Windows 发现设备名：`GALPANEL`；
- 首次配对：通过；
- BLE 左 Ctrl：通过；
- D2/P0.17：USB 与 BLE 两条输出路径均通过。

### 尚未完成

- 断电再上电后的自动重连；
- D8/SYS 单击切换 BLE Profile；
- D8/SYS 双击切换 USB/BLE 输出；
- Windows 与手机两个主机之间的 Profile 切换；
- 载板焊接后的天线和装壳 RF 测试。

### 计划提交

```text
docs: record successful BLE input test [skip ci]
```

---

## 2026-07-26 - 确认 D2 Ctrl 成功并规划裸板 BLE 阶段

### 本次完成

- 用户在按住左机械键对应的 D2→GND 后，按电脑键盘 `C` 成功复制文字；
- 确认 D2/P0.17 电气输入、内部上拉、ZMK direct kscan、`LCTRL` modifier 报告和 Windows HID 合并输入全部正常；
- 将《学习日志》中的左机械键测试标记为通过；
- 核对当前固件已经具备 BLE HID、多 Profile、SYS 单击 `BT_NXT` 和双击 `OUT_TOG`；
- 根据“载板尚未焊接”的现实条件，把下一阶段调整为先做裸开发板 BLE 闭环；
- 写入 Windows 首次配对、纯 BLE 输入、断电重连和 D8/SYS Profile/输出切换的分步实操方案；
- 更新项目认知中已经过时的“尚未编译和刷写”状态。

### 为什么这么做

当前无法方便地验证所有载板 GPIO，但 BLE 的广播、配对、绑定和重连只依赖主控模块。先验证 BLE 能继续推进核心软件路线，也能在装壳和焊板前尽早发现模块、天线或主机兼容性问题。

第一轮不修改代码，是为了先验证 ZMK 默认 BLE 基线。如果直接同时修改连接参数、发射功率、Profile 行为和灯效，出现问题时难以判断原因。

### 对后续的好处

- USB 输入链路已从“枚举正常”提升为“真实 Ctrl 功能通过”；
- 可以在载板未完成时提前完成双模设备最关键的无线闭环；
- 用充电器/移动电源供电可以物理排除 USB 数据路径，证明输入确实来自 BLE；
- 将广播、绑定、输出路由和 GPIO 分层测试，后续故障更容易定位；
- 暂不开放清除配对行为，避免在没有 LED 反馈时误删 bond。

### 本次验证

- D2→GND + 电脑 `C`：成功复制文字；
- 当前 keymap：D2 基础层 `LCTRL`；
- 当前 SYS：D8/P1.04，单击下一个 BLE Profile，双击切换 USB/BLE 输出；
- Git 修改前工作区与 `origin/main` 同步。

### 尚未完成

- Windows 搜索和配对 `GALPANEL`；
- 使用非电脑 USB 电源验证纯 BLE Ctrl；
- 断电上电后的自动重连；
- 使用绝缘跳线验证 D8/SYS；
- 手机与 Windows 两个 Profile 的切换；
- 载板焊接后的全部 GPIO、EC11、LED 和 RF 装壳测试。

### 计划提交

```text
docs: plan bare-board Bluetooth validation [skip ci]
```

---

## 2026-07-26 - 复刷并诊断左机械键无可见输出

### 本次完成

- 收到“按住左侧机械键没有反应”的实物反馈；
- 核对设备树：左机械键为扫描序号 0，连接 `D2/P0.17`，使用 `GPIO_ACTIVE_LOW | GPIO_PULL_UP`；
- 核对 keymap：基础层绑定 `LCTRL`，Fn 层绑定 `S`，扫描顺序与 keymap 顺序一致；
- 确认用户已重新进入 `NICENANO` Bootloader，Board-ID 仍为 `nRF52840-nicenano`；
- 重新刷入 Run #3 的 `galpanel.uf2`，SHA-256 为 `6805b40de4d7214abe371d7dc0138b99bb5ab18e32165ed1b43dc92ed017f5e4`；
- 确认复制后 `NICENANO` 自动消失，`GALPANEL` 的 Keyboard、Consumer Control 和 Mouse HID 再次正常枚举；
- 运行 20 秒 Windows 左 Ctrl 状态探测，本次未捕获到按下事件；由于不能确认实物是否在探测窗口内按下，该结果暂不作为硬件故障结论。

### 为什么这么做

Ctrl 是修饰键，单独按下不会像字母键一样在记事本显示字符。先核对扫描引脚、逻辑顺序和 HID 绑定，再复刷已知正确的 UF2，可以避免把正常的 Ctrl 行为误判成烧录失败。

Fn 层把同一个 D2 映射为 `S`，因此它可以在不修改固件的情况下区分“Ctrl 没有可见字符”和“D2 电气链路没有被扫描到”。

### 对后续的好处

- 建立了修饰键的正确测试方法；
- 排除了 UF2 文件损坏、错误 Board-ID 和 USB 枚举失败；
- 如果 Fn+左键仍无输出，可以直接进入 D2 专用诊断固件或万用表通断检查，不必重复怀疑整个 ZMK 工程。

### 本次验证

- `NICENANO`：刷写后自动消失；
- USB Bus Reported Device Description：`GALPANEL`；
- VID/PID：`1D50:615E`；
- Keyboard、Consumer Control、Mouse 接口状态均为 `OK`；
- 项目 Git 状态在修改前与 `origin/main` 同步。

### 尚未完成

- 请用户按一次右下 Fn 侧键，再按左机械键，观察是否输入 `s`；
- 请用户使用 Windows 屏幕键盘观察左 Ctrl 是否高亮；
- 若两项均失败，下一步生成 D2 可见字符诊断固件，并检查 D2 到 GND 的硬件通断。

### 计划提交

```text
docs: record left key reflash diagnosis [skip ci]
```

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

### 固定版本后的构建重试

- Run #2（ID `30197411201`）未进入 ZMK 编译，GitHub Runner 在初始化容器时多次出现 `Docker pull failed with exit code 1`；
- 后续 `ContainerId` 为空是容器创建失败的连带错误，不是 Kconfig、Devicetree 或 GALPANEL 源码错误；
- 采用新增一次可追溯提交的方式重新触发干净构建，不修改固件行为。
- Run #3（ID `30197484437`）构建成功，总耗时 3 分 33 秒，证明固定 ZMK revision 后可以重复构建；
- Run #3 最终 ZIP SHA-256 为 `edb08c7919bb8f04c1da92461d07cf54e0e7e5ef6418b821eb3b00bc1e33c0a4`；
- Run #3 的 `galpanel.uf2` 与第一次刷入文件 SHA-256 完全一致，因此没有重复刷写开发板。

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
- 固定版本复验：<https://github.com/Rzl6/galpanel/actions/runs/30197484437>，结论 `Success`；
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
