// SPDX-License-Identifier: GPL-2.0+
/*
 * FB driver for the SSD1325 OLED Controller
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>

#include "fbtft.h"

#define DRVNAME		"fb_ssd1325"

#define WIDTH 128
#define HEIGHT 64
#define GAMMA_NUM   1
#define GAMMA_LEN   15
#define DEFAULT_GAMMA "7 1 1 1 1 2 2 3 3 4 4 5 5 6 6"

/*
 * write_reg() caveat:
 *
 *    This doesn't work because D/C has to be LOW for both values:
 *      write_reg(par, val1, val2);
 *
 *    Do it like this:
 *      write_reg(par, val1);
 *      write_reg(par, val2);
 */

/* Init sequence taken from the Adafruit SSD1306 Arduino library */
static int init_display(struct fbtft_par *par)
{
	par->fbtftops.reset(par);

	gpio_set_value(par->gpio.cs, 0);
	/* Turn the display off. */
	write_reg(par, 0xAE);
	/* Set the oscillator division. (set clock as 135 frames/sec) */
	write_reg(par, 0xb3);
	write_reg(par, 0xF1);
	/* Set the multiplex ratio to 1/64 duty cycle. */
	write_reg(par, 0xa8);
	write_reg(par, 0x3f);
	/* Set the display offset  */
	write_reg(par, 0xA2);
	write_reg(par, 0x4C);
	/* Set the start line to 0. */
	write_reg(par, 0xa1);
	write_reg(par, 0x00);
	/* Set Master Config to DC/DC Converter. */
	write_reg(par, 0xAD);
	write_reg(par, 0x02);
	/* Set segment remap to 0x50: */
	/* 1010000 (COM split, remap bottom-up, horiz. increment, no nibble remap).*/
	write_reg(par, 0xa0);
	write_reg(par, 0x56); // 56 - 180 - 45/50 normal
	/* Set full current range. */
	write_reg(par, 0x86);
	/* Set the gray color palette. */
	/* 0x01, 0x11, 0x22, 0x32, 0x43, 0x54, 0x65, 0x76 */
	write_reg(par, 0x01);
	write_reg(par, 0x11);
	write_reg(par, 0x22);
	write_reg(par, 0x32);
	write_reg(par, 0x43);
	write_reg(par, 0x54);
	write_reg(par, 0x65);
	write_reg(par, 0x76);
	/* Set the contrast to maximum. */
	write_reg(par, 0x81);
	write_reg(par, 0x7f);
	/* Set the row period. */
	write_reg(par, 0xB2);
	write_reg(par, 0x51);
	/* Set the phase length. */
	write_reg(par, 0xB1);
	write_reg(par, 0x55);
	/* Set the precharge comparator to 2. */
	write_reg(par, 0xB4);
	write_reg(par, 0x02);
	/* Enable the precharge comparator. */
	write_reg(par, 0xB0);
	write_reg(par, 0x28);
	/* Set the high voltage level of the COM pin (0x1C = 0.80 * Vref). */
	write_reg(par, 0xBE);
	write_reg(par, 0x55);
	/* Set the low voltage level of the SEG pin. Value may be wrong. */
	write_reg(par, 0xBF);
	write_reg(par, 0x02);
	/* Set the display to non-inverted configuration. */
	write_reg(par, 0xA4);
	/* turn Display on */
	write_reg(par, 0xAF);
	return 0;
}

static uint8_t rgb565_to_g16(u16 pixel)
{
	u16 b = pixel & 0x1f;
	u16 g = (pixel & (0x3f << 5)) >> 5;
	u16 r = (pixel & (0x1f << (5 + 6))) >> (5 + 6);

	pixel = (299 * r + 587 * g + 114 * b) / 195;
	if (pixel > 255)
		pixel = 255;
	return (uint8_t)pixel / 16;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	fbtft_par_dbg(DEBUG_SET_ADDR_WIN, par,
		      "%s(xs=%d, ys=%d, xe=%d, ye=%d)\n", __func__, xs, ys, xe,
		      ye);

	write_reg(par, 0x75);
	write_reg(par, 0x00);
	write_reg(par, 0x3f);
	write_reg(par, 0x15);
	write_reg(par, 0x00);
	write_reg(par, 0x7f);
}

static int blank(struct fbtft_par *par, bool on)
{
	fbtft_par_dbg(DEBUG_BLANK, par, "(%s=%s)\n",
		      __func__, on ? "true" : "false");

	if (on)
		write_reg(par, 0xAE);
	else
		write_reg(par, 0xAF);
	return 0;
}

/*
 * Grayscale Lookup Table
 * GS1 - GS15
 * The "Gamma curve" contains the relative values between the entries
 * in the Lookup table.
 *
 * 0 = Setting of GS1 < Setting of GS2 < Setting of GS3.....<
 * Setting of GS14 < Setting of GS15
 */
static int set_gamma(struct fbtft_par *par, u32 *curves)
{
	int i;

	fbtft_par_dbg(DEBUG_INIT_DISPLAY, par, "%s()\n", __func__);

	for (i = 0; i < GAMMA_LEN; i++) {
		if (i > 0 && curves[i] < 1) {
			dev_err(par->info->device,
				"Illegal value in Grayscale Lookup Table at index %d.\n"
				"Must be greater than 0\n", i);
			return -EINVAL;
		}
		if (curves[i] > 7) {
			dev_err(par->info->device,
				"Illegal value(s) in Grayscale Lookup Table.\n"
				"At index=%d, the accumulated value has exceeded 7\n",
				i);
			return -EINVAL;
		}
	}
	write_reg(par, 0xB8);
	for (i = 0; i < 8; i++)
		write_reg(par, (curves[i] & 0xFF));
	return 0;
}

static int write_vmem(struct fbtft_par *par, size_t offset, size_t len)
{
	u16 *vmem16 = (u16 *)par->info->screen_buffer;
	u8 *buf = par->txbuf.buf;
	u8 n1;
	u8 n2;
	int y, x;
	int ret;

	for (x = 0; x < par->info->var.xres; x++) {
		if (x % 2)
			continue;
		for (y = 0; y < par->info->var.yres; y++) {
			n1 = rgb565_to_g16(vmem16[y * par->info->var.xres + x]);
			n2 = rgb565_to_g16(vmem16
					   [y * par->info->var.xres + x + 1]);
			*buf = (n1 << 4) | n2;
			buf++;
		}
	}

	gpio_set_value(par->gpio.dc, 1);

	/* Write data */
	ret = par->fbtftops.write(par, par->txbuf.buf,
				par->info->var.xres * par->info->var.yres / 2);
	if (ret < 0)
		dev_err(par->info->device,
			"%s: write failed and returned: %d\n", __func__, ret);

	return ret;
}

static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.txbuflen = WIDTH * HEIGHT / 2,
	.gamma_num = GAMMA_NUM,
	.gamma_len = GAMMA_LEN,
	.gamma = DEFAULT_GAMMA,
	.fbtftops = {
		.write_vmem = write_vmem,
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		.blank = blank,
		.set_gamma = set_gamma,
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "solomon,ssd1325", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:ssd1325");
MODULE_ALIAS("platform:ssd1325");

MODULE_DESCRIPTION("SSD1325 OLED Driver");
MODULE_AUTHOR("Alexey Mednyy");
MODULE_LICENSE("GPL");
