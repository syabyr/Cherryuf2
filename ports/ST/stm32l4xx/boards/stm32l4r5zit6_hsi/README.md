# NUCLEO-L4R5ZI(STM32L4R5ZI)CherryUF2 Bootloader

基于 CherryUSB 的 UF2 Bootloader,为 ST NUCLEO-L4R5ZI(STM32L4R5ZIT6,LQFP144)提供
"拖拽 UF2 固件文件到 U 盘即可烧录 App" 的能力。

- App 起始地址:`0x08008000`(bootloader 占用前 32KB)
- UF2 家族 ID:`0x00ff6919`(STM32L4,文件 `00FF6919.ID`)
- USB:OTG_FS 全速(板上的 **CN13** USB 连接器,PA11/PA12)

---

## 1. 板卡信息

| 项目 | 说明 |
| :--- | :--- |
| MCU | STM32L4R5ZIT6(Cortex-M4F,STM32L4+ 系列) |
| Flash | 2MB,双 bank(出厂默认 DBANK=1),页大小 **4KB** |
| RAM | 640KB(SRAM1 192K + SRAM2 64K 连续于 0x20000000;SRAM3 384K @0x10040000) |
| 系统时钟 | 120MHz(HSI16 → PLL ÷1 ×15 ÷2),Range 1 **Boost** 模式,FLASH_LATENCY_4 |
| USB 时钟 | HSI48(48MHz 片内 RC,未做 CRS 校准) |
| 用户 LED | LD1 绿色 = **PB0**,高电平点亮(DFU 模式常亮) |
| 调试口 | 板载 ST-LINK(SWD),兼作供电 |

## 2. 环境准备

1. 工具链:`arm-none-eabi-gcc`(已测试 10.3-2021-q4)。
2. 初始化子模块(仓库根目录):

   ```
   git submodule update --init lib/CherryUSB lib/uf2 sdk/CMSIS_5
   ```

3. 下载 STM32L4 SDK 到 `sdk/ST/stm32l4xx_library/`(`sdk/` 在 .gitignore 中,需自行获取):

   ```
   mkdir -p sdk/ST/stm32l4xx_library
   cd sdk/ST/stm32l4xx_library
   git clone https://github.com/STMicroelectronics/stm32l4xx_hal_driver.git
   git clone https://github.com/STMicroelectronics/cmsis_device_l4.git
   ```

## 3. 构建

```
cd ports/ST/stm32l4xx
make BOARD=stm32l4r5zit6_hsi all        # 也可加 copy-artifact 复制产物
make BOARD=stm32l4r5zit6_hsi clean      # 清理
```

产物位于 `.build/stm32l4r5zit6_hsi/`:

- `cherryuf2_stm32l4r5zit6_hsi.bin` — 用于烧录(约 16.5KB)
- `cherryuf2_stm32l4r5zit6_hsi.hex`

预编译版本:`bootloader/v2.0/ST/cherryuf2_stm32l4r5zit6_hsi.bin`。

## 4. 烧录 Bootloader

任选其一(首次推荐 ST-LINK):

**方式 A:板载 ST-LINK + st-flash(macOS / Linux 实测)**

```
st-info --probe                                       # 应识别出 STM32L4Rx, flash 2MB, pagesize 4096
st-flash erase                                        # 全片擦除(清掉旧 App,防止误跳转)
st-flash write bootloader/v2.0/ST/cherryuf2_stm32l4r5zit6_hsi.bin 0x08000000
st-flash reset
```

> Windows 下等价操作:STM32CubeProgrammer(SWD)或 DfuSe(BOOT0=1 从系统 DFU 启动)。

**方式 B:OpenOCD / 其它 SWD 工具**:把 bin 写到 `0x08000000` 即可。

## 5. 使用方法

1. 用 USB 线连接板上 **CN13**(目标 MCU 的 OTG_FS 口;仅接 ST-LINK 口不会出现 U 盘)。
2. 进入 DFU 模式:
   - App 区为空(刚擦除/无有效固件)→ 上电自动进入;
   - 已有 App → **双击 NRST 复位键**,500ms 内第二次复位即进入 DFU,LD1 常亮。
3. 电脑出现 `CherryUF2` U 盘,包含 `INFO_UF2.TXT`、`INDEX.HTM`、`00FF6919.ID`。
4. 把 App 的 `.uf2` 文件拖入 U 盘,写入完成后板子自动复位并启动新 App。

### App 固件要求

- **链接起始地址 `0x08008000`**(启动文件/链接脚本 FLASH ORIGIN),不要超过 0x08200000;
- 启动早期设置 `SCB->VTOR = 0x08008000;`(CubeMX 生成的 SystemInit 已含);
- App 内 `VECT_TAB_OFFSET` 需设为 `0x8000`,或改 `FLASH_BASE == 0x08008000`。

## 6. 实现说明(维护者参考)

与同 port 的 STM32L433(fsdev USB)相比,L4R5 有三处实质差异:

1. **USB IP 是 Synopsys OTG_FS(DWC2)**,不是 fsdev:
   - `board.mk` 中 `USB_IP = dwc2`,`port.mk` 据此选择
     `CherryUSB/port/dwc2/usb_dc_dwc2.c + usb_glue_st.c`(否则编 fsdev);
   - `board_usb.c` 以 `#if defined(USB_OTG_FS)` 分支:
     `OTG_FS_IRQHandler`、基址 `USB_OTG_FS_PERIPH_BASE`、复用 `GPIO_AF10_OTG_FS`、
     `__HAL_RCC_USB_OTG_FS_CLK_ENABLE()`;
   - DWC2 FIFO 分配回调(RX=160 / TX0=16 / TX1=128 words,合计 304 ≤ 320 words/1.25KB)。
2. **2MB 双 bank Flash、4KB 页**:`board_flash.c` 以 `#ifdef FLASH_BANK_2` 按
   `0x08100000` 边界选择 bank,页号相对 bank 基址计算。
3. **120MHz 需 Range 1 Boost**(`PWR_REGULATOR_VOLTAGE_SCALE1_BOOST`,L4+ 特有)。

衍生问题:

- **HAL tick**:DWC2 驱动初始化经 `usbd_dwc2_delay_ms()` → `HAL_Delay()`,而 bootloader
  不跑 `HAL_Init()`。已在 `boards.c` 的 `SysTick_Handler` 补 `HAL_IncTick()`,并在
  DWC2 路径的 `board_uf2boot_init()` 里启动 SysTick,否则初始化会死锁。
- **macOS 构建**:`ports/make.mk` 的 `realpath --relative-to` 在 BSD realpath 下不可用,
  本 port 的 `port.mk` 已改用 `$(PORT_DIR)` 拼路径(ArteryTek 各 port 未改,macOS 上
  需同样处理)。

## 7. 已知限制

- 需保持 **DBANK=1**(出厂默认,双 bank 4KB 页);若被改为单 bank(DBANK=0,页 8KB),
  页布局不同,不支持,需改回。
- HSI48 未做 CRS 自动校准(与 L433 板一致);如遇个别主机枚举不稳,可在 clock_init
  中增加 CRS(USB SOF 作为同步源)。
- App 区大小上限:2MB − 32KB;UF2 写入跨 bank1/bank2 边界已支持。

## 8. 实测记录

- 2026-08-27(commit `9a9d55f`):macOS + st-flash v1.8.0 实测通过 ——
  `st-info --probe` 识别 `STM32L4Rx / 2MB / pagesize 4096`;全片擦除 + 写入校验 OK;
  复位后 CN13 枚举出 `CherryUF2` 卷,`INFO_UF2.TXT` 显示
  `Model: STMicroelectronics STM32L4R5 / Board-ID: NUCLEO-L4R5ZI`。
