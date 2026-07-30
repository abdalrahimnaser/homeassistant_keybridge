

# HASS KeyBridge: Puente MQTT del Teclado a Home Assistant
<p align="center">
  <img width="421" height="410" alt="Untitled Diagram drawio" src="https://github.com/user-attachments/assets/68ae9a1e-cb8d-4bdf-9eef-5706d07371ea" />
</p>

Esta guía te ayudará a configurar tu KeyBridge (o tu configuración de devkit esp32 según `/hardware/devkit_setup`) para convertir los comandos de teclado y macropad en mensajes MQTT de Home Assistant y así desencadenar automatizaciones.
Nota: asegúrate de que tu versión de esp32 sea compatible con USB Host (se ha probado y funciona con ESP32S3).

## Licencia
Este proyecto está licenciado solo para uso no comercial. Queda prohibido el uso comercial sin el permiso previo por escrito del titular de los derechos de autor. Consulta el archivo LICENSE para conocer todos los términos.

## ⭐ ¡Muestra tu apoyo!

Si encuentras útil este proyecto, ¡considera darle una estrella! Esto ayuda a aumentar la visibilidad del proyecto y es una gran manera de mostrar tu agradecimiento. ¡Gracias por tu apoyo! 🙏

## Requisitos previos

Antes de comenzar, asegúrate de que la configuración de MQTT de Home Assistant esté correctamente establecida. Para instrucciones detalladas, consulta este tutorial: https://www.youtube.com/watch?v=ImBv2mtaRR0&pp=ygUTaG9tZSBhc3Npc3RhbnQgbXF0dA%3D%3D

## Configuración del firmware

### 1. Cargar el proyecto base
En ESP-IDF, carga el proyecto de ejemplo **HID** ubicado bajo los ejemplos de **Host**.

### 2. Reemplazar los archivos de origen
Reemplaza todos los archivos en el directorio `/main` (en el proyecto de esp idf) con los archivos del directorio `/FW` en este repositorio.

### 3. Configurar la red
Ejecuta el siguiente comando para abrir el menú de configuración:
```bash
idf.py menuconfig
```
Navega a **Example Connection Configuration** y configura:
- El SSID de tu red WiFi
- La contraseña de tu WiFi

### 4. Configurar MQTT
En `firmware_main.c` (línea 56), actualiza los siguientes parámetros:
- **MQTT Broker URL**: Reemplázala con la IP de tu Home Assistant y el puerto de MQTT (mantén el prefijo `mqtt://`)
- **MQTT Username**: Establece el nombre de usuario de tu broker MQTT
- **MQTT Password**: Establece la contraseña de tu broker MQTT

### 5. Compilar y programar
Compila y programa el proyecto en tu ESP32. Puedes hacerlo en un solo paso mediante el ícono de fuego en la barra inferior de VS Code (debes tener la extensión ESP IDF configurada):

## Configuración de automatizaciones en Home Assistant

### Crear automatizaciones activadas por MQTT

1. Navega a **Settings** → **Automations & Scenes** → **Create Automation** → **Create New Automation**

2. **Configurar el activador (sección "When")**:
   - Busca y selecciona la opción **MQTT**
   - Establece el **Topic** en: `/keybridge/key`
   - Establece el **Payload** en el identificador de tecla deseado (por ejemplo, `0` para la tecla cero)

3. **Configurar la acción (sección "Then Do")**:
   - Especifica la acción a realizar cuando se presione la tecla
   - Ejemplo: Crear una notificación persistente para fines de prueba

## Caso de uso de ejemplo

Una automatización básica podría desencadenar una notificación persistente cada vez que se presione la tecla `0` en tu teclado/macropad conectado, demostrando una comunicación exitosa entre tu KeyBridge y Home Assistant.
<p align="center"> <img width="489" height="212" alt="image" src="https://github.com/user-attachments/assets/82b15e7d-d673-4ae5-a9f8-e6eabc46f880" /> </p>

Puedes expandir esta configuración para controlar luces, interruptores, escenas o cualquier otra entidad de Home Assistant en función de diferentes combinaciones de teclas de tu dispositivo de entrada.
