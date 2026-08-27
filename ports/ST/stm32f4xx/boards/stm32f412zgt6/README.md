# NUCLEO-F412ZG(STM32F412ZG)CherryUF2 Bootloader

基于 CherryUSB 的 UF2 Bootloader,为 ST NUCLEO-F412ZG(STM32F412ZGT6,LQFP144)提供
"拖拽 UF2 固件文件到 U 盘即可烧录 App" 的能力。

- App 起始地址:**`0x08010000`**(bootloader 占用前 64KB = 4×16KB 扇区)
- UF2 家族 ID:`0x57755a57`(STM32F4,文件 `57755A57.ID`)
- USB:OTG_FS 全速(板上的 **CN13** USB 连接器,PA11/PA12)

---

## 1. 板卡信息

| 项目 | 说明 |
| :--- | :--- |
| MCU | STM32F412ZGT6(Cortex-M4F,主频最高 100MHz) |
| Flash | 1MB:4×16KB + 1×64KB + 7×128KB 变长扇区 |
| RAM | SRAM1 112K + SRAM2 16K = 128K @0x20000000(bootloader 仅用此区域;另有 64K CCM @0x10000000) |
| 系统时钟 | **96MHz**(HSE 8MHz bypass ÷8 ×192 ÷2),FLASH_LATENCY_3,APB1 ÷4 / APB2 ÷2 |
| USB 时钟 | PLLQ ÷4 = **48MHz**(USB 必须精确 48MHz,故不用 HSI) |
| HSE 来源 | 板载 ST-LINK 的 MCO 输出 8MHz(出厂焊桥默认连通,无需外部晶振) |
| 用户 LED | LD1 绿色 = **PB0**,高电平点亮(DFU 模式常亮) |
| 调试口 | 板载 ST-LINK(SWD),兼作供电 |

## 2. 环境准备

1. 工具链:`arm-none-eabi-gcc`(已测试 10.3-2021-q4)。
2. 初始化子模块(仓库根目录):

   ```
   git submodule update --init lib/CherryUSB lib/uf2 sdk/CMSIS_5
   ```

3. 下载 STM32F4 SDK 到 `sdk/ST/stm32f4xx_library/`(`sdk/` 在 .gitignore 中,需自行获取):

   ```
   mkdir -p sdk/ST/stm32f4xx_library
   cd sdk/ST/stm32f4xx_library
   git clone https://github.com/STMicroelectronics/stm32f4xx_hal_driver.git
   git clone https://github.com/STMicroelectronics/cmsis_device_f4.git
   ```

## 3. 构建

```
cd ports/ST/stm32f4xx
make BOARD=stm32f412zgt6 all
make BOARD=stm32f412zgt6 clean
```

产物:`.build/stm32f412zgt6/cherryuf2_stm32f412zgt6.{bin,hex}`(bin 约 16.4KB)。
预编译版本:`bootloader/v2.0/ST/cherryuf2_stm32f412zgt6.bin`。

> 注意:本 port 的 `stm32f4xx_hal_conf.h` 中 `HSE_VALUE` 默认 25MHz(自定义板晶振),
> 本板通过 `board.mk` 的 `-DHSE_VALUE=8000000U` 覆盖为 Nucleo MCO 的 8MHz。

## 4. 烧录 Bootloader

**板载 ST-LINK + st-flash(macOS / Linux 实测)**

```
st-info --probe                                        # 应识别出 STM32F41x, flash 1MB
st-flash erase                                         # 全片擦除(清掉旧 App,防止误跳转)
st-flash write bootloader/v2.0/ST/cherryuf2_stm32f412zgt6.bin 0x08000000
st-flash reset
```

> Windows 下等价操作:STM32CubeProgrammer(SWD)或 DfuSe(BOOT0=1 从系统 DFU 启动)。

## 5. 使用方法

1. 用 USB 线连接板上 **CN13**(目标 MCU 的 OTG_FS 口;仅接 ST-LINK 口不会出现 U 盘)。
2. 进入 DFU 模式:
   - App 区为空(刚擦除/无有效固件)→ 上电自动进入;
   - 已有 App → **双击 NRST 复位键**,500ms 内第二次复位即进入 DFU,LD1 常亮。
3. 电脑出现 `CherryUF2` U 盘,包含 `INFO_UF2.TXT`、`INDEX.HTM`、`57755A57.ID`。
4. 把 App 的 `.uf2` 文件拖入 U 盘,写入完成后板子自动复位并启动新 App。

### App 固件要求

- **链接起始地址 `0x08010000`**(64KB 边界,F4 扇区 4 起始),App 区上限 960KB;
- 启动早期设置 `SCB->VTOR = 0x08010000;`(CubeMX 生成的 SystemInit 已含,
  `VECT_TAB_OFFSET` 设为 `0x10000` 即可)。

## 6. 实现说明(维护者参考)

- **App 起址为何是 0x08010000**:bootloader 二进制约 16.4KB,超出单个 16KB 扇区
  (sector 0),若沿用其它 F4 板的 0x08004000 会与 App 头部重叠,故采用 F4 经典的
  64KB bootloader 布局(4×16KB 扇区),App 从 64KB 边界(扇区 4)开始。
- **时钟**:USB 需要精确 48MHz,而 F4 的 HSI(±1%)不满足 USB 容差,必须用 HSE;
  Nucleo 的 HSE 来自 ST-LINK MCO 8MHz(`RCC_HSE_BYPASS`),PLL:M=8、N=192、P=2、
  Q=4 → SYSCLK 96MHz + USB 48MHz。
- **USB**:OTG_FS(DWC2 IP),复用 `CherryUSB/port/dwc2`,port 层 `board_usb.c` 已有
  全套支持(OTG_FS_IRQHandler 等)。**FIFO 配置已修正**:原表(RX=256 + TX 24×4
  = 340 words)超出 OTG_FS 硬件 320 words(1.25KB)上限,CherryUSB 驱动检测到溢出会
  `while(1)` 死锁(现象:LED 常亮但无 USB 设备);现改为 RX=160 / TX0=16 / TX1=128,
  共 304 words,且 MSC IN 端点 FIFO(512B)足量。此修复对 F401/F407/F411 同样生效。
- **Flash**:F4 变长扇区表在 `board_flash.c` 中统一处理,1MB 布局与 F407ZG 相同。
- **HAL tick 修复**:CherryUSB DWC2 驱动初始化(`dwc2_set_mode` → `HAL_Delay(50)`)
  依赖 SysTick 计数;本 port 此前未启动 SysTick 会在 USB 初始化时死锁(旧预编译 bin
  构建于 CherryUSB 更新之前故未触发)。已在 `boards.c` 的 `SysTick_Handler` 补
  `HAL_IncTick()`,并在 `board_usb.c` 的 `board_uf2boot_init()` 启动 SysTick,
  对 F401/F407/F411 同样生效(重建后仍在 16KB 内,实测 16.2~16.3KB)。
- **macOS 构建**:`port.mk` 已改用 `$(PORT_DIR)` 拼源文件路径(同 stm32l4xx port 的
  修复,BSD `realpath` 不支持 `--relative-to`)。

## 7. 已知限制

- 依赖 ST-LINK MCO 8MHz(出厂焊桥 SB120 ON 状态);若 MCO 通路被改焊,HSE 无法起振,
  需改用外部晶振并相应调整 `HSE_VALUE`。
- App 区大小上限:1MB − 64KB = 960KB。
- LED 使用 LD1(PB0);若焊桥改为 PA5 方案(SB119),需同步改 `board.h`。

## 8. 验证记录

- 2026-08-27:macOS 构建 + **st-flash 上板实测通过**(st-flash 1.8.0 写 F412 需加
  `--flash=1M` 参数,其 flash 大小自动探测会误报 0KiB / "Unknown memory region"):
  全片擦除 → 写入校验 OK → 复位后 CN13 枚举出 `CherryUF2` 卷,`INFO_UF2.TXT` 显示
  `Model: STMicroelectronics STM32F412 / Board-ID: NUCLEO-F412ZG`。
  排障过程:初次烧录无 U 盘,经 ST-LINK 读 RCC(时钟 96MHz 配置正确)与 OTG_FS
  寄存器(GRXFSIZ 已写 256)定位到 CherryUSB FIFO 溢出死锁,修复后验证通过。
- F401/F407/F411 回归构建通过(16.2~16.3KB,均小于 16KB)。
