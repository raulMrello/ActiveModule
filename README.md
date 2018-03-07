# ActiveModule

ActiveModule es un componente que define una interfaz por medio de la cual se pueden crear objetos activos con su propia máquina de estados, suscripción y publicación a topics MQLib.
Las clases que hereden el interfaz deberán implementar las funciones virtuales puras que se definen en éste.

- Versión 13 Feb 2018
  
  
## Changelog

*07.03.2018*
>**"Habilito DefaultPutTimeout para evitar dead-locks ocultos en mutex.lock"**
>
- [x] Mejora control de dead-locks en mutex
  

*14.02.2018*
>**"Habilito flag ready una vez procesado Init::EV_ENTRY"**
>
- [x] Compatibilizo para que sea funcional en ambas plataformas.
  

*13.02.2018*
>**"Compatibilidad MBED y ESP-IDF"**
>
- [x] Compatibilizo para que sea funcional en ambas plataformas.
  

