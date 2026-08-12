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
  printVersion();  // date/heure de compilation + chemin du fichier
}

void loop()
{
}
```

- `boot_info()` : dumps la cause de la dernière réinitialisation, plateforme par plateforme.
- `printVersion()` : date/heure de compilation (`__DATE__`, `__TIME__`) + chemin du source (`__FILE__`).

Les messages de sortie sont en **anglais** (usage international).

## Diagnostic d'un plantage (où ça a fait planter)

`__FILE__` seul ne donne que le fichier compilé (déjà affiché par `printVersion()`).
Pour connaître l'**emplacement exact** d'un plantage, il faut imprimer `__FILE__`/`__LINE__`
**au moment du crash**, avec la macro `BOOT_HALT` :

```cpp
void loop()
{
  if (quelqueChoseDeGrave) {
    BOOT_HALT("situation anormale détectée");   // affiche fichier:ligne puis bloque
  }
}
```

Séquence :

1. `BOOT_HALT(msg)` affiche `At: <fichier>:<ligne>` puis boucle à l'infini.
   Sur ESP8266/ESP32 le watchdog/panique provoque ensuite une remise à zéro.
2. Au reboot, `boot_info()` (appelée en `setup()`) confirme le motif (`ESP_RST_TASK_WDT`,
   `ESP_RST_PANIC`, « Exception », …).

Pour un **vrai** crash (panique imprévue, pas un `BOOT_HALT`), le firmware PlatformIO est compilé
avec les symboles de debug (`firmware.elf`). Il suffit alors de mapper les adresses affichées par
`boot_info()` (`epc1/epc2/epc3` sur ESP8266, backtrace ESP32) vers fichier:ligne :

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