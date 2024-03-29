# Tiny-thermomether

![Alt text](case/img1.jpg) 

This project is mostly a recopilation of other projects I've found on various pages.  
I've just joined them at my personal taste, so I'll let the link to those projects below.  


## Parts used

+ ATtiny85
+ SSD1306 128x32 OLED I2C Display
+ DHT11 temperature and humidity sensor
+ DS18B20 water temperature sensor


## Electrical connection

ATtiny85  | PCB                   | Debug
--------- | --------------------- | --------
PB0       | Display SDA           | MOSI
PB1       | Push button           | MISO
PB2       | Display SCL           | CLK
PB3       | DHT11 (pin 2)         |
PB4       | DS18B20 (yellow wire) |
PB5       | N/C                   | RST

The sensors data lines have a 4.7K PullUp resistor.  
The push button have a 10K resistor.  
The I2C bus have a 10K PullUp resistor, both in SDA and SCL.

## Compilation and flashing

I've only tested it with avr-gcc on linux.  
To compile you need `avr-gcc` and `avr-g++`.  
To flash it I've used `avrdude` with an `usbasp-clone` avr programmer from [this other project](https://github.com/feer9/FASTUSBasp) running on an STM32F407VET6.  
Running `make flash` will compile and flash the program, but the first time you can `make install` to flash the program and fuses to match the clock speed.  

## Code included in this project:

https://www.instructables.com/ATTiny85-connects-to-I2C-OLED-display-Great-Things/

https://github.com/Jacajack/avr-ds18b20

https://www.hackster.io/boyanov/tinudht-c-library-for-dht11-temperature-humidity-sensor-c492e9


## Other pages of utility:

https://github.com/lexus2k/ssd1306

https://aprendiendoarduino.wordpress.com/2016/11/14/bus-i2ctwi/

