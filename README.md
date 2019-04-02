# COMBISTATO :smiley:

Es un circuito electrónico basado en [arduino](https://es.wikipedia.org/wiki/Arduino) para que haga la función de termostato doble accion para controlar frio / calor.
Esta diseñado con un arduino nano con atmega 328.
Su funcionamiento básico es de comandar dos salidas que manejen equipos de frio y calor para mantener la temperatura de lo que se desea controlar. Para ello se puede ajustar:
* Punto de ajuste de trabajo (set point)
* Ajuste de la ventana de accion (histeresis)
* Retardo de reconeccion para frio y calor separados (algunos equipos lo requieren)
* Error: ajuste de calibracion para eliminar el posible error de la medicion.

La presición del aparato depende mayormente del sensensor. En este caso, el fabricante del 18b20 dice que puede tener un error de 0,5 grados centígrados. Otros sensores, se verá.
Tiene la funcion de guardar los valores de ajustes en la memoria eeprom para que sean legibles luego de una desconexión de la alimentación.
Posee la posbilidad de habilitar o deshabilitar el regitro en consola serial.

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

#### Para hacer:

- Conectar un _buzzer_ y agregar el codigo que falta para las alarmas.
- Desarrollar el codigo para poder configurarlo como dos termostatos separados.
