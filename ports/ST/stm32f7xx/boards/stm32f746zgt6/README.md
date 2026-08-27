# NUCLEO-F746ZG(STM32F746ZG)CherryUF2 Bootloader

基于 CherryUSB 的 UF2 Bootloader,为 ST NUCLEO-F746ZG(STM32F746ZGT6,LQFP144,
Cortex-M7)提供 "拖拽 UF2 固件文件到 U 盘即可烧录 App" 的能力。

- App 起始地址:**`0x08010000`**(bootloader 占用前 64KB = 4×16KB 扇区)
- UF2 家族 ID:`0x53b1f5d8`(STM32F7,文件 `53B1F5D8.ID`)
- USB:OTG_FS 全速(板上的 **CN13** USB 连接器,PA11/PA12)

---

## 1. 板卡信息

| 项目 | 说明 |
| :--- | :--- |
| MCU | STM32F746ZGT6(**Cortex-M7**,双精度 FPU,主频最高 216MHz) |
| Flash | 1MB:4×16KB + 1×64KB + 7×128KB 变长扇区(与 F4 1MB 布局相同) |
| RAM | SRAM1 **240KB @0x20010000**(AXI 总线)+ SRAM2 16KB @0x2004C000;bootloader 使用 SRAM1 前 128KB,另 DTCM 64KB @0x20000000 未用 |
| 系统时钟 | **216MHz**(HSE 8MHz bypass ÷8 ×432 ÷2),VOS1 + **OverDrive**,FLASH_LATENCY_7 + ART 加速,APB1 ÷4(54MHz)/ APB2 ÷2(108MHz) |
| USB 时钟 | PLLQ ÷9 = **48MHz**(USB 必须精确 48MHz,故不用 HSI) |
| HSE 来源 | 板载 ST-LINK 的 MCO 输出 8MHz(出厂焊桥默认连通,无需外部晶振) |
| 用户 LED | LD1 绿色 = **PB0**,高电平点亮(DFU 模式常亮) |
| 调试口 | 板载 ST-LINK(SWD),兼作供电 |

## 2. 环境准备

1. 工具链:`arm-none-eabi-gcc`(已测试 10.3-2021-q4)。
2. 初始化子模块(仓库根目录):

   ```
   git submodule update --init lib/CherryUSB lib/uf2 sdk/CMSIS_5
   ```

3. 下载 STM32F7 SDK 到 `sdk/ST/stm32f7xx_library/`(`sdk/` 在 .gitignore 中,需自行获取):

   ```
   mkdir -p sdk/ST/stm32f7xx_library
   cd sdk/ST/stm32f7xx_library
   git clone https://github.com/STMicroelectronics/stm32f7xx_hal_driver.git
   git clone https://github.com/STMicroelectronics/cmsis_device_f7.git
   ```

## 3. 构建

```
cd ports/ST/stm32f7xx
make BOARD=stm32f746zgt6 all
make BOARD=stm32f746zgt6 clean
```

产物:`.build/stm32f746zgt6/cherryuf2_stm32f746zgt6.{bin,hex}`(bin 约 16.8KB)。
预编译版本:`bootloader/v2.0/ST/cherryuf2_stm32f746zgt6.bin`。

> 注意:本 port 的 `stm32f7xx_hal_conf.h` 中 `HSE_VALUE` 默认 25MHz,本板通过
> `board.mk` 的 `-DHSE_VALUE=8000000U` 覆盖为 Nucleo MCO 的 8MHz。

## 4. 烧录 Bootloader

**板载 ST-LINK + st-flash(macOS / Linux 实测)**

```
st-info --probe                                        # 应识别出 STM32F74x_F75x, flash 1MB
st-flash erase                                         # 全片擦除(清掉旧 App,防止误跳转)
st-flash write bootloader/v2.0/ST/cherryuf2_stm32f746zgt6.bin 0x08000000
st-flash reset
```

> Windows 下等价操作:STM32CubeProgrammer(SWD)或 DfuSe(BOOT0=1 从系统 DFU 启动)。

## 5. 使用方法

1. 用 USB 线连接板上 **CN13**(目标 MCU 的 OTG_FS 口;仅接 ST-LINK 口不会出现 U 盘)。
2. 进入 DFU 模式:
   - App 区为空(刚擦除/无有效固件)→ 上电自动进入;
   - 已有 App → **双击 NRST 复位键**,500ms 内第二次复位即进入 DFU,LD1 常亮。
3. 电脑出现 `CherryUF2` U 盘,包含 `INFO_UF2.TXT`、`INDEX.HTM`、`53B1F5D8.ID`。
4. 把 App 的 `.uf2` 文件拖入 U 盘,写入完成后板子自动复位并启动新 App。

### App 固件要求

- **链接起始地址 `0x08010000`**(64KB 边界,F7 扇区 4 起始),App 区上限 960KB;
- 启动早期设置 `SCB->VTOR = 0x08010000;`(CubeMX 生成的 SystemInit 已含,
  `VECT_TAB_OFFSET` 设为 `0x10000` 即可);
- App 需自行初始化时钟(Cortex-M7 的 cache/ART 状态在跳转前均为关闭,DTCM 可用)。

## 6. 实现说明(维护者参考)

- **新建 `ports/ST/stm32f7xx` port**:F7 与 F4 的 HAL 结构几乎一致,
  `boards.c` / `board_flash.c`(F7 HAL 保留了 `FLASH_Erase_Sector` 旧 API)/
  `board_usb.c`(OTG_FS + DWC2,含修正后的 FIFO 配置 RX=160/TX0=16/TX1=128)
  / `usb_config.h` 均从 stm32f4xx port 复用;差异在:
  - 编译参数 `-mcpu=cortex-m7 -mfpu=fpv5-d16`(双精度硬 FPU);
  - UF2 家族 ID `0x53b1f5d8`;
  - `port.mk` 增加 `stm32f7xx_hal_pwr_ex.c`(OverDrive API)。
- **App 起址 0x08010000**:bootloader 约 16.8KB,超出 16KB 扇区,采用 64KB 布局
  (同 stm32f4xx/stm32f412zgt6)。
- **时钟**:216MHz 需要 VOS1 + `HAL_PWREx_EnableOverDrive()`(置于 OscConfig 与
  ClockConfig 之间)+ `FLASH_LATENCY_7`,并在时钟切换后使能 flash 预取与 ART
  (`__HAL_FLASH_PREFETCH_BUFFER_ENABLE()` / `__HAL_FLASH_ART_ENABLE()`)。
  PLL:M=8、N=432、P=2、Q=9 → SYSCLK 216MHz + USB 48MHz。
- **RAM 链接在 0x20010000(SRAM1)**:F746 的 SRAM1 是 240KB AXI SRAM;DTCM
  (0x20000000)与向量表/USB 描述符访问无冲突,bootloader 不使用。

## 7. 已知限制

- 依赖 ST-LINK MCO 8MHz(出厂焊桥默认连通);若 MCO 通路被改焊,需改外部晶振
  并调整 `HSE_VALUE`。
- App 区大小上限:1MB − 64KB = 960KB。
- LED 使用 LD1(PB0);若焊桥改为 PA5 方案,需同步改 `board.h`。

## 8. 验证记录

- 2026-08-27:macOS 构建 + **st-flash 上板实测通过**(st-flash 1.8.0,无需
  `--flash` 参数):全片擦除 → 写入校验 OK → 复位后 CN13 枚举出 `CherryUF2` 卷,
  `INFO_UF2.TXT` 显示 `Model: STMicroelectronics STM32F746 / Board-ID: NUCLEO-F746ZG`。
