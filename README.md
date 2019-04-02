# COMBISTATO 

Es un circuito electrónico basado en [arduino](https://es.wikipedia.org/wiki/Arduino) para que haga la función de termostato doble accion para controlar frio / calor.
Esta diseñado con un arduino nano con atmega 328.
Tiene la funcion de guardar los valores de ajustes en la memoria eeprom para que sean legibles luego de una desconexión de la alimentación.
## La idea:
La idea es poder realizar con poco dinero, una circuito util, de gran versatilidad y bastante fiable.
Se usara sensor digital de tempertatura [ds18b20](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf), pero puede usarse cualquiera.
En la salida tendrá un modulo de relé doble, con capacidad de 10A. En su idea original, fue diseñado para controla un motocompresor de refrigeración.
## La electrónica:
* Arduino [nano](https://www.arduino.cc/en/Guide/ArduinoNano) con atmega 328
* modulo de relé de 5v con contacto para 10A
* teclado matricial 4x4
* pequeña fuente de alimentación 
* resistores varios
* display [lcd-16x02](https://www.engineersgarage.com/electronic-components/16x2-lcd-module-datasheet)
