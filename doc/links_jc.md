- http://digistump.com/wiki/digispark (abajo hay una sección  específica que tienen para emular dispositivos USB! **Using the          Digispark's USB features**)
- Neopixels:        https://learn.adafruit.com/adafruit-neopixel-uberguide/neopixel-strips
- Vídeo de funcionamiento de NeoPixels:        https://www.youtube.com/watch?v=PPVi3bI7_Z4
- Proyectos basados en v-usb:        https://www.obdev.at/products/vusb/projects.html
- Proyecto reciente que usa v-usb sobre Digispark ATtiny-85:        https://hackaday.com/tag/v-usb/
- Código del proyecto V-USB: https://github.com/obdev/v-usb 
- Producto de amazon: (Placa de Desarrollo Digispark Rev.3        Kickstarter ATTiny85 con USB Compatible con Arduino) https://www.amazon.es/dp/B076KVKHH1/ref=cm_sw_em_r_mt_dp_W5PHHJXQMXWPV891EBCW
- z-lib.org





## Web az-delivery



* https://azde.ly/Buch
* https://azde.ly/Blog





## Título



## 16.2. `dict` comprehensions

They are used in a similar way. Here is an example which I found recently:

```python
mcase = {'a': 10, 'b': 34, 'A': 7, 'Z': 3}

mcase_frequency = {
    k.lower(): mcase.get(k.lower(), 0) + mcase.get(k.upper(), 0)
    for k in mcase.keys()
}

# mcase_frequency == {'a': 17, 'z': 3, 'b': 34}
```

In the above example we are combining the values of keys which are same but in different typecase. I personally do not use `dict` comprehensions a lot. You can also quickly switch keys and values of a dictionary: