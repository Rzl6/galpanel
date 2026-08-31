# GALPANEL 工作日志

本文件记录每轮开发做了什么、为什么这么做、对后续有什么好处、如何验证以及遗留问题。

日志按时间倒序追加。完成一轮代码或硬件相关工作后，必须先更新本文件，再创建 Git 提交。

---

## 2026-08-31 - 修复 EC11 鼠标滚轮脉冲过短

### 本次完成

- 根据实物反馈“最新 EC11_TEST 仍然滚动无反应”，检查 ZMK `&msc` 的实现；
- 确认 `&msc` 是 `zmk,behavior-input-two-axis` 按住周期输出行为，不是立即发送一次滚轮值；
- 将 EC11 滚轮行为的 `tap-ms` 从 `5 ms` 调整为 `20 ms`；
- 将本项目的滚轮量宏 `ZMK_POINTING_DEFAULT_SCRL_VAL` 从默认 `10` 调整为 `100`，使首个 16 ms 输入周期即可产生非零滚轮报告；
- 同步修改正式 `galpanel` keymap 和 `galpanel_ec11_test` 诊断 keymap。

### 为什么这么做

原配置在旋钮事件中只保持 5 ms。ZMK 鼠标滚轮行为默认每 16 ms 计算一次移动量，滚轮速度为 10 时，5 ms 内不会形成可发送的整数滚轮单位；释放动作又会取消后续周期任务，因此电脑能看到鼠标接口，却收不到滚轮报告。

### 对后续的好处

- 每个明显 EC11 刻度都能稳定产生至少一个滚轮 HID 报告；
- 延迟约 20 ms，仍符合旋钮操作的即时感；
- 正式固件和诊断固件的滚轮参数一致，测试结果可直接作为产品回归依据；
- 不改变已经验证过的 A/B 引脚、方向和每圈 20 个刻度设置。

### 本次验证

- 已阅读当前 ZMK `behavior_input_two_axis.c`：默认 `trigger-period-ms=16 ms`，滚轮行为在 release 时取消周期任务；
- 已确认 Windows 当前枚举出 GALPANEL 的 `HID-compliant mouse` 集合，说明 HID 描述符已经存在；
- GitHub Actions Run #29（Run ID `33376150467`）五个目标全部构建成功；
- 已下载新版 `galpanel-ec11-test.uf2`，长度 `404480` bytes，SHA-256 为 `A55E7B06DD8FD946C43FF9028849A5C68BCBA72121ABB55DC731807B12ED7F77`；
- 已检测到 `E:` 盘符，卷标为 `NICENANO`；
- 已复制 `galpanel-ec11-test.uf2` 到 `E:`；
- 复制后 `E:` 盘符自动消失，确认设备已跳转运行新版应用固件。

### 尚未完成

- 实物确认顺时针下滚、逆时针上滚，以及快速连续刻度无漏步。

### 计划提交

```text
fix: lengthen EC11 mouse scroll pulse
```

---

## 2026-08-31 - 修复 EC11_TEST 未启用鼠标 HID

### 本次完成

- 根据用户反馈修正版仍无网页滚动，继续检查 EC11_TEST 的 Kconfig；
- 定位遗漏：`galpanel_ec11_test.conf` 只启用了 EC11，没有启用 `CONFIG_ZMK_POINTING=y`；
- 已补充鼠标/指针功能配置，保持 `sensor-rotate → &msc` 绑定不变。

### 为什么这么做

`&msc` 是鼠标滚轮行为，必须同时有 ZMK pointing 功能和对应 HID 报告支持。正式 `galpanel.conf` 已启用 pointing，EC11_TEST 漏掉后会出现“构建成功但网页没有滚动”。

### 对后续的好处

- 测试固件与正式固件拥有一致的鼠标 HID 能力；
- 可以区分传感器事件正常但 HID 输出未启用的配置问题；
- 后续网页滚动测试结果将真正覆盖最终产品链路。

### 本次验证

- 静态项目检查待执行；
- 等待云端重新构建；
- 尚未烧录本次修正版。

### 尚未完成

- 云端构建通过；
- 重新烧录；
- 网页确认顺时针下滚、逆时针上滚。

### 计划提交

```text
fix: enable pointing for EC11 test
```

---

## 2026-08-31 - 烧录修正版 EC11 鼠标 HID 测试固件

### 本次完成

- 修复 EC11_TEST 中错误使用 `&inc_dec_kp` 导致滚轮无响应的问题；
- 将自定义 `sensor-rotate` behavior 放置到正确的根级 `/behaviors` 节点；
- Run #12（Run ID `33374745278`）全部构建成功；
- 下载包 SHA-256：`67fcc60f18b04b78c800d603481d840f3b92227deddc31f4c5667b13d23a6c97`；
- 烧录 `galpanel-ec11-test.uf2`，UF2 SHA-256：`239b09974a7aeeac29690b9022343a3b0073cac3d0fc54590beb8b60b6e719f0`；
- 目标 Board-ID 为 `nRF52840-nicenano`，烧录后 `E:` 自动消失。

### 为什么这么做

滚轮属于鼠标 HID 报告，必须由 `zmk,behavior-sensor-rotate` 直接调用 `&msc`。此前 `inc_dec_kp` 只能调用键盘 `&kp`，所以旋钮虽能产生传感器事件，却不会让网页滚动。

### 对后续的好处

- 诊断固件和正式固件现在使用同一条鼠标 HID 逻辑；
- 网页滚动结果可以直接作为正式基础层功能的回归证据；
- 错误已经从“硬件无响应”收敛为并修复为“行为类型配置错误”。

### 本次验证

- 云端五目标构建：通过；
- UF2 摘要与格式：通过；
- 烧录复制：完成；
- 等待用户确认网页顺时针下滚、逆时针上滚。

### 尚未完成

- 快速连续刻度漏步检查；
- EC11 按压 `9` 复测；
- RST 双击进入 Bootloader；
- 刷回正式 `galpanel.uf2`。

### 计划提交

```text
test: flash mouse HID EC11 diagnostic firmware
```

---

## 2026-08-31 - 修复 EC11_TEST 未发送鼠标滚轮报告

### 本次完成

- 根据用户反馈“旋钮滚动没反应”检查 EC11_TEST keymap；
- 定位错误：使用 `&inc_dec_kp SCRL_UP SCRL_DOWN`，该行为内部是 `&kp` 键盘行为，不是 `&msc` 鼠标行为；
- 改为专用 `zmk,behavior-sensor-rotate` 节点，方向绑定为 `&msc SCRL_UP` 与 `&msc SCRL_DOWN`；
- 保留已验证的硬件方向：顺时针下滚、逆时针上滚；
- 更新测试计划和学习日志，明确键盘 HID 与鼠标 HID 的区别。

### 为什么这么做

滚轮事件属于鼠标 HID 报告，不能通过键盘 keycode 发送。之前的测试方向键版本能看到 `Up/Down`，但改成 `inc_dec_kp` 后只是把滚轮常量错误地交给了键盘行为，因此网页没有滚动。

### 对后续的好处

- EC11_TEST 与正式固件使用同一类 `sensor-rotate → &msc` 链路；
- 网页滚动测试可以直接验证最终鼠标 HID 行为；
- 正式固件的基础层滚轮配置不需要另改，只需确认实物方向。

### 本次验证

- 代码和结构静态检查：通过；
- 首次云端构建暴露设备树层级错误：自定义 behavior 被错误放在 `/keymap/behaviors`；
- 已移到根节点 `/behaviors`，等待下一次云端编译确认。

### 尚未完成

- 云端构建；
- 重新烧录修正版 EC11_TEST；
- 网页确认顺时针下滚、逆时针上滚。

### 计划提交

```text
fix: route EC11 test through mouse HID
```

---

## 2026-08-31 - 烧录修正版 EC11 滚轮固件

### 本次完成

- 直接烧录 Run #10 生成的 `galpanel-ec11-test.uf2`；
- 烧录前确认 `E: NICENANO`，Board-ID 为 `nRF52840-nicenano`；
- 下载包 SHA-256：`61222ddbe3ea2ecf32436b14cbf567be02f9a6a016889a1ce7a68813f7fb5a7a`；
- UF2 SHA-256：`6ad5c49eb8e0f3c57099e86277b13e2ef1a7402360f8fedf3f6601e0a8bbbbf6`；
- UF2 长度 394240 字节、770 个 512 字节块；
- 复制完成后 `E:` 自动消失，确认应用重启。

### 为什么这么做

该版本已将 EC11 测试和正式功能统一为鼠标滚轮语义，并根据实测硬件方向反转绑定顺序：顺时针下滚，逆时针上滚。

### 对后续的好处

- 可以在真实网页上直接验证最终产品滚轮行为；
- 若网页方向正确，正式固件无需再次调整 EC11 基础方向；
- Fn 层音量方向也沿用同一硬件方向校正。

### 本次验证

- Bootloader 目标与 UF2 格式：通过；
- 修正版烧录：完成；
- 等待用户网页滚动实测。

### 尚未完成

- 确认顺时针下滚、逆时针上滚；
- EC11 快速刻度漏步检查；
- 刷回正式 `galpanel.uf2`。

### 计划提交

```text
test: flash corrected EC11 scroll firmware
```

---

## 2026-08-31 - 校正 EC11 正式滚轮方向

### 本次完成

- 根据实测“顺时针对应诊断键值 Up、逆时针对应 Down”，确认本硬件的顺时针是驱动负方向；
- 正式基础层改为：顺时针滚轮下滚、逆时针滚轮上滚；
- 正式 Fn 层改为：顺时针音量增加、逆时针音量降低；
- EC11_TEST 改为实际鼠标滚轮输出，不再发送键盘 Up/Down；
- 更新测试计划和学习日志，区分“诊断可见方向键”和“最终产品滚轮行为”。

### 为什么这么做

此前 `EC11_TEST` 使用键盘方向键只是为了在记事本里观察方向，但它不代表最终功能。根据 ZMK 传感器绑定顺序和实测方向，正式绑定必须按硬件极性反转，才能满足“顺时针下滚、逆时针上滚”。

### 对后续的好处

- 正式固件的基础层符合最初产品设计；
- Fn 层音量方向也与旋钮直觉一致；
- EC11_TEST 与正式功能使用同一滚轮语义，回归测试更可靠；
- 不需要修改 PCB，只调整软件绑定顺序。

### 本次验证

- 项目结构检查：通过；
- `git diff --check`：通过；
- GitHub Actions Run #10（Run ID `33373877764`）五个目标全部构建成功；
- 正式 `galpanel` 与修正版 `galpanel_ec11_test` 均已生成可烧录 UF2；
- 本轮尚未向开发板烧录。

### 尚未完成

- 刷入修正后的 EC11_TEST，确认网页顺时针下滚、逆时针上滚；
- 刷回正式 `galpanel.uf2` 完成最终验收。

### 计划提交

```text
fix: correct EC11 scroll direction
```

---

## 2026-08-31 - EC11 刻度与方向测试通过

### 本次完成

- 用户确认顺时针每个明显刻度输出一次 `Up`；
- 用户确认逆时针每个明显刻度输出一次 `Down`；
- 确认 EC11 的测试单位应是单个机械 detent，而不是普通按键长按或必须统计整圈；
- T3 EC11 方向与离散步进初步通过。

### 为什么这么做

EC11 编码器由 A/B 相位变化产生离散方向事件。一个可感知刻度对应一次事件，连续旋转只是多个事件的连续序列。此前“按整圈统计”的说法只是附加的漏步压力测试，不是功能定义。

### 对后续的好处

- 正式固件可以把每个刻度稳定映射为滚轮、音量或其他单步动作；
- 顺时针 `Up`、逆时针 `Down` 的方向语义已经有实物依据；
- 后续只需补充快速连续刻度、按压复测和 RST，不必修改 PCB 或基础 EC11 配置。

### 本次验证

- 顺时针：每明显刻度一次 `Up`：通过；
- 逆时针：每明显刻度一次 `Down`：通过；
- 未观察到必须长按或整圈才触发的现象：通过。

### 尚未完成

- 快速连续刻度是否漏步或乱跳；
- EC11 按压是否稳定输出 `9`；
- RST 双击进入 Bootloader；
- 刷回正式 `galpanel.uf2`。

### 计划提交

```text
docs: record EC11 detent direction pass
```

---

## 2026-08-31 - 烧录 EC11_TEST 固件

### 本次完成

- 烧录目标：`galpanel-ec11-test.uf2`；
- 烧录前确认 `E: NICENANO` 与 Board-ID `nRF52840-nicenano`；
- 核验 UF2：394240 字节、770 个 512 字节块，SHA-256 `07fa4612ca468464900ce5b9c26db3cdcae0c7ad8df511bf4bc7b2fc248f91ed`；
- 复制完成后盘符自动消失，确认 EC11_TEST 应用已启动。

### 为什么这么做

T2 已确认 9 个普通按键全部正常，EC11_TEST 将测试重点转移到 A/B 相位、旋转方向、步数、快速旋转漏步以及旋钮按压，同时保留数字按键作为回归检查。

### 对后续的好处

- 可以确认 EC11 硬件 RC、A/B 走线和 Zephyr EC11 驱动；
- 若方向相反，只需交换软件 `UP/DOWN` 绑定，不改 PCB；
- 若步数稳定，正式固件的滚轮/音量行为就有可靠输入基础。

### 本次验证

- Bootloader Board-ID：通过；
- UF2 格式与摘要：通过；
- 烧录复制：完成；
- `E:` 自动消失：已观察到；
- 等待用户报告旋转和按压结果。

### 尚未完成

- 顺/逆时针方向确认；
- 每圈步数与快速旋转漏步检查；
- EC11 按压 `9` 复测；
- RST 双击进入 Bootloader；
- 刷回正式 `galpanel.uf2`。

### 计划提交

```text
test: flash EC11 diagnostic firmware
```

---

## 2026-08-31 - 九键 IO_TEST 全部通过

### 本次完成

- 用户确认 `galpanel-io-test.uf2` 下 9 个普通按键全部正常；
- D2/D3/D4 三个机械键分别输出 `1/2/3`；
- D5/D7/D14/D18 四个侧键分别输出 `4/5/7/8`；
- D8 SYS 输出 `6`；
- D19 EC11 按压输出 `9`；
- 短按、保持、松开和再次短按均无异常；
- T2 九键 GPIO 输入测试通过。

### 为什么这么做

IO_TEST 将正式宏替换为简单数字，使测试证据直接对应每个 GPIO 输入。九键全部正常，说明整板按键焊接和软件扫描链路已经闭环。

### 对后续的好处

- 不需要再单独排查机械键、侧键、SYS 或 EC11 按压的基本 GPIO 通路；
- 正式固件中的 `Ctrl/A/Enter/F11/Win+D/Profile/Fn/Esc` 映射有了可靠硬件基础；
- 下一步 EC11_TEST 可以把注意力集中在 A/B 相位、方向、步数和机械按压手感。

### 本次验证

- 9 个键全部有且仅有对应数字输出：通过；
- 保持按下无异常连发：通过；
- 松开后能够再次触发：通过；
- T2 结论：通过。

### 尚未完成

- EC11 顺/逆时针方向和每圈步数；
- EC11 按压复测（IO_TEST 已初步通过）；
- RST 双击进入 Bootloader；
- 刷回正式 `galpanel.uf2` 并完成整板功能验收。

### 计划提交

```text
docs: record nine-key IO test pass
```

---

## 2026-08-31 - 烧录九键 IO_TEST 固件

### 本次完成

- 烧录目标：`galpanel-io-test.uf2`；
- 烧录前确认 `E: NICENANO` 与 Board-ID `nRF52840-nicenano`；
- 核验 UF2：389632 字节、761 个 512 字节块，SHA-256 `7eedb44c59d19642f02d0bdcdd5fddba3e39aea82ac7b9c4884304796a8e2183`；
- 复制完成后盘符自动消失，确认 IO_TEST 应用已启动。

### 为什么这么做

LED 独立控制已通过，下一步需要把九个普通按键映射成可见数字，隔离正式宏和蓝牙 Profile 行为，逐路确认焊点、GPIO 映射、消抖和释放。

### 对后续的好处

- 机械键、侧键、SYS 和 EC11 按压可以用同一套短按/保持/松开流程验证；
- 数字输出便于发现漏检、连发、串键或松开不释放；
- IO_TEST 通过后，EC11_TEST 只需关注旋转 A/B 相和方向/步数。

### 本次验证

- Bootloader Board-ID：通过；
- UF2 格式与摘要：通过；
- 烧录复制：完成；
- 等待用户按键实测 1～9。

### 尚未完成

- D2/D3/D4 三个机械键；
- D5/D7/D14/D18 四个侧键；
- D8 SYS；
- D19 EC11 按压；
- 测试结束后烧录 `galpanel-ec11-test.uf2`。

### 计划提交

```text
test: flash nine-key IO diagnostic firmware
```

---

## 2026-08-31 - LED 独立控制测试通过

### 本次完成

- 用户确认三个机械键分别对应三个 LED 指示状态；
- 左机械键 → Num Lock → INFO 蓝灯；
- 中机械键 → Caps Lock → LINK 绿灯；
- 右机械键 → Scroll Lock → FN 白灯；
- 确认机械键不再附带正式固件的 `A` 字符输出；
- T1 LED 独立控制前三路验证通过。

### 为什么这么做

此前旧版测试固件出现“LED 指示正常，但机械键同时输出 `a`”，原因是正式 keymap 被错误加载。修复版重新烧录后，三个机械键只执行测试 keymap 中的锁定键行为，证明 keymap 选择和 HID 指示链路均已正确。

### 对后续的好处

- INFO、LINK、FN 三路 LED 已完成“逐路独立点亮/熄灭”验证；
- 五灯公共回路与前三路独立控制均通过，LED 硬件层风险大幅收敛；
- 后续正式固件接入 LED 状态事件时，可以直接复用这三路 GPIO；
- 测试流程可以进入九键 GPIO 输入，不再需要重复 LED 排查。

### 本次验证

- 三个机械键分别触发 Num/Caps/Scroll Lock：通过；
- 三颗对应 LED 分别亮灭：通过；
- 中机械键不再输入 `a`：通过；
- T1 LED 独立控制前三路：通过。

### 尚未完成

- WARN 红、AUX 蓝的 Compose/Kana HID 输出验证；
- 九个普通按键的 `IO_TEST` 数字输出验证；
- EC11 旋转、按压和 RST 硬件复位验证。

### 计划提交

```text
docs: record independent LED control pass
```

---

## 2026-08-31 - 烧录修复版 LED_IND_TEST

### 本次完成

- Run #9（Run ID `33370272455`）正式、LED、IO、EC11 五个构建目标全部成功；
- 新 `galpanel-led-ind-test.uf2` 使用 `config/galpanel_led_ind_test.keymap`，避免再次加载正式 `galpanel.keymap`；
- 核验新 UF2：392704 字节、767 个 512 字节块，SHA-256 `cba5d223ab0b5dd4bafa191c369f8885a7ea7cf79864bfe17c3f569a29dd8092`；
- 确认 `E: NICENANO` 的 Board-ID 为 `nRF52840-nicenano` 后完成烧录；
- 烧录后盘符自动消失，确认应用重启。

### 为什么这么做

旧版虽然 LED HID 指示位正常，但 keymap 选中了正式层，导致中机械键同时发送 Caps Lock 和 `A`。本版已修复 ZMK keymap 优先级，并将测试键值改为有效的 `KP_NUMLOCK/CAPSLOCK/SCROLLLOCK`。

### 对后续的好处

- 中机械键不再混入正式宏，LED 独立控制结果可信；
- 同一根因已同时修复 IO_TEST 和 EC11_TEST 的 keymap 选择；
- 后续测试可按设备名 `GP LED IND` 和实际输出行为双重确认固件版本。

### 本次验证

- 云端构建：全部通过；
- UF2 格式和摘要：通过；
- 目标盘符/Board-ID：通过；
- 修复版烧录：完成；
- 等待用户确认中机械键不再输出 `a`。

### 尚未完成

- 用户实测 INFO/LINK/FN 三路独立键控；
- WARN/AUX HID 输出工具验证；
- 转入 `galpanel-io-test.uf2`。

### 计划提交

```text
test: flash fixed LED indicator keymap
```

---

## 2026-08-31 - 修复诊断固件加载正式 keymap 的问题

### 本次完成

- 根据用户实测确认 `GP LED IND` 中 LED 指示功能正常，但中机械键同时输出 `A/a`；
- 检查已烧录 UF2，发现同时包含正式层名 `GALGAME/FN`，证明诊断 keymap 未被选中；
- 阅读 ZMK `post_boards_shields.cmake`，确认 keymap 搜索会先使用 shield 目录名 `galpanel`，因此优先找到 `galpanel.keymap`；
- 在 `config/` 根目录新增四个诊断目标的专用 keymap 覆盖文件，使精确测试目标在进入 shield 目录前被选中；
- 扩展项目验证脚本，强制检查这些根目录覆盖文件。

### 为什么这么做

ZMK 的 overlay、Kconfig 和 keymap 并非使用完全相同的文件选择路径。此前测试 overlay 和设备名正确生效，但 keymap 搜索先命中了父目录中的正式 `galpanel.keymap`，于是形成“测试 LED + 正式按键”的混合固件。仅重复烧录同一 UF2 无法解决源码选择错误。

### 对后续的好处

- LED_IND_TEST 的三个机械键将真正发送 Num/Caps/Scroll Lock，不再附带正式 A/Enter 等行为；
- IO_TEST 将真正输出 `1～9`，避免之后再次把正式宏误认为测试结果；
- EC11_TEST 将加载专用 Up/Down 旋转绑定；
- 根目录覆盖文件清晰记录了 ZMK 的 keymap 优先级要求，新增诊断固件可复用同一结构。

### 本次验证

- 用户实物证据：Caps Lock 指示灯响应与中键输出 `a` 同时存在；
- 旧 UF2 字符串包含 `GP LED IND` 和正式层名 `GALGAME/FN`；
- 已定位 ZMK CMake keymap 候选顺序；
- 等待修复后的云端构建与 UF2 内容复核。

### 修复动作

- 将 HID 指示位改为其明确的数值位掩码 `1/2/4/8/16`，避免 overlay 预处理时的表达式解析问题；
- 将 EC11_TEST 改为 ZMK 官方常用的直接 `&inc_dec_kp DOWN UP` 绑定，移除不必要的自定义 sensor-rotate 节点。
- 将 LED_IND_TEST 键值修正为 `KP_NUMLOCK/CAPSLOCK/SCROLLLOCK`；Compose/Kana 不是普通键值，其余按键保持 `&none`。

### 尚未完成

- 云端重新构建所有目标；
- 确认新 LED_IND_TEST UF2 包含 `LED IND TEST` 且不包含 `GALGAME`；
- 重新烧录并实测中机械键不再输出 `a`。

### 计划提交

```text
fix: select diagnostic keymaps before formal keymap
```

---

## 2026-08-31 - 重新烧录 LED 独立测试固件

### 本次完成

- 因用户观察到中机械键仍输入 `a`，重新进入 `NICENANO` Bootloader；
- 再次确认 Board-ID 为 `nRF52840-nicenano`；
- 使用已核验的 `galpanel-led-ind-test.uf2`（SHA-256 `fea1badacd4c5021c1c1dc4f39b4ca41876c74eb7b91ab35f67f60bd84a9c8d0`）重新烧录；
- 烧录完成后 `E:` 自动消失，确认应用重新启动。

### 为什么这么做

LED_IND_TEST 的中机械键应发送 Caps Lock，不应输入 `A/a`。重新烧录同一份已验证 UF2，可以先排除第一次复制未真正跳转或仍运行旧固件的可能性，再判断映射问题。

### 对后续的好处

- 设备名 `GP LED IND` 和按键行为可以作为固件版本的双重识别；
- 如果重烧后仍输入 `a`，问题范围将收敛到设备枚举/实际运行镜像，而不是云端产物；
- 确认测试镜像后再继续 LED 独立控制，避免错误记录。

### 本次验证

- Bootloader Board-ID：通过；
- UF2 SHA-256：通过；
- 复制烧录：完成；
- `E:` 自动消失：已观察到；
- 等待用户确认设备名和中键行为。

### 尚未完成

- 确认设备名为 `GP LED IND`；
- 确认中机械键切换 LINK 绿灯而非输入 `a`；
- 完成 FN 白灯及后续 IO_TEST。

### 计划提交

```text
test: reflash LED indicator diagnostic firmware
```

---

## 2026-08-31 - LED 独立控制前三路测试进展

### 本次完成

- 用户确认 Num Lock 对应的 INFO 蓝灯正常响应；
- 用户确认 Caps Lock 对应的 LINK 绿灯正常响应；
- 用户电脑没有独立的 Scroll Lock 按键，补充说明由 GALPANEL 右机械键发送 Scroll Lock。

### 为什么这么做

测试固件的三个第一排机械键已经分别绑定 Num Lock、Caps Lock、Scroll Lock，因此不需要主机键盘物理存在这些按键。右机械键本身就是 Scroll Lock 的测试触发器。

### 对后续的好处

- 不依赖电脑键盘布局即可完成前三路 LED 独立控制验证；
- INFO 和 LINK 已经确认软件指示链路与硬件支路独立正常；
- FN 只需按 GALPANEL 右机械键即可继续验证。

### 本次验证

- INFO 蓝：通过；
- LINK 绿：通过；
- FN 白：等待用户按 GALPANEL 右机械键验证；
- WARN 红/AUX 蓝：仍需 HID Compose/Kana 输出工具或后续专用测试方案。

### 尚未完成

- 右机械键 → Scroll Lock → FN 白灯开关；
- WARN、AUX 独立控制；
- 刷入 `galpanel-io-test.uf2`。

### 计划提交

```text
docs: record initial LED indicator results
```

---

## 2026-08-31 - 烧录 LED 独立控制测试固件

### 本次完成

- 烧录目标：`galpanel-led-ind-test.uf2`；
- 烧录前确认开发板为 `E: NICENANO`，Board-ID 为 `nRF52840-nicenano`；
- 核验 UF2 长度 398848 字节、779 个 512 字节块，UF2 魔数正确；
- 复制完成后 `E:` 自动消失，确认 Bootloader 已跳转到 LED 独立控制测试应用。

### 为什么这么做

T1 全亮测试已经证明五路 LED 电气链路成立，现在需要验证各路是否能独立受控。该固件等待主机 HID 指示灯状态，不会默认点亮 LED，避免把“全部常亮”误认为独立控制通过。

### 对后续的好处

- 可以区分单路 LED 控制问题和公共回路问题；
- 确认 INFO/LINK/FN 三路后，再决定是否需要专用 HID 输出工具验证 WARN/AUX；
- 独立控制通过后即可安全进入 IO_TEST，不再重复排查 LED 硬件。

### 本次验证

- Bootloader 盘符与 Board-ID：通过；
- UF2 格式：通过；
- 复制烧录：完成；
- 等待用户报告上电后的 LED 初始状态和逐路响应。

### 尚未完成

- Num Lock → INFO 蓝；
- Caps Lock → LINK 绿；
- Scroll Lock → FN 白；
- Compose/Kana → WARN 红/AUX 蓝（需要主机发送对应 HID 指示位）；
- 完成后刷入 `galpanel-io-test.uf2`。

### 计划提交

```text
test: flash independent LED diagnostic firmware
```

---

## 2026-08-31 - 澄清整板输入数量与测试边界

### 本次完成

- 根据用户指出的硬件组成，重新明确“9 个 kscan 按键”与“整板 11 种动作”的区别；
- 在 IO_TEST 与 EC11_TEST keymap 中补充 9 个位置的真实网络注释；
- 在测试计划和学习日志中说明：EC11 A/B 是一个旋转传感器，RST 是硬件复位，不属于普通按键扫描；
- 保持测试固件代码逻辑不变，因为现有 9 个 keymap 位置已经覆盖 3 个机械键、4 个侧键、SYS 和 EC11 按压。

### 为什么这么做

此前“九路输入”的说法把软件扫描通道和硬件动作总数混在了一起。对于 STM32 经验来说，EC11 的 A/B 是两个 GPIO 边沿输入，但在 ZMK 中会被 EC11 驱动合并为一个 sensor；RST 则直接复位 nRF52840，不能由应用层 keymap 读取。

### 对后续的好处

- IO_TEST 的数字序列与实物按键一一对应，不会误以为漏测 SYS、旋钮或 RST；
- EC11_TEST 可以同时覆盖 9 个按键、旋转传感器和 EC11 按压；
- RST 测试单独保留为 Bootloader/复位验收，避免在固件中错误占用复位脚；
- 后续测试记录可以区分“GPIO 输入”“传感器事件”“硬件复位”三类证据。

### 本次验证

- 对照 `项目认知.md` 的最终引脚表：3 机械键 + 4 侧键 + SYS + EC11_SW = 9 个 kscan 输入；
- 对照 `galpanel.overlay`：EC11 A/B 为 D21/D20，RST 不在 `input-gpios`；
- 项目结构检查和差异检查待提交前执行。

### 尚未完成

- 用户回家后实测 9 个数字按键；
- 实测 EC11 方向、每圈步数和按压；
- 单独实测 RST 双击进入 Bootloader。

### 计划提交

```text
docs: clarify input count and reset boundaries
```

---

## 2026-08-31 - 设计后三个独立诊断固件

### 本次完成

- 新增 `galpanel_led_ind_test`：使用 ZMK HID indicator-leds，将五路 LED 映射到 Num/Caps/Scroll/Compose/Kana 指示位；
- 新增 `galpanel_io_test`：九路普通输入分别输出 `1～9`，禁用 EC11 与 LED，减少测试变量；
- 新增 `galpanel_ec11_test`：保留九路数字输入，同时把 EC11 旋转映射为 `Up/Down`、按压映射为 `9`；
- 在 `build.yaml` 中登记三个独立 artifact：`galpanel-led-ind-test`、`galpanel-io-test`、`galpanel-ec11-test`；
- 为三个 shield 增加独立 Kconfig、metadata、overlay、keymap 和配置文件；
- 扩展项目验证脚本，检查三个固件的文件、构建入口和关键绑定；
- 更新《测试计划.md》和《学习日志.md》，明确三套固件的测试边界和执行顺序。

### 为什么这么做

LED 全亮已通过，但仍需证明每路能独立控制；九路输入和 EC11 也需要可见、无副作用的输出。把三类问题拆成三份固件后，用户可以用字符、方向键和 HID 指示位分别判断硬件层，而不必同时承担正式宏、BLE Profile 和 LED 状态逻辑。

### 对后续的好处

- 每个 artifact 都能单独刷写、回退和记录；
- IO_TEST 的数字输出避免 F11、Win+D 等危险或不可见按键造成误判；
- EC11_TEST 可以一次覆盖普通按键、旋钮 A/B 相位和按压开关；
- LED 独立测试完成后，正式固件接入状态灯时已有硬件基线；
- 以后若云端某个目标失败，日志可以直接定位到对应 shield，而不是整板配置。

### 本次验证

- 项目结构验证脚本：通过；
- `git diff --check`：通过；
- GitHub Actions Run #6（Run ID `33346436573`）五个构建目标全部成功：`galpanel`、`galpanel_led_test`、`galpanel_led_ind_test`、`galpanel_io_test`、`galpanel_ec11_test`；
- firmware ZIP SHA-256 为 `3c823b77a0956c4a07ea190d6e4d2d0cf9df3396772b785d7312d6f61599d60f`，与 GitHub artifact digest 匹配；
- 已下载并核对五个 UF2，保存在 `artifacts/run-33346436573/firmware/`，本轮不向开发板烧录。

### 尚未完成

- 用户回家后按 LED 独立、IO、EC11 顺序烧录和实测。

### 计划提交

```text
test: add LED, IO, and EC11 diagnostic firmware
```

---

## 2026-08-31 - LED_TEST 五灯全亮实物验证通过

### 本次完成

- 用户确认 INFO 蓝、LINK 绿、FN 白、WARN 红、AUX 蓝五颗 GALPANEL LED 全部常亮；
- 将 T1 LED 电气测试第一阶段标记为通过；
- 确认此前“GALPANEL LED 一个都不亮”是正式固件尚未实现状态驱动造成的，不是五路 LED 硬件同时损坏。

### 为什么这么做

全亮固件将五个 GPIO 同时置高，实物五灯全部常亮说明从主控 GPIO、载板走线、各路限流电阻、LED 极性到公共 `LED_GND/R_LED_EN` 回路均已形成有效电流通路。

### 对后续的好处

- 后续开发 BLE、Fn、WARN 等状态灯行为时，可把主要注意力放在软件事件和亮度策略上；
- 不需要拆焊或逐颗返修 LED；
- 下一版逐灯测试只需验证各路能否独立关断，以及颜色与网络是否一一对应；
- T2 IO_TEST 可以在 LED 独立控制确认后继续进行。

### 本次验证

- INFO（D9/P1.06，蓝）：常亮；
- LINK（D6/P1.00，绿）：常亮；
- FN（D15/P1.13，白）：常亮；
- WARN（D1/P0.06，红）：常亮；
- AUX（D0/P0.08，蓝）：常亮；
- T1 第一阶段结论：通过。

### 尚未完成

- 确认长时间全亮时无异常发热、闪烁或 USB 反复重启；
- 编写逐灯循环版本，验证五路 LED 能够独立控制；
- 逐灯验证通过后进入 T2 IO_TEST。

### 计划提交

```text
docs: record five-LED hardware test pass
```

---

## 2026-08-30 - LED_TEST 云端构建、校验与烧录完成

### 本次完成

- 提交并推送 BLE 名称修复，commit 为 `5d3c975`；
- GitHub Actions Run #5（Run ID `33320544045`）中，正式 `galpanel` 与 `galpanel_led_test` 均构建成功；
- 下载 `firmware` artifact，并核对 ZIP SHA-256 为 `4e638f3c34875822db70bbe1f8036715cae3aa4559ecc3f260ee2e55a3c57335`，与 GitHub artifact digest 完全一致；
- 核验 `galpanel-led-test.uf2` 为 396800 字节、775 个 UF2 块，双魔数正确；
- 烧录前再次确认目标为 `E: NICENANO`，Board-ID 为 `nRF52840-nicenano`；
- 将 `galpanel-led-test.uf2` 复制到开发板，盘符随后自动消失，确认 Bootloader 已重启到应用；
- 更新《测试计划.md》和《学习日志.md》，使全亮测试行为与真实实现一致。

### 为什么这么做

烧录测试固件会直接改变开发板当前运行程序，因此必须同时确认“云端产物未损坏、UF2 格式正确、目标盘符正确”。三层核验通过后再复制，可以避免把下载错误或选错设备误判为 PCB 故障。

### 对后续的好处

- 已证明 LED_TEST 从源码、GitHub Actions 到 UF2 Bootloader 的完整软件链路成立；
- 用户现在只需报告五颗 GALPANEL LED 的可见结果，即可把问题边界收敛到硬件；
- 测试产物与正式固件同时保存在忽略目录中，后续可快速切换而不污染 Git；
- 教程保留了构建错误、摘要校验和烧录判断方法，后续 IO_TEST 可复用同一流程。

### 本次验证

- 项目结构检查：通过；
- Run #5 全部 jobs：成功；
- artifact SHA-256：匹配；
- UF2 块对齐与魔数：通过；
- 烧录复制：完成；
- `NICENANO` 盘符自动消失：已观察到。

### 尚未完成

- 等待用户报告 INFO、LINK、FN、WARN、AUX 的实际点亮情况；
- 等待确认是否存在异常发热、闪烁或 USB 反复重启；
- 根据实物结果决定进入逐路 LED 诊断还是 T2 IO_TEST。

### 计划提交

```text
docs: record successful LED test flash
```

---

## 2026-08-30 - 修正 LED_TEST 蓝牙名称长度

### 本次完成

- 检查 GitHub Actions Run #4（Run ID `33319996295`）的完整构建日志；
- 确认正式 `galpanel` 构建成功，只有独立 `galpanel_led_test` 构建失败；
- 将 LED_TEST 的 `ZMK_KEYBOARD_NAME` 从 `GALPANEL LED TEST` 缩短为 `GP LED TEST`；
- 保留 GPIO hog 全亮测试逻辑不变。

### 为什么这么做

ZMK 对 BLE 设备名执行编译期长度检查，最大为 16 字节。原名称超过限制，导致 Kconfig 静态断言失败；这与五路 LED 的 GPIO 配置和硬件无关。

### 对后续的好处

- 消除当前唯一已知的编译阻塞；
- 保持测试固件在蓝牙设备列表中仍容易识别；
- 不改动 LED 测试电平、正式固件或用户 PCB 文件，减少重新验证范围。

### 本次验证

- Run #4 正式 `galpanel` 构建：通过；
- Run #4 `galpanel_led_test`：仅因 BLE 名称超长失败；
- 构建日志在 `artifacts/led-test-job-99280191833.log` 留存，该目录已被 Git 忽略。

### 尚未完成

- 触发并等待修正后的云端构建；
- 下载、校验和烧录 `galpanel-led-test.uf2`；
- 记录五颗 LED 的实际点亮结果。

### 计划提交

```text
fix: shorten LED test Bluetooth name
```

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
