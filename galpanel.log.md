# GALPANEL 工作日志

本文件记录每轮开发做了什么、为什么这么做、对后续有什么好处、如何验证以及遗留问题。

日志按时间倒序追加。完成一轮代码或硬件相关工作后，必须先更新本文件，再创建 Git 提交。

---

## 2026-09-01 - 清空 Profile 5 后烧录历史 Windows 可用基线

### 本次完成

- 用户已在 BLE recovery 固件中执行：D7 一次选择 Profile 5，D14 一次清除当前 Profile 的 bond；
- 用户双击 RST 后，确认 `E:` 的 `INFO_UF2.TXT` 为 `Board-ID: nRF52840-nicenano`；
- 已烧录历史 Run #3 固件 `artifacts/firmware-run-3/galpanel.uf2`；
- 写入前 SHA-256 已核对为 `6805B40DE4D7214ABE371D7DC0138B99BB5AB18E32165ED1B43DC92ED017F5E4`；
- 写入后三秒内 `NICENANO` 盘符自动消失，确认 Bootloader 接受 UF2 并重新启动。

### 为什么这么做

这次只保留两个受控条件：确定空闲的 Profile 5 和历史上已在本机 Windows 配对成功过的 Run #3 固件。它将当前问题从后来增加的动态蓝牙命名、状态灯和 SYS 行为中隔离出来。

### 对后续的好处

- Windows 若现在能首次配对，说明故障在后续正式固件演进中，可从 Run #3 开始逐项回归；
- Windows 若仍不能配对，硬件与常规 ZMK 应用代码基本排除，应转查 Windows 蓝牙服务、适配器驱动或系统 BLE 状态；
- Profile 1～4 的现有配对记录未受本轮操作影响。

### 本次验证

- Bootloader 型号、Board-ID 与 UF2 的目标板一致；
- 目标 UF2 的 SHA-256 与 2026-07-26 成功记录一致；
- 盘符消失表明烧录已完成。

### 尚未完成

- 等待用户在 Windows 蓝牙设置中搜索并首次配对当前广播设备；
- 记录配对结果及 D2/D3/D4 的 HID 输入结果。

### 关联提交

待提交：`test: flash clean Profile 5 historical BLE baseline`

---

## 2026-09-01 - 找回 Windows 历史可用固件并识别基线测试残留变量

### 本次完成

- 查阅 Git 历史，确认 2026-07-26 的提交 `d24d7f6` 记录了这台 Windows 对 `GALPANEL` 的首次配对成功，以及 D2 BLE Ctrl 输入成功；
- 找到对应的 Run #3 真实 UF2：`artifacts/firmware-run-3/galpanel.uf2`，SHA-256 为 `6805B40DE4D7214ABE371D7DC0138B99BB5AB18E32165ED1B43DC92ED017F5E4`；
- 用户截图确认修正版基线实际广播名已经是 `GP WIN BLE`，但 Windows 仍提示无法连接；因此动态 Profile 名称、状态 LED 和 SYS 自定义代码不再是首要嫌疑；
- 识别出基线测试仍未控制的变量：ZMK BLE Profile/bond 存储位于重刷 UF2 不会自动清除的设置区，手机已绑定当前 Profile 时，Windows 对同一 Profile 的新配对会被 ZMK 拒绝。

### 为什么这么做

“历史上这台 Windows 成功配对过”比泛泛的兼容性猜测更可靠。Run #3 提供了可复现的已知可用程序基线。当前静态名称固件能显示 `GP WIN BLE`，证明它的名称覆盖已生效；但没有先切换到确定空闲的 Profile，不能据此断言 Windows 蓝牙适配器或当前 ZMK 基线损坏。

### 对后续的好处

- 下一次测试可以只改变两个明确变量：Profile 是否干净、固件是否为历史可用 Run #3；
- 无需继续在正式灯效/动态命名代码上做无效修改；
- 可采用 Profile 5 作为独立 Windows 测试槽，保留 Profile 1～4 的现有配对数据。

### 本次验证

- `d24d7f6` 历史日志明确记录 Windows 发现 `GALPANEL`、首次配对通过、BLE 左 Ctrl 通过；
- 本地 Run #3 UF2 的 SHA-256 与当时工作日志完全一致；
- 用户当前截图显示 `GP WIN BLE`，证明修正版基线实际在运行。

### 尚未完成

- 用户已授权 Profile 5 的受控清除；已烧录 BLE recovery UF2（SHA-256 `213EF8F489BFFC9FA8A15D43D46C30444BF36ED64F2D454E55ADDF9815B475F5`），等待按 D7 选择 Profile 5、按 D14 清除该 Profile；
- 完成后刷入 Run #3，形成干净的 Windows 回归测试；
- 若干净 Profile 5 + Run #3 仍失败，再将问题定向到 Windows 蓝牙服务/适配器驱动，而不是 GALPANEL 应用逻辑。

### 关联提交

待提交：`docs: record Windows BLE regression baseline`

---

## 2026-09-01 - 增加 Windows 静态名称 BLE 基线固件

### 本次完成

- 根据 Windows 设备列表中不存在任何 `GALPANEL` 条目的截图，确认当前失败不是 Windows 保留旧 GALPANEL 条目导致；
- 新增 `galpanel-win-ble-baseline` 构建目标，固定广播名为 `GP WIN BLE`；
- 基线固件复用同一块 nRF52840 和 D2～D19 按键 GPIO，但禁用动态 Profile 名称、状态灯/SYS 自定义模块、LED 和 EC11；
- D2/D3/D4 在基线固件中分别输出 `A/B/C`，用于确认 Windows 配对成功后 HID 真的可用；D8 保留 ZMK 原生 `BT_NXT`；
- 在《测试计划》中增加 T11 对照测试与结果判定规则。

### 为什么这么做

手机可以首次配对，Windows 却不能，说明需要用最小化对照实验而不是继续同时修改 Windows 设置、Profile 名称、状态灯和配对代码。静态名称基线只保留 ZMK 标准 BLE 和输入，能明确定位自定义软件与 Windows 主机侧之间的边界。

### 对后续的好处

- Windows 成功时，可以精确审查动态名称模块，而不是怀疑硬件；
- Windows 失败时，可以停止在主控固件上做无关改动，转向适配器驱动和 Windows BLE 栈；
- 基线文件会长期保留为可复现的售后/回归工具。

### 本次验证

- 已核对该 Shield 不满足状态/动态名称模块的 Kconfig 依赖，因此不会链接 `galpanel_status.c` 或 `galpanel_profile_name.c`；
- GitHub Actions Run #58（提交 `243b099`）构建成功；
- 下载 `galpanel-win-ble-baseline.uf2`，长度 `391168` bytes，SHA-256 为 `5E5AAD9F9DB781EB6332ED828B07153CCBB2300B28F8ACF928D1DD8B6204D3E1`；
- 已确认 `E:` / `NICENANO` 的 `Board-ID: nRF52840-nicenano`，并完成 UF2 复制；盘符自动消失，确认基线固件被接受并重启。

### 更正（同日）

首次基线固件虽然包含编译期名称 `GP WIN BLE`，但实物仍显示 `GALPANEL 2`。这是因为此前正式固件的动态命名通过 `bt_set_name()` 更新了 BLE 设置区；日常 UF2 重刷不会抹除该非易失设置。基线固件的“无应用 C 模块”描述不足以处理这个残留状态，已在后续修订中加入仅用于诊断的启动名覆盖模块。该修订不清除任何 bond，仅强制广播名为 `GP WIN BLE`。

后续修订的 GitHub Actions Run #61（提交 `4423a20`）已构建成功；下载的修正版 `galpanel-win-ble-baseline.uf2` 长度 `391680` bytes，SHA-256 为 `7CB1AB956A93405DD33F65D540FCF477D6694F710CADFFC6F1F13E00B6C8603D`。已确认 `E:` / `NICENANO` 的 `Board-ID: nRF52840-nicenano`，完成 UF2 复制后盘符自动消失，确认修正版基线固件被接受并重启。

### 尚未完成

- 尚未烧录并完成 Windows 与手机的对照配对。

### 关联提交

待提交：`test: add Windows BLE pairing baseline`

---

## 2026-09-01 - 确认 Windows 新设备配对失败的 Profile 占用原因

### 本次完成

- 根据初始描述“手机可以连接、Windows 对 `GALPANEL 1` 提示无法连接”，检查固定版本 ZMK 的 BLE 配对实现；
- 确认每个 ZMK BLE Profile 只允许保存一台主机的 bond；
- 确认当前 Profile 已被手机占用时，ZMK 会主动拒绝另一台主机的新配对请求，而不是允许同一 Profile 同时绑定手机和 Windows；
- 确认这不表示 nRF52840 天线、BLE 广播或 HID 服务整体失效：手机成功连接已证明这些基础链路可用。

### 为什么这么做

Windows 的“无法连接，请尝试重新连接”既可能由旧配对缓存造成，也可能由设备端拒绝新配对造成。手机能够连接将问题范围缩小到 Profile/bond 管理或 Windows 主机侧，而 ZMK 的配对保护代码明确解释了“同一 Profile 已绑定手机，Windows 新配对被拒绝”的现象。

### 对后续的好处

- 手机和 Windows 应使用不同 Profile，例如手机使用 `GALPANEL 1`，Windows 使用 `GALPANEL 2`；
- 不必为正常的多设备使用反复清除 bond 或怀疑硬件；
- 只有要把同一 Profile 从手机转交给 Windows 时，才清除该 Profile bond，并同时删除 Windows 中对应旧条目。

### 本次验证

- 对照固定 ZMK 源码 `pairing_allowed_for_current_profile()`：只有空 Profile 才接受新主机配对；
- 对照 `auth_pairing_accept()`：Profile 已占用时返回 `BT_SECURITY_ERR_PAIR_NOT_ALLOWED`；
- 用户已确认手机能够成功连接，证明 BLE 广播和基础 HID 配对路径有效。

### 尚未完成

- 需在空的 Profile 2～5 之一完成 Windows 首次配对；
- 若空 Profile 也失败，再执行该 Profile 的设备端/Windows 端双端 bond 清理，并单独排查 Windows 蓝牙适配器。

### 关联提交

待提交：`docs: record BLE profile pairing diagnosis`

### 更正（同日）

用户随后明确：Windows 和手机都是对空设备进行首次配对，且 Windows 先失败、手机后成功。该条件不符合“Profile 已被手机占用”的前提，因此本条不能作为本次失败的最终原因，只保留为多主机使用时的规则说明。当前诊断应转向 Windows 的 BLE/GATT 缓存、蓝牙适配器和动态 Profile 名称兼容性。

---

## 2026-09-01 - 增加 Windows BLE/GATT 兼容配置

### 本次完成

- 在正式 GALPANEL 配置中加入 `CONFIG_BT_GATT_ENFORCE_SUBSCRIPTION=n`；
- 该项是 ZMK 官方文档给出的 Windows BLE/GATT 兼容配置；
- 更新《使用说明》的 Windows 失败恢复步骤，并规定下一步用静态名称基线固件做 A/B 隔离。

### 为什么这么做

手机可以对空设备首次配对，说明设备的广播、射频、SMP 和 HID 基础功能成立。Windows 失败因此不能再归因于 Profile 占用。ZMK 明确记录 Windows 与默认 GATT 订阅策略的兼容问题，先启用该低风险配置能够减少一个已知 Windows 变量。

### 对后续的好处

- 不改变 5 个 Profile 的一主机一 bond 规则；
- 不改变 Android/iOS 配对路径；
- 若 Windows 仍失败，可将问题收敛到“Windows 主机/适配器”或“动态名称模块”，用静态名称基线固件直接区分。

### 本次验证

- 对照 ZMK 官方 Bluetooth Configuration：该项用于规避 Windows 的 GATT/battery notification 兼容问题；
- GitHub Actions Run #55（提交 `54a9265`）构建成功；
- 下载正式 `galpanel.uf2`，长度 `413696` bytes，SHA-256 为 `49625D62E66C75ECA0ACF035568437FFC659CF4C3BEFBE534D3BEABA9FB43874`；
- 已确认 `E:` / `NICENANO` 的 `Board-ID: nRF52840-nicenano`，并完成 UF2 复制；盘符自动消失，确认 Bootloader 接受固件并重启。

### 尚未完成

- 尚未完成静态名称 BLE 基线 A/B 测试；
- 需确认 Windows 使用的蓝牙适配器和驱动版本，作为软件基线固件之后的下一层排查。

### 关联提交

待提交：`fix: add Windows BLE compatibility setting`

---

## 2026-09-01 - 修正 SYS 短按误亮 WARN

### 本次完成

- 确认正式状态模块中，SYS 按下事件会立即设置 `safety_held`，旧逻辑因此让 WARN 红灯从每次 SYS 按下就开始闪烁；
- 将 WARN 提示延后到 SYS 按住达到 5 秒，只覆盖清除配对前的最后 1 秒；
- SYS 单击切 Profile、双击切换 USB/BLE 时不再点亮 WARN；
- 清除成功后的短确认仍保留，INFO 蓝灯持续 10 秒的行为不变。

### 为什么这么做

WARN 应表示危险操作或错误，而不是普通 SYS 操作。原实现把“进入按住状态”和“进入危险倒计时”混为一谈，导致每次按键都出现红灯反馈，容易误判为故障或已经清除配对。

### 对后续的好处

- 日常 Profile/输出切换不会产生误导性红灯；
- 真正长按清除时，用户仍能在最后 1 秒看到明确的危险提示；
- 可以把 BLE 连接失败与 WARN 灯行为分开验证，减少排障变量。

### 本次验证

- 静态检查确认 WARN 条件现在依赖 `held_ms >= 5000` 或实际清除确认；
- GitHub Actions Run #53（提交 `62ea45b`）构建成功；
- 下载正式 `galpanel.uf2`，长度 `414720` bytes，SHA-256 为 `4CD74E5A4845AD48B7D8539430FF1BE0C2F4B896503372BC11B80A43CA3488D8`；
- 已确认 `E:` / `NICENANO` 的 `Board-ID: nRF52840-nicenano`，并完成 UF2 复制；盘符自动消失，确认固件被接受并重启。

### 尚未完成

- BLE 无法连接仍需在新固件下先确认 LINK 广播状态，再执行双端 bond 清理。

### 关联提交

待提交：`fix: delay WARN until bond-clear hold threshold`

---

## 2026-09-01 - 修复正式固件状态模块并启用 SYS 清除配对

### 本次完成

- 定位到正式 `galpanel` Shield 不满足 `GALPANEL_STATUS_INDICATORS` 的 Kconfig 依赖，导致状态模块虽然写入 `.conf`，却没有进入正式固件；
- 将正式 Shield 加入状态模块和 SYS 安全逻辑的 Kconfig 依赖，并在正式配置中启用 SYS 安全逻辑；
- 正式 SYS 改由状态模块独占：单击切换 Profile、300 ms 内双击切换 USB/BLE、按住约 6 秒清除当前 Profile bond；
- LINK 改为仅表达 BLE 状态：USB 实际输出时熄灭；BLE 未连接时约 1 秒周期慢闪；BLE 连接后亮 30 秒再熄灭；
- INFO 恢复 Profile 1～5 对应 1～5 次闪烁；
- 清除当前 Profile bond 后，INFO 持续亮 10 秒作为明确确认；长按期间 WARN 红灯继续闪烁；
- 更新《使用说明》的正式行为和恢复流程。

### 为什么这么做

用户实测同时出现 LINK 不闪、INFO 不报告 Profile、SYS 长按无效，这三个现象由同一个构建配置错误解释。Profile 名称模块不受该依赖限制，所以 Windows 仍能看到 `GALPANEL 1/2`，但并不代表状态和安全模块已经编入固件。修正编译依赖比继续调整灯效定时更直接。

Windows 显示“无法连接，请尝试重新连接”通常还包含主机旧 bond 与设备当前 bond 不一致。正式固件必须先提供可确认的设备端清除动作，之后才能进行双端删除和重新配对。

### 对后续的好处

- 正式固件与已验证的状态/安全测试模块使用同一份实现，不再出现“测试固件正常、正式固件缺模块”；
- 用户无需为日常单 Profile 恢复反复刷入维修固件；
- INFO 10 秒确认可以明确区分“还在计时”和“已经执行清除”；
- LINK 在有线输出时不闪，避免把 USB 正常状态误判为 BLE 等待连接。

### 本次验证

- 对照固定 ZMK 源码确认 `zmk_ble_clear_bonds()` 只清除当前活动 Profile，并立即刷新广播；
- `validate-project.ps1` 通过；
- GitHub Actions Run #50/#51（提交 `f7c3665`）均构建成功；
- 下载正式 `galpanel.uf2`，长度 `414208` bytes，SHA-256 为 `88064C132A6214FCA6F92234F617FD5316076C50BBE0ADB75254DB6B5F824C0C`；
- 已检测到 `E:` / `NICENANO`，并确认 `Board-ID: nRF52840-nicenano`；
- 已将该 UF2 复制到 `E:`，盘符随后自动消失，确认 Bootloader 接受并重启正式固件。

### 尚未完成

- 尚需实测 LINK、INFO、SYS 6 秒清除和 INFO 10 秒确认；
- 清除后需在 Windows 删除对应的 `GALPANEL N` 条目再重新配对，不能只点击旧条目的“连接”。

### 关联提交

待提交：`fix: enable final status and bond recovery controls`

---

## 2026-09-01 - 最终固件增加 Profile 名称和 LINK 节能策略

### 本次完成

- 正式固件启用状态模块，LINK 绿灯在 BLE 连接成功后亮 30 秒，之后自动熄灭；
- BLE 断开后 LINK 恢复慢闪，下一次连接会重新开始 30 秒显示窗口；
- 新增 Profile 名称模块，当前 Profile 广播名设置为 `GALPANEL 1`～`GALPANEL 5`；
- Profile 名称会在启动和 SYS 单击切换 Profile 后更新；
- 正式固件由状态模块统一驱动 AUX，避免端点模块和状态模块同时写 D0；
- 状态测试固件保留 LINK 连接常亮，便于持续观察，节能策略仅在正式固件启用；
- 更新《使用说明》中的 LED 和蓝牙配对说明。

### 为什么这么做

LINK 常亮会在电池模式下持续消耗 LED 电流，但完全没有连接反馈又不利于排障。30 秒连接确认窗口兼顾可见性和功耗。五个 Profile 使用不同广播名，可以让电脑或手机直接区分绑定槽位，避免只看到多个同名 `GALPANEL` 设备。

### 对后续的好处

- 最终固件在无线使用时降低 LINK 灯长期功耗；
- 主机蓝牙列表可直接识别 Profile，不必依赖 INFO 闪烁次数；
- Profile 切换、LED 提示和实际广播状态由同一事件链驱动；
- 状态测试固件仍保留可观察性，不影响后续维修诊断。

### 本次验证

- 对照固定 ZMK 版本确认 `zmk_ble_set_device_name()` 会更新 BLE 广播名称；
- `galpanel-status-test` 和正式 `galpanel` 目标均已配置独立 LINK 超时参数；
- GitHub Actions Run #48（Run ID `33483010172`）正式固件和全部测试固件构建成功；
- 下载并校验新的正式 `galpanel.uf2`，长度 `412672` bytes，SHA-256 为 `2932835BABE237775A32E51318ED13E7E4E0FB955330A8A8468770378BA4A775`；
- 当前未检测到 `NICENANO` 盘符，因此没有强行烧录，等待开发板重新进入 UF2 Bootloader。
- 2026-09-01，重新进入 `E:` / `NICENANO` 后烧录该正式 UF2；
- 烧录前确认 `INFO_UF2.TXT` 为 `nRF52840-nicenano`，复制后盘符自动消失，确认固件已被 Bootloader 接受并重启。

### 尚未完成

- 实物观察 LINK 连接后 30 秒熄灯，并确认断开/重连会重新计时；
- 切换五个 Profile，确认 Windows/手机蓝牙列表显示 `GALPANEL 1`～`GALPANEL 5`；
- 验证名称切换时不会误清除已有 bond。

### 关联提交

待提交：`feat: add profile names and power-saving LINK indicator`

---

## 2026-09-01 - 增加状态灯与 SYS 安全行为测试固件

### 本次完成

- 新增 `galpanel-status-test` 构建目标，用于验证 LINK、INFO、FN、AUX 状态灯；
- 新增 `galpanel-safety-test` 构建目标，用于验证 SYS 单击、双击、长按边界和 WARN 反馈；
- 新增 GALPANEL Zephyr 状态模块：轮询实际 BLE/Profile/层/endpoint 状态，并监听 Profile 与 layer 事件；
- 状态测试行为：LINK 未连接慢闪、已连接常亮；INFO 按 Profile 闪烁；FN 随第 1 层状态；AUX 随 USB endpoint；
- 安全测试行为：SYS 300 ms 内双击切换输出，单击切换 Profile，按住约 6 秒清除当前 Profile bond；
- 安全测试中，短于 6 秒但超过短按窗口的中断长按不会被误判为单击；
- 更新 `测试计划.md`，增加 T9 状态灯和 T10 安全行为验收步骤。

### 为什么这么做

最终固件的状态灯和 SYS 长按功能都依赖系统事件，不能只靠 LED_TEST 或 keymap 静态检查确认。单独测试固件可以把状态逻辑、定时器和破坏性操作从正式固件中隔离，先验证边界再合并到产品版本。

### 对后续的好处

- 可以先确认 LED 状态是否与真实 endpoint/Profile/layer 一致；
- 可以在不改正式固件的情况下验证 SYS 手势，避免误清除日常配对；
- T9/T10 通过后，正式版本只需复用已验证的状态模块和参数；
- 测试结果可以直接对应《使用说明》的最终交互定义。

### 本次验证

- `validate-project.ps1` 通过；
- `git diff --check` 通过；
- 已核对固定 ZMK 版本提供的 `zmk_ble_active_profile_is_connected()`、`zmk_ble_active_profile_index()`、`zmk_endpoint_get_selected()`、`zmk_keymap_layer_active()` 和 `zmk_ble_clear_bonds()` API；
- GitHub Actions Run #46（Run ID `33480668625`）的正式固件、原有诊断固件和两个新测试固件均构建成功；
- 已下载并烧录 `galpanel-status-test.uf2`，长度 `411136` bytes，SHA-256 为 `DDEFFA12807BC0A93EB38AE72B07B54E5FF89AC5F03AEAE62A3282D3530EC177`；
- 烧录前确认 `E:` 卷标为 `NICENANO`，复制后盘符自动消失。

### 尚未完成

- 实物验证 `galpanel-status-test.uf2` 的 LED 组合；
- 烧录 `galpanel-safety-test.uf2` 并验证 SYS 长按清除当前 Profile；
- 将通过验证的状态模块合并进正式 `galpanel` 固件。

### 关联提交

待提交：`test: add status and safety diagnostic firmware`

---

## 2026-09-01 - 按实物位置统一侧键命名

### 本次完成

- 按用户确认统一四颗侧键的物理位置与引脚：左下 FN=`D14`、左上=`D18`、右上=`D5`、右下=`D7`；
- 更新《使用说明》中的基础层/FN 层表格，改用“左上/左下/右上/右下侧键”作为主标识；
- 修正《项目认知》中的 GPIO 位置和固定侧键功能表；
- 同步更新学习日志、设备树注释和 IO/EC11 测试固件注释；
- 功能绑定未改变，仅修正物理位置说明：D5=QSAVE、D7=Win+D、D14=FN、D18=QLOAD。

### 为什么这么做

用户实际操作时按的是 PCB 上的空间位置，而不是 D 编号。此前文档把 D7 和 D14 的左右位置写反，会导致说明、测试记录和实物操作出现歧义，虽然软件引脚绑定本身是正确的。

### 对后续的好处

- 使用说明、测试固件和原理图可以用同一套位置语言；
- 后续状态灯和安全操作指导不会把 FN 与桌面宏的位置写错；
- 维修、焊接和复测时可同时用“位置 + D 引脚”快速定位。

### 本次验证

- 对照当前 keymap 确认 D5/D7/D14/D18 的功能绑定未被修改；
- 项目结构检查通过，未修改用户未提交的测试计划、DXF 和图片文件。

### 关联提交

待提交：`docs: correct side-key physical positions`

---

## 2026-09-01 - 编写最终功能使用说明审核稿

### 本次完成

- 新增 `使用说明.md`，统一记录最终产品的按键、FN 层、EC11、USB/BLE、5 个 Profile、电源、刷写、恢复和故障判断；
- 明确 AUX 只表示实际 HID 输出端点：USB 亮，BLE/无端点灭；
- 设计 LINK、INFO、FN、WARN 四颗状态灯的最终语义，并明确其与 AUX 的区别；
- 将已经实测的功能与仍待编码/验收的功能分开，避免把设计目标误认为已完成；
- 在说明末尾列出 INFO 闪烁、LINK 慢闪、SYS 长按清除配对和 FN 自动退出等需要审核的参数；
- README 增加使用说明入口。

### 为什么这么做

当前项目已经有硬件认知、开发规划、测试计划和工作日志，但缺少一份从使用者角度描述“最终成品应该怎么操作、灯应该表示什么”的统一文件。先冻结行为定义，再实现剩余状态灯和安全功能，可以避免代码、测试和用户预期再次分叉。

### 对后续的好处

- LINK（蓝牙链路）、INFO（Profile/提示）、FN（层状态）和 AUX（当前输出端点）各自只表达一个概念；
- USB 供电、USB HID 输出和 BLE 连接不再混为一谈；
- 后续每个固件改动都可以对照本文的验收顺序；
- 用户可以先审核最终交互，再决定是否实现 SYS 长按清除配对和 FN 自动退出等有风险或有争议的行为。

### 本次验证

- 对照当前正式 keymap、overlay、项目规划和测试计划核对按键、旋钮和 AUX 定义；
- 确认文档没有把尚未完成的 LINK/INFO/FN/WARN 状态模块写成已烧录功能；
- README、文档和日志改动通过 `git diff --check`。

### 尚未完成

- 用户审核第 9 节参数；
- 实现并烧录 LINK、INFO、FN、WARN 状态模块；
- 完成电池、充电、功耗和装壳后的 RF 验收。

### 关联提交

待提交：`docs: add final GALPANEL usage guide`

---

## 2026-08-31 - AUX 蓝灯显示当前 USB/BLE 输出端点

### 本次完成

- 将 AUX 蓝灯定义为正式固件的输出端点指示灯：亮表示当前 HID 报告实际发送到 USB，灭表示当前端点为 BLE 或尚无可用端点；
- 新增 GALPANEL Zephyr/ZMK 扩展模块，并监听原生 `zmk_endpoint_changed` 事件；
- 上电后延迟读取一次 `zmk_endpoint_get_selected()`，避免只依赖后续切换事件而漏掉初始状态；
- 保留现有 SYS 双击 `OUT_TOG`，切换 USB/BLE 后 AUX 会跟随实际端点变化；
- 更新正式 shield 配置、LED 注释和 README；
- GitHub Actions Run #43（Run ID `33406845357`）全部六个目标构建成功；
- 下载并烧录新的正式 `galpanel.uf2`，长度 `412672` bytes，SHA-256 为 `B8B437328FB7E8D6858F10F3DA48C65409650FCB99A9F0F2CB46181403844C17`。

### 为什么这么做

只检测 USB 供电会把“插电脑充电”和“按键实际走 USB”混为一谈。ZMK 已经维护了当前选择的 HID endpoint，因此直接显示 endpoint 才能回答用户真正关心的问题：按键现在发送给 USB 主机还是蓝牙主机。

### 对后续的好处

- 同时连接 USB 和 BLE 时，可以直接确认当前输入流向；
- 插普通充电器而没有 USB HID 数据连接时，不会误显示为 USB 模式；
- 插电脑充电导致 ZMK 自动选择 USB 时，AUX 会如实点亮，用户可以用 SYS 双击切回 BLE并看到灯熄灭；
- 后续 LINK、INFO、FN、WARN 可以继续复用同一事件监听模块，不必把状态逻辑塞进 keymap。

### 本次验证

- 对照固定版本 ZMK 源码确认 `zmk_endpoint_changed`、`zmk_endpoint_get_selected()` 和 `ZMK_TRANSPORT_USB/BLE` 接口存在；
- 项目静态检查与 `git diff --check` 通过；
- GitHub Actions Run #43 的正式固件和五个诊断固件均构建成功；
- 烧录前确认 `E:` 卷标为 `NICENANO`、Board-ID 为 `nRF52840-nicenano`；
- UF2 复制后盘符自动消失，Windows 重新枚举出 GALPANEL USB HID 与既有 BLE 设备。

### 尚未完成

- 烧录后验证：USB 亮、BLE 灭、SYS 双击时随实际端点切换；
- LINK 绿灯连接状态、INFO Profile 闪烁、FN 层和 WARN 状态。

### 关联提交

- `a97d740 feat: indicate active endpoint with AUX LED`
- `3abd577 build: expose GALPANEL Zephyr module`

---

## 2026-08-31 - 清除 BLE bond 后刷回正式固件

### 本次完成

- 用户已使用 BLE 恢复固件完成设备端 bond 清除，并在 Windows 删除旧设备后重新配对；
- 将正式 `galpanel.uf2` 刷回开发板；
- 正式 UF2 长度 `412160` bytes，SHA-256 为 `2688FCFDB712894BCCBD28C2278D3F5887CEFF28BFEFB3B215DF7B9C951FC404`；
- `E:` / `NICENANO` 复制完成后自动消失，确认设备已跳转正式应用固件。

### 为什么这么做

BLE 恢复固件只负责选择 Profile 和清除旧 bond，不能作为产品固件长期使用。清除设备端和 Windows 端旧配对后，必须刷回正式镜像，才能验证产品键位、EC11、Fn、输出切换和 BLE 自动重连。

### 对后续的好处

- 正式固件现在使用全新的 BLE 配对关系，排除旧 Profile/bond 缓存影响；
- 可以直接验证断 USB 后的无线连接和实际 GALGAME 键位；
- 后续 LED 状态模块可以在干净的 BLE 连接事件上实现，不受旧配对状态干扰。

### 本次验证

- 用户确认 bond 清除和重新配对完成；
- 正式 UF2 烧录复制成功；
- Bootloader 盘符自动消失。

### 尚未完成

- 正式固件断 USB 后 BLE 自动重连复测；
- 确认 SYS 单击 `BT_NXT` 和双击 `OUT_TOG`；
- 实现并测试 Profile/连接 LED 状态指示。

### 关联提交

待提交：`test: restore formal firmware after BLE recovery`

---

## 2026-08-31 - 烧录 BLE Profile 恢复固件

### 本次完成

- GitHub Actions Run #30（Run ID `33398675515`）五个原有目标及新增 BLE 恢复目标构建成功；
- 下载 `galpanel-ble-recovery.uf2`，长度 `392192` bytes，SHA-256 为 `213EF8F489BFFC9FA8A15D43D46C30444BF36ED64F2D454E55ADDF9815B475F5`；
- 烧录到 `E:` / `NICENANO`，复制完成后盘符自动消失，确认设备已启动恢复固件。

### 为什么这么做

正式固件的 SYS 键只能轮换 Profile，且 LED 尚未显示 Profile 状态；Windows 当前保存的 GALPANEL bond 无法确认对应哪个 Profile。恢复固件提供直接选择 Profile 和清除 bond 的按键，能够在 BLE 未连接时通过 USB 完成恢复。

### 对后续的好处

- 可以先清除设备端全部旧 bond，再删除 Windows 旧设备并建立干净配对；
- 可以单独验证 5 个 Profile 的选择行为；
- 恢复流程完成后，再实现正式 LED 状态显示，避免把 LED 缺口与配对故障混为一谈。

### 本次验证

- 恢复固件构建成功；
- UF2 复制成功；
- Bootloader 盘符自动消失。

### 尚未完成

- 按 D8（SYS）清除全部 bond；
- Windows 删除旧 `GALPANEL` 设备；
- 刷回正式 `galpanel.uf2` 并重新配对；
- 实现 Profile/连接 LED 指示。

### 关联提交

`25f5518`（恢复固件）

---

## 2026-08-31 - 增加 BLE Profile 恢复固件

### 本次完成

- 检查正式 keymap，确认 SYS 单击为 `BT_NXT`、双击为 `OUT_TOG`；
- 确认五颗 GALPANEL LED 当前只有 `gpio-leds` 声明，没有 Profile/连接状态事件监听，因此切换 Profile 不会亮灯；
- 新增 `galpanel_ble_recovery` 构建目标；
- 恢复固件提供 Profile 0～4 直接选择、当前 Profile 清除、全部 bond 清除和下一个 Profile 操作；
- 恢复固件禁用 EC11、鼠标和睡眠，降低恢复阶段变量。

### 为什么这么做

Windows 保留了 `GALPANEL` 设备条目，但始终提示无法连接。仅通过 `BT_NXT` 轮询无法确认当前 Profile，也无法清理设备端旧 bond。先用 USB 烧录恢复镜像，可以在没有 BLE 连接时完成 Profile 选择和配对清除；LED 状态显示属于下一步独立功能，不与恢复动作混在一起。

### 对后续的好处

- 可以明确区分“广播/Profile 选择问题”和“电脑配对缓存问题”；
- 通过 `BT_CLR_ALL` 清除设备端全部旧 bond 后，可建立干净的正式配对；
- 后续 LED 状态模块可以基于确认过的 BLE 事件实现真实 Profile 指示，而不是用假设替代状态。

### 本次验证

- 正式 keymap 与 ZMK Bluetooth behavior 源码已核对；
- `BT_CLR_ALL` 确认为清除全部 Profile bond 的原生行为；
- 恢复固件代码和构建目标已加入仓库，等待云端构建。

### 尚未完成

- 云端构建恢复固件；
- 烧录恢复固件并清除设备端 bond；
- Windows 删除旧 `GALPANEL` 条目并重新配对；
- 实现正式固件的 Profile/连接 LED 指示。

### 计划提交

```text
feat: add BLE profile recovery firmware
```

---

## 2026-08-31 - 电池供电下 BLE 配对记录存在但连接失败

### 本次完成

- 用户在 Windows 蓝牙面板看到 `GALPANEL`，但手动连接提示“无法连接，请尝试重新连接”；
- 检查 Windows PnP 状态，确认系统仍保留 GALPANEL 的 BLE HID 服务和鼠标 HID 集合；
- 检查当前 ZMK endpoint 源码，确认 USB 偏好在 USB 不可用时不会永久禁止 BLE，BLE 已连接后可作为回退端点；
- 将首要排查方向调整为活动 BLE Profile 与 Windows 保存的 bond 不一致。

### 为什么这么做

Windows 蓝牙面板中存在设备条目，只能证明电脑保存过配对记录；不能证明设备当前使用同一个 ZMK Profile。GALPANEL 的 SYS 单击会执行 `BT_NXT`，五个 Profile 中只有原配对 Profile 能使用旧 bond 自动连接。BLE 键盘通常由设备主动重连，手动点击 Windows 的“连接”并不是可靠测试方式。

### 对后续的好处

- 先通过无破坏的 Profile 轮询恢复连接，不立即清除 Windows 和设备端 bond；
- 若五个 Profile 全部失败，可明确进入 BLE bond 双端重置流程；
- 后续 LINK 绿灯/Profile 闪烁模块将避免这种“当前 Profile 不可见”的问题。

### 本次验证

- Windows 中仍存在 GALPANEL 对应的 BLE HID/鼠标 HID 设备集合；
- 正式固件含 5 个 BLE Profile，SYS 单击绑定为 `BT_NXT`；
- ZMK endpoint 源码确认未连接 USB 时允许 BLE 作为有效传输端点。

### 尚未完成

- 用户轮询五个 BLE Profile并测试自动重连；
- 若全部失败，删除 Windows 配对并使用 BLE recovery/settings reset 清除设备端 bond；
- 重新配对后验证断电重连。

### 关联提交

待提交：`docs: diagnose BLE profile reconnect failure`

---

## 2026-08-31 - 说明 3.7 V 供电与板载蓝灯状态

### 本次完成

- 根据用户确认，`BAT+`、`B+`、`RAW`、`VCC` 和 `GND` 均无压降且电压正常；
- 确认 `3.7 V` 是单节锂电池的标称电压，不是过低的供电值；
- 区分板载蓝灯与 BLE 状态：USB/充电路径存在时蓝灯常亮，不能直接解释为 BLE 连接状态；
- 记录 USB-C 与可调电源同时连接时的电源路径风险，后续电池测试改为单独供电。

### 为什么这么做

SuperMini 的 `B+/RAW` 电源路径面向单节锂电池，3.7 V 标称值对应约 4.2 V 满电。板载指示灯由开发板电源/USB/充电电路控制，不是 GALPANEL 的 LINK 绿灯，也没有被当前 ZMK 正式固件绑定成 BLE 状态输出。USB 和外部电源同时连接时，蓝灯常亮更可能表示 USB/充电电源状态。

### 对后续的好处

- 不会把正常的电源指示灯常亮误判成 BLE 失败；
- 避免台式电源与 USB 充电器同时驱动同一电池电源节点；
- BLE 测试可以用“断 USB、保持 3.7 V 电池模拟供电、电脑确认 HID 连接”作为唯一判据。

### 本次验证

- 用户确认供电节点不存在压降；
- 用户观察到 USB-C 与可调电源同时连接时板载蓝灯常亮；
- 正式固件 BLE 功能此前已完成配对和无线按键验证。

### 尚未完成

- 断开 USB，仅保留 3.7 V 接 `B+/B-`，复测 BLE 广播和自动重连；
- 后续实现 GALPANEL LINK 绿灯的真实 BLE/输出状态驱动。

### 关联提交

待提交：`docs: explain battery voltage and onboard LED`

---

## 2026-08-31 - 确认 SuperMini RAW 与 B+ 的关系

### 本次完成

- 根据用户疑问复核 SuperMini/ProMicro nRF52840 的公开板级原理图和 PCB 网络；
- 确认该型号模块的 `B+` 与 `RAW` 两个相邻排针焊盘都连接到同一个 `VBAT` 网络；
- 确认 GALPANEL V2.1 只连接 `B+`、不连接 `RAW` 是允许的，不需要在载板上额外串联或短接；
- 将断电测量 `B+ ↔ RAW` 近似 0 Ω作为实物板版本确认方法。

### 为什么这么做

传统 5 V Pro Micro 上的 `RAW` 常表示稳压前输入，但这块 nRF52840 替代板的两个顶部正极焊盘在模块内部共用 `VBAT`。仅根据传统 Pro Micro 命名推断并外部短接，容易错误理解电源路径；应以该模块实际网络和连续性测量为准。

### 对后续的好处

- 排除“RAW 未连接导致模块不上电”的错误方向；
- 保持 GALPANEL 电源设计简单，只使用一个 `B+` 输入；
- 若实物 `B+` 焊点不良，可以通过比较 `B+`、`RAW` 电压快速定位模块排针焊接问题。

### 本次验证

- 公开板级 PCB 网络中，两个相邻电源焊盘均为 `VBAT`；
- GALPANEL V2.1 原理图当前 `B+` 已连接，`RAW` 标记为不连接。

### 尚未完成

- 用户断电确认实物 `B+ ↔ RAW` 连通；
- 带电测量模块侧 `B+/RAW ↔ B-/GND` 是否均为 3.7 V；
- 如果正极、3.3 V 均正常，再检查 RST 与实际供电电流。

### 关联提交

待提交：`docs: clarify RAW and B+ power net`

---

## 2026-08-31 - 排除负极断路，转查 BAT+ 到 B+ 的开关路径

### 本次完成

- 根据用户确认，`B- ↔ GND` 已实测导通，排除“电源负极未接地”作为主要故障原因；
- 复核 V2.1 原理图，确认外部电源正极路径为 `BAT+ → R4(0 Ω) → SW1 → B+ → SuperMini B+`；
- 确认 `SW1` 是电池正极的电源开关，`BAT+` 有电压不等于开发板模块侧 `B+` 一定有电压；
- 将下一步排查点收敛到 SW1 状态、R4、焊点、走线以及 SuperMini 实际 B+ 引脚。

### 为什么这么做

如果开关断开或 R4/走线不通，电源可以在 `BAT+` 测得 3.7 V，但模块的 `B+` 仍然没有带载供电。此时在 `VCC` 测到 3.3 V 也可能是高阻万用表读到的残余/浮地电压，不能证明 nRF52840 已经正常运行。

### 对后续的好处

- 把故障从“地回路问题”进一步缩小到“电池正极开关路径”；
- 通过带电和断电两组测量，可以明确区分开关断路、串联电阻开路和开发板内部故障；
- 固件无需修改，避免用软件变化掩盖硬件供电问题。

### 本次验证

- [V2.1 原理图](PCB/V2.1/SCH_Schematic1_2_2026-07-26.pdf) 显示 `B-` 接 `GND`，`BAT+` 经 `R4` 和 `SW1` 进入 `B+`；
- 用户确认 `B-` 与 `GND` 导通。

### 尚未完成

- 断电测量 `BAT+ ↔ B+` 在 SW1 开/关两状态的通断；
- 断 USB 后带电测量 SuperMini `B+ ↔ B-` 是否为 3.7 V；
- 确认 `RST` 未被拉低、可调电源未进入限流。

### 关联提交

待提交：`docs: isolate battery positive switch path`

---

## 2026-08-31 - 定位外部电源负极未接地风险

### 本次完成

- 根据用户测量结果 `BAT+ ≈ 3.7 V`、`VCC ≈ 3.3 V` 但开发板不上电，复核 V2.1 原理图；
- 确认 V2.1 电池接口的负极应回到系统 `GND`，正极进入 `B+`；
- 确认如果外部电源负极接到未与 `GND` 相连的独立 `BAT-` 焊盘，整个电源回路会悬空，`VCC` 读数不能证明 nRF52840 得到有效供电；
- 暂不修改固件，先排除硬件电源回路问题。

### 为什么这么做

稳压器输出存在不代表负载供电闭合。万用表高阻输入可能通过稳压器、保护器件或其他信号路径读到 3.3 V，但没有可靠的回流路径时，芯片不能正常启动，板载灯也可能只短亮后熄灭。

### 对后续的好处

- 将“蓝牙/固件故障”与“电源回路断路”明确分开；
- 后续电池测试可以用连续性和带载电压两项证据判断，而不是只看某个电压测点；
- 保留正式固件不变，避免硬件问题被软件改动掩盖。

### 本次验证

- 已复核 [V2.1 原理图](PCB/V2.1/SCH_Schematic1_2_2026-07-26.pdf)：SuperMini 的 `B-` 与 `GND` 同一电源网络，外部电池接口负极使用 `GND`；
- 已提出断电测量 `BAT-/B- ↔ GND` 连通性的检查方法。

### 尚未完成

- 用户确认 `BAT-/B-` 到 GALPANEL `GND` 是否近似 0 Ω；
- 重新接线后用 3.7 V、100～200 mA 限流进行带载上电；
- 断 USB 验证 BLE 自动重连。

### 关联提交

待提交：`docs: diagnose battery negative return path`

---

## 2026-08-31 - 电池电源与板载指示灯现象确认

### 本次完成

- 根据实物测试确认：可调电源设置为约 `3.7 V` 时，开发板可作为无线设备供电；
- 确认将电压降到 `1.67 V` 后红灯短暂变亮并熄灭，属于欠压下的掉电/复位现象；
- 区分 SuperMini 板载红/蓝灯与 GALPANEL 的 INFO/LINK/FN/WARN/AUX 五颗灯；
- 确认当前正式固件尚未实现 BLE/USB/层状态灯事件驱动，因此 LINK 绿灯不会自动按蓝牙状态闪烁。

### 为什么这么做

蓝牙是否工作应通过电脑是否保持 BLE HID 连接和按键是否能输入判断，不能仅凭板载蓝灯判断。官方板卡资料将外接电池接口定义为 3.7 V 锂电池接口；将供电压降至 1.67 V 已低于 nRF52840 正常运行所需电压范围，继续测试可能导致反复复位和异常功耗。

### 对后续的好处

- 避免把欠压复位误判为 BLE 故障；
- 电池测试时可以使用正确的 B+/B- 供电路径，并保持足够电压余量；
- 后续实现 LED 状态模块时，LINK 绿灯才会真正承担 BLE/输出状态提示，不与板载电源灯混淆。

### 本次验证

- 正式固件已经包含 BLE 功能，之前的配对和无线输入验证通过；
- 3.7 V 供电观察到板载灯状态变化；
- 1.67 V 供电观察到红灯短亮后熄灭。

### 尚未完成

- 使用 3.7 V、正确 B+/B- 极性和限流设置进行断 USB 的 BLE 重连测试；
- 编写正式 LED 状态事件驱动，让 LINK/FN/INFO/WARN/AUX 显示实际状态。

### 关联提交

待提交：`docs: record battery power LED behavior`

---

## 2026-08-31 - 烧录正式固件并进入整板验收

### 本次完成

- EC11 滚轮诊断固件测试通过：顺时针下滚、逆时针上滚，各刻度均有响应；
- 使用 Run #29 构建产物 `galpanel.uf2` 烧录正式产品固件；
- 正式固件 SHA-256：`2688FCFDB712894BCCBD28C2278D3F5887CEFF28BFEFB3B215DF7B9C951FC404`，长度 `412160` bytes；
- 检测到 `E:` / `NICENANO` Bootloader，复制完成后盘符自动消失，确认已启动应用固件。

### 为什么这么做

诊断固件已经验证普通输入、EC11 A/B 相位、按压和鼠标滚轮 HID 链路。正式固件包含实际 GALGAME 键位、Fn 层、SYS/BLE 行为和正式 EC11 双层映射，必须在诊断通过后切换，避免把测试数字键位误当成产品功能。

### 对后续的好处

- 设备回到实际使用键位，不再输出诊断数字；
- 可以在真实工作流中验收 Ctrl/A/Enter、侧键宏、Fn、BLE Profile 和滚轮/音量；
- 后续问题可以明确归类为产品行为问题，而不是底层 GPIO 或 HID 诊断问题。

### 本次验证

- EC11_TEST：用户确认各项功能均正常；
- 正式 UF2 复制成功；
- Bootloader 盘符自动卸载。

### 尚未完成

- 正式固件整板回归：基础层键位、Fn 层、SYS/BLE、LED 状态和 BLE 重连；
- 记录长时间连续旋转、快速按键和多 Profile 验证结果。

### 关联提交

待提交：`test: flash formal GALPANEL firmware`

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
