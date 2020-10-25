# Arduino LED Control
Arduino code for controlling LEDs:  
See working [animation code on a Paramotor Frame](https://youtu.be/QCYgQ3sWGrA) (youtube video)

![Paramotor LED Lights](images/led-ac-demo-code.jpg)
![Paramotor LED Lights Purple](images/led-ac-demo-purple.jpg)
![Paramotor LED Lights Flight](images/led-ac-demo-flight.jpg)
![Halloween Themed Home LED Lights](images/test-led-pumpkin-colors.jpg)

---------------------------------------

# Hardware/Build
* LED Lights 2x: [ws2815](https://www.amazon.com/gp/product/B07SFTK99V/)
* Controller: [Arduino Nano v3 ATmega328P](https://www.amazon.com/gp/product/B0713XK923/)
* Battery: [12V 6000mAh](https://www.amazon.com/dp/B00ME3ZH7C/)
* More gear linked on [my website](https://deniszholob.com/ppg)

## Soldering
LED strips had 4 connections:

| Strip Connection | Arduino Pin |
| ---------------- | ----------- |
| Ground | GND (Shared) |
| 12v | VIN (Shared) |
| Data | D6 (Output) |
| Data Backup | D6 (Output) |

## Simple LED Test
5 Volt Board power: ![5v circuit](images/arduino-led-circuit-5v.jpg)
12 Volt External Power ![12v circuit](images/arduino-led-circuit-12v.jpg)
Test LED (TM1809 chipset) Connection ![LED TM1809](images/test-led-12v.jpg)

## Paramotor Strips
![LED Node Soldering](images/led-node-solder.jpg)
![LED Section](images/led-section.jpg)

## PPG AC Nitro200 Wiring
Global LED array => 270 total  
Purchased 2 rolls, 150 per led roll = 300 total => 30 left over

* 3 sections
  * Left
  * Top
  * Right
* 2 clusters per section
  * Left
  * Right
* 3 Nodes per cluster
  * Front
  * Back
  * Side
* 15 leds per node

Air Conception Paramotor LED Wiring![Air Conception LED Wiring](images/led-ac-wiring.jpg)



# Resources
## Arduino
* https://www.arduino.cc/reference/en/language/structure/pointer-access-operators/dereference/
* https://learn.sparkfun.com/tutorials/data-types-in-arduino/all
* https://www.tutorialspoint.com/cplusplus/cpp_data_structures.htm
* https://www.cprogramming.com/c++11/c++11-lambda-closures.html
* https://stackoverflow.com/questions/28746744/passing-capturing-lambda-as-function-pointer
* https://learn.adafruit.com/multi-tasking-the-arduino-part-1/using-millis-for-timing
* https://forum.arduino.cc/index.php?topic=580012.0
* https://www.arduino.cc/en/Tutorial/StateChangeDetection
* https://forum.arduino.cc/index.php?topic=426632.0
* https://learn.sparkfun.com/tutorials/pull-up-resistors/all
* https://www.arduino.cc/en/tutorial/pushbutton
* http://ediy.com.my/index.php/tutorials/item/95-arduino-function-with-optional-arguments

## FastLED
* https://github.com/FastLED/FastLED
* https://github.com/FastLED/FastLED/wiki/Overview
* https://github.com/FastLED/FastLED/wiki/Controlling-leds
* http://fastled.io/docs/3.1/group___colorutils.html#ga3144bb2bb66aeed33e20f4fdd6cc1a98

## Code Ref
* https://www.youtube.com/results?search_query=FastLED+programming+patterns+tutorial
* https://www.youtube.com/watch?v=EcMKditEVtY
* http://hamburgtech.de/downloads/
* https://github.com/thehookup/Holiday_LEDs_2.0/blob/master/Holiday_LED_2.0_6_Zones.ino
* https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectMeteorRain
* http://yaab-arduino.blogspot.com/p/ala.html

## Color
* https://sighack.com/post/procedural-color-algorithms-color-variations

## Sim
* https://dougalcampbell.github.io/LEDStrip/
* https://github.com/dougalcampbell/LEDStrip
* https://forum.arduino.cc/index.php?topic=534060.0
