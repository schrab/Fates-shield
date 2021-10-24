#SSD 1325 Device Tree overlay

Compile

`sudo dtc -W no-unit_address_vs_reg -@ -I dts -O dtb -o /boot/overlays/fates-ssd1325.dtbo /home/we/fates/overlays/fates-shield-ssd1325-overlay.dts`

Edit /boot/config.txt

``` # Screen
dtoverlay=fates-ssd1325
