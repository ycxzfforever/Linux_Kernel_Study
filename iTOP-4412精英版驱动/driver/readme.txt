设备 			源码位置 							设备名
蜂鸣器 			drivers/char/itop4412_buzzer.c 		/dev/buzzer_ctl
LED 驱动 		drivers/char/itop4412_leds.c 		/dev/leds
AD 数模转换 	drivers/char/itop4412_adc.c 		/dev/adc
485 驱动 		drivers/char/max485_ctl.c 			/dev/max485_ctl_pin
GPS 驱动 		drivers/char/gps.c 					/dev/gps
RTC 实时时钟 	drivers/rtc/rtc-s3c.c 				/dev/rtc*
串口 0-3 		drivers/tty/serial/samsung.c 		/dev/ttySAC*
i2c 总线 0-8 	drivers/i2c/busses/i2c-s3c2410.c 	/dev/i2c*
SPI 总线 		drivers/spi/spi_s3c64xx.c
can 驱动 		net/can/af_can.c
触摸屏驱动（TP）drivers/input/touchscreen/ft5x06_ts.c
开机画面（Log）	drivers/video/samsung/iTop-4412.h
显卡驱动 		drivers/video/samsung/s3cfb_wa101s.c
LCD 背光 		drivers/video/backlight/backlight.c
HDMI_HPD 		drivers/media/video/samsung/tvout/s5p_tvout_hpd.c 			/dev/ HPD
HDMI__Audio		drivers/media/video/samsung/tvout/hw_if/mixer.c
SD/eMMC 		drivers/mmc/host/sdhci-s3c.c
U 盘驱动 		drivers/usb/storage/usb-storage.c 							/mnt/udisk*
网卡 			drivers/net/usb/dm9620.c
USB 转串口 		drivers/usb/serial/pl2303.c
USB 摄像头 		drivers/media/video/uvc/*
MIPI-DSI 显卡	drivers/media/video/exynos/mipicsis/mipi-csis.c
声卡驱动 		/sound/* 													/dev/snd/
矩阵键盘 		drivers/input/keyboard/gpio_keys.c
MFC 			drivers/media/video/samsung
JPEG 			drivers/media/video/samsung
USB_蓝牙 		drivers/bluetooth
SDIO_WIFI 		drivers/mtk_wcn_combo/*
USB 鼠标 		drivers/hid /dev/input/mice
USB 键盘 		drivers/hid /dev/input/event