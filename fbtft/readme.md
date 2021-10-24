Dependencies
```sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get -y install libncurses5-dev
sudo apt-get install git bc raspberrypi-kernel-headers
```
Linux source
```git clone --depth=1 https://github.com/monome/linux
cd ~/linux
```

```cp /usr/src/linux-headers-$(uname -r)/Module.symvers .
cp /usr/src/linux-headers-$(uname -r)/.config .
```

Copy fb_ssd1325.c to `~/linux/drivers/staging/fbtft`

```make menuconfig
```
        Device Drivers  ---> Staging Drivers ---> Support for small TFT LCD display modules  --->
        <M>   SSD1325 driver

```make prepare
make -j4 -C ~/linux SUBDIRS=drivers/staging/fbtft modules
```

```sudo cp -v ~/linux/drivers/staging/fbtft/*.ko /lib/modules/$(uname -r)/kernel/drivers/staging/fbtft/
sudo modep -a
```
