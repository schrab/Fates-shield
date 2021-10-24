
`make -j4 -C ~/linux SUBDIRS=drivers/staging/fbtft modules`

`sudo cp -v ~/linux/drivers/staging/fbtft/*.ko /lib/modules/$(uname -r)/kernel/drivers/staging/fbtft/`

`sudo modep -a`

