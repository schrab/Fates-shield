# Fates shield DAC board for Raspberry Pi

Monome [Fates](https://github.com/okyeron/fates) with MIDI and onboard microphone in Monome Shield form-factor

![<fates shield>](<hardware/fates-shield-proto.jpg>)

### Specs:

- WM8731 stereo audio codec with headphone driver
- Microphone
- ZJYM270-7PV2.0 SSD1325 128x64 OLED display (NHD-2.7-12864WDW3 alternative [from Aliexpress](https://aliexpress.ru/item/4000265549668.html))
  ![<ssd1325oled>](<hardware/ssd1325.jpg>)
- 3 pushbuttons
- 4 rotary encoders
- 2x 1/8in inputs
- 2x 1/8in outputs
- 1/8in stereo headphone out
- 1/8in UART MIDI in/out


###TODO
  
- [X] Fix upper OLED holes position
- [ ] Add RC filtering for encoders and keys
- [ ] Fix headphone jack hole
- [X] Fix swapped MIDI IN lines
- [X] Fix MIDI OUT buffer lines

 ![<fates shield back>](<hardware/fates-shield-proto2.jpg>)
