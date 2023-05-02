# Drivers
## AllDevicesDriver
Este driver permite usar toda las funcionalidades del dispositivo. Para ello, previamente, el dispositivo ha tenido que ser flasheado con la plantilla `ALL_DEVICES`.

La lista de comandos que admite el dispositivo es la siguiente:

### Escritura
* setFullColor *R:G:B*, pone el anillo del color especificado
* setLedColor *N_LED:R:G:B*, pone el led especificado dentro del anillo del color dado
* oled *text*, imprime el texto correspondiente en la pantalla
* 7s *hexValue*, imprime en el display 7S el valor envíado
* blinkLed *stop*, empieza a parpadear un led
* buzzer *frequency stop*, pone a vibrar a la frecuencia dada el buzzer

### Lectura
Dependiendo de la macro definida al compilar el driver el driver pedirá una cosa u otra al dispositivo.
Con la opcion LED_MSG devolverá la combinación actual de color del aro:

```
Hello, World! I'm PwnedDevice ;)
```

Con la opción GET_MSG devolverá un string de 32 bytes:

```
R: 213 G: 31 B: 34
```
