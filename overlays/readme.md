#SSD 1325 Device Tree overlay

SPI frequency chaCancel changesnged to 32MHz

Copy `fates-shield-ssd1325-overlay.dts` to `/home/we/fates/overlays/`

Compile dtb

    sudo dtc -W no-unit_address_vs_reg -@ -I dts -O dtb -o /boot/overlays/fates-ssd1325.dtbo /home/we/fates/overlays/fates-shield-ssd1325-overlay.dts

Edit `/boot/config.txt`

    # Screen
    dtoverlay=fates-ssd1325
