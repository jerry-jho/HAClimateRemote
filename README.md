# HAClimateRemote

适用于Home Assistant的空调遥控器

## 硬件

理论上任何接有128x32 OLED显示器的ESP32开发板都可以，将src/hardware.h中的I2C pin改成你自己板子上的pin

这里参考使用这款板子 https://www.diymore.cc/collections/esp8266/products/diymore-esp32-0-96-inch-oled-display-wifi-bluetooth-18650-battery-shield-development-board-cp2102-module-for-arduino，并自己做了键盘（键盘的图纸在hw目录）

## 软件

参考 src/auth.private.h.txt 修改
Arduino 打开 src/ACC.ino 烧写，如果有缺的库自行安装

