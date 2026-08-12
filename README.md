# boot_info

Diagnostic de la cause de boot / réinitialisation sur **5 plateformes** via le port série.

Librairie header-only (un seul fichier `.h`), compatible PlatformIO et Arduino IDE.

| Plateforme | Fonctionnalité |
|---|---|
| **ESP8266** | `ESP.getResetInfoPtr()` (`rst_info`) + registres Xtensa (`bootmode_detect()`) |
| **ESP32** | `esp_reset_reason()` + libellé détaillé (`ESP_RST_*`) |
| **STM32** | Registre `RCC->CSR` : flags reset (POR/PIN/BOR/SOFT/IWDG/WWDG/LPWR), effacés via `RMVF` |
| **Arduino Due (SAM3X)** | `RSTC->RSTC_SR` : type de reset (`RSTTYP`) + flag utilisateur (`URSTS`) |
| **RP2xxx (Raspberry Pi Pico)** | Registre `WATCHDOG_REASON` (timeout / force / power-on) |

## Installation

- **PlatformIO** : ajouter dans `platformio.ini` :
  ```ini
  lib_deps = https://github.com/Fo170/boot_info
  ```
- **Arduino IDE** : via le Library Manager, ou copier `boot_info.h` dans le dossier du sketch.

## Utilisation

Le contenu de la librairie n'est actif que si la macro `_boot_info_` est définie **avant** l'`#include`.

```cpp
#define _boot_info_
#include "boot_info.h"

void setup()
{
  Serial.begin(115200);
  delay(1000);
  boot_info();     // cause du boot / reset
  printVersion();  // date/heure de compilation
}

void loop()
{
}
```

- `boot_info()` : dumps la cause de la dernière réinitialisation, plateforme par plateforme.
- `printVersion()` : date/heure de compilation (`__DATE__`, `__TIME__`).

Les messages de sortie sont en **anglais** (usage international).

## Diagnostic d'un plantage (où ça a fait planter)

`__FILE__` ne donne que le fichier compilé, jamais l'emplacement
d'un crash : un vrai plantage ne passe pas par `__FILE__`. La bonne méthode est de **mapper l'adresse
du compteur programme** affichée au crash (`epc1/epc2/epc3` sur ESP8266, backtrace ESP32) vers
fichier:ligne avec l'outil des symboles du firmware PlatformIO (`firmware.elf` compilé avec `-g`) :

```bash
xtensa-lx106-elf-addr2line -pfiaC .pio/build/nodemcuv2/firmware.elf 0x<epc1>   # ESP8266
xtensa-esp32-elf-addr2line  -pfiaC .pio/build/esp32dev/firmware.elf <pc>        # ESP32
arm-none-eabi-addr2line     -pfiaC .pio/build/<env>/firmware.elf <pc>           # STM32 / Due
```

## Exemples PlatformIO

Chaque exemple est un mini-projet autonome (board + `platformio.ini`) :

```bash
pio run -d examples/ESP8266_BootInfo
pio run -d examples/ESP32_BootInfo
pio run -d examples/STM32_BootInfo
pio run -d examples/Due_BootInfo
pio run -d examples/RP2040_BootInfo
```

## Plateformes non supportées

Sur toute autre plateforme, une erreur de compilation est levée (`#error "boot_info: unsupported platform..."`).

## Licence

GPL-3.0-only — © Olivier FOURNET.

Dépôt : <https://github.com/Fo170/boot_info>