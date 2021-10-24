```sudo apt-get update
sudo apt-get dist-upgrade
sudo apt-get -y install libncurses5-dev
sudo apt-get install git bc raspberrypi-kernel-headers
```


```make -j4 -C ~/linux SUBDIRS=drivers/staging/fbtft modules
sudo cp -v ~/linux/drivers/staging/fbtft/*.ko /lib/modules/$(uname -r)/kernel/drivers/staging/fbtft/
sudo modep -a
```
