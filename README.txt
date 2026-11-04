FIXED ARDUINO PHP LED CONTROLLER - V2
=======================================

The previous ZIP contained a PHP backslash syntax error. This V2 package
corrects that error.

IMPORTANT FILES
---------------
index.php
config.php
test_com_port.php
arduino_led_controller.ino

INSTALL
-------
1. Replace the old project with this folder:

   C:\xampp\htdocs\arduino_php_led

2. Open config.php.

3. Set your actual Arduino COM port:

   define('ARDUINO_PORT', 'COM6');

4. Upload arduino_led_controller.ino.

5. Close Arduino Serial Monitor.

6. Start Apache in XAMPP.

7. Open:

   http://localhost/arduino_php_led/

DIAGNOSTIC
----------
Open:

   http://localhost/arduino_php_led/test_com_port.php

The correct successful result is:

   RESULT: PHP successfully opened COM6.
   The COM port is accessible by PHP.

IMPORTANT
---------
The browser does not directly access COM6.

The path is:

Browser
  -> Apache/PHP
  -> Windows COM6
  -> USB
  -> Arduino
  -> LED

If PHP can open COM6 but the LED still does not respond, check that the
Arduino sketch is uploaded and that Serial Monitor is closed.

WIRING
------
The supplied sketch uses the Arduino Uno built-in LED on pin 13.

For an external LED:

D13 -> 220/330 ohm resistor -> LED long leg (+)
GND ------------------------------ LED short leg (-)

SERIAL SETTINGS
---------------
9600 baud
8 data bits
No parity
1 stop bit
