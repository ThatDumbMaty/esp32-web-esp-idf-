working now!

What you need:

- esp-idf

- esp32

- LED with a resistor (optional, anything you want to control with a button)

- hc-sr04 if you want (careful to not kill your esp since echo output is 5V and esp is running on 3.3V)

now with working index.html using spiffs, will be adding .css and .js soon,
still learning esp32, will be adding more very cool stuff :)

main/

- main.c

- CMakeLists.txt

- wifi_credentials.h (make your own, define WIFI_SSID to name of your network, 
WIFI_PASS to password of chosen network)

spiffs/

- index.html

- index.js

- styling.css

