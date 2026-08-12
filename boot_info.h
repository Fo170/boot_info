#ifdef _boot_info_

// boot_info
// Diagnostic de la cause de boot / réinitialisation.
// Plateformes : ESP8266, ESP32, STM32, SAM (Arduino Due), RP2xxx (Raspberry Pi Pico)
// Copyright (c) Olivier FOURNET - GPL-3.0-only - version 1.0.0
// https://github.com/Fo170/boot_info
// ESP8266 (origine) : https://www.sigmdel.ca/michel/program/esp8266/arduino/watchdogs_en.html?fbclid=

#define BM_WDT_SOFTWARE 0
#define BM_WDT_HARDWARE 1
#define BM_ESP_RESTART 2
#define BM_ESP_RESET 3

#define BOOT_MODE BM_WDT_SOFTWARE

void printVersion() 
{
  Serial.println("------printVersion-------");
  Serial.print("Compiled: ");
  Serial.print( __DATE__ );
  Serial.print(" - ");
  Serial.println( __TIME__ );
  Serial.println("------------------------");
  Serial.println(__FILE__);
}

// BootInfo : fichier source et ligne de l'emplacement d'un arrêt volontaire.
// __FILE__/__LINE__ sont évalués la MACRO BOOT_HALT au point d'appel.
// BOOT_HALT(msg) : imprime l'emplacement (fichier.s:ligne) puis bloque.
// Le watchdog/panique ESP provoque ensuite un reset, et boot_info()
// (appelé au boot suivant) confirme le motif.
#define BOOT_HALT(msg) do { \
  Serial.println("-----------boot_halt-------------"); \
  Serial.print("At: "); Serial.print(__FILE__); Serial.print(":"); Serial.println(__LINE__); \
  if(msg){ Serial.print("Reason: "); Serial.println(msg); } \
  Serial.flush(); \
  while(1) {} \
} while(0)

#if defined(ARDUINO_ARCH_ESP8266)

extern "C" {
#include "user_interface.h"
}

struct bootflags
{
  unsigned char raw_rst_cause : 4;
  unsigned char raw_bootdevice : 4;
  unsigned char raw_bootmode : 4;

  unsigned char rst_normal_boot : 1;
  unsigned char rst_reset_pin : 1;
  unsigned char rst_watchdog : 1;

  unsigned char bootdevice_ram : 1;
  unsigned char bootdevice_flash : 1;
};

struct bootflags bootmode_detect(void) 
{
  int reset_reason, bootmode;
  asm (
    "movi %0, 0x60000600\n\t"
    "movi %1, 0x60000200\n\t"
    "l32i %0, %0, 0x114\n\t"
    "l32i %1, %1, 0x118\n\t"
    : "+r" (reset_reason), "+r" (bootmode)
    :
    : "memory"
  );

  struct bootflags flags;

  flags.raw_rst_cause = (reset_reason & 0xF);
  flags.raw_bootdevice = ((bootmode >> 0x10) & 0x7);
  flags.raw_bootmode = ((bootmode >> 0x1D) & 0x7);

  flags.rst_normal_boot = flags.raw_rst_cause == 0x1;
  flags.rst_reset_pin = flags.raw_rst_cause == 0x2;
  flags.rst_watchdog = flags.raw_rst_cause == 0x4;

  flags.bootdevice_ram = flags.raw_bootdevice == 0x1;
  flags.bootdevice_flash = flags.raw_bootdevice == 0x3;

  return flags;
}

void boot_info()
{
  Serial.println("-----------boot_info-------------");
  rst_info* rinfo = ESP.getResetInfoPtr();
  Serial.printf ( "Reset reason: %d, %s\n" , rinfo->reason, ESP.getResetReason().c_str() ); 
  Serial.printf("rinfo->exccause: %d\n", rinfo->exccause);
  Serial.printf("rinfo->epc1:     %d\n", rinfo->epc1);
  Serial.printf("rinfo->epc2:     %d\n", rinfo->epc2);
  Serial.printf("rinfo->epc3:     %d\n", rinfo->epc3);
  Serial.printf("rinfo->excvaddr: %d\n", rinfo->excvaddr);
  Serial.printf("rinfo->depc:     %d\n", rinfo->depc);

  struct bootflags bflags = bootmode_detect();

  Serial.printf("\nbootflags.raw_rst_cause: %d\n", bflags.raw_rst_cause);
  Serial.printf("bootflags.raw_bootdevice: %d\n", bflags.raw_bootdevice);
  Serial.printf("bootflags.raw_bootmode: %d\n", bflags.raw_bootmode);
  
  Serial.printf("bootflags.rst_normal_boot: %d\n", bflags.rst_normal_boot);
  Serial.printf("bootflags.rst_reset_pin: %d\n", bflags.rst_reset_pin);
  Serial.printf("bootflags.rst_watchdog: %d\n", bflags.rst_watchdog);
  
  Serial.printf("bootflags.bootdevice_ram: %d\n", bflags.bootdevice_ram);
  Serial.printf("bootflags.bootdevice_flash: %d\n", bflags.bootdevice_flash);

  Serial.printf("\n\nrinfo->reason=%d\n\n", ESP.getResetInfoPtr()->reason);

  if(bflags.raw_bootdevice == 1) {
    Serial.println("The sketch has just been uploaded over the serial link to the ESP8266");
    Serial.println("Beware: the device will freeze after it reboots in the following step."); 
    Serial.println("It will be necessary to manually reset the device or to power cycle it");
    Serial.println("and thereafter the ESP8266 will continuously reboot.");
  }
  Serial.println("---------------------------------");
}

#elif defined(ARDUINO_ARCH_ESP32)

#include "esp_system.h"

void boot_info()
{
  Serial.println("-----------boot_info-------------");
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset reason: %d\n", (int)reason);
  switch (reason)
  {
    case ESP_RST_UNKNOWN:   Serial.println("Reset reason can not be determined"); break;
    case ESP_RST_POWERON:   Serial.println("Reset due to power-on event");         break;
    case ESP_RST_EXT:       Serial.println("Reset by external pin (not applicable for ESP32)"); break;
    case ESP_RST_SW:        Serial.println("Software reset via esp_restart");      break;
    case ESP_RST_PANIC:     Serial.println("Software reset due to exception/panic"); break;
    case ESP_RST_INT_WDT:   Serial.println("Reset (software or hardware) due to interrupt watchdog"); break;
    case ESP_RST_TASK_WDT:  Serial.println("Reset due to task watchdog");           break;
    case ESP_RST_WDT:       Serial.println("Reset due to other watchdogs");         break;
    case ESP_RST_DEEPSLEEP: Serial.println("Reset after exiting deep sleep mode");  break;
    case ESP_RST_BROWNOUT:  Serial.println("Brownout reset (software or hardware)"); break;
    case ESP_RST_SDIO:      Serial.println("Reset over SDIO");                      break;
    default:                Serial.println("Reset reason unknown");                  break;
  }
  Serial.println("---------------------------------");
}

#elif defined(ARDUINO_ARCH_STM32)

// Drapeaux communs du registre RCC->CSR (F0/F1/F3/F4/L0/G0/G4/L4/H7...)
#define _EBI_RCC_CSR_LPWRRSTF (1UL << 0)
#define _EBI_RCC_CSR_WWDGRSTF (1UL << 1)
#define _EBI_RCC_CSR_IWDGRSTF (1UL << 2)
#define _EBI_RCC_CSR_SFTRSTF  (1UL << 3)
#define _EBI_RCC_CSR_PORRSTF  (1UL << 4)
#define _EBI_RCC_CSR_PINRSTF  (1UL << 5)
#define _EBI_RCC_CSR_BORRSTF  (1UL << 6)
#define _EBI_RCC_CSR_RMVF     (1UL << 31)

void boot_info()
{
  Serial.println("-----------boot_info-------------");
  uint32_t csr = RCC->CSR;
  Serial.printf("RCC->CSR (raw): 0x%08X\n", (unsigned)csr);
  if(csr & _EBI_RCC_CSR_BORRSTF)  Serial.println("Brownout reset (BORRSTF)");
  if(csr & _EBI_RCC_CSR_PINRSTF)  Serial.println("Reset by NRST pin (PINRSTF)");
  if(csr & _EBI_RCC_CSR_PORRSTF)  Serial.println("Power-on reset (PORRSTF)");
  if(csr & _EBI_RCC_CSR_SFTRSTF)  Serial.println("Software reset (SFTRSTF)");
  if(csr & _EBI_RCC_CSR_IWDGRSTF) Serial.println("Independent watchdog reset (IWDGRSTF)");
  if(csr & _EBI_RCC_CSR_WWDGRSTF) Serial.println("Window watchdog reset (WWDGRSTF)");
  if(csr & _EBI_RCC_CSR_LPWRRSTF) Serial.println("Low-power reset (LPWRRSTF)");
  if((csr & 0x7F) == 0)           Serial.println("No reset flag set");
  RCC->CSR |= _EBI_RCC_CSR_RMVF; // effacement des drapeaux pour les prochains reboots
  Serial.println("---------------------------------");
}

#elif defined(ARDUINO_ARCH_SAM)

// RSTC_SR (Controleur de reset SAM3X) : RSTTYP bits 8-10, URSTS bit 0
#define _EBI_RSTC_SR_RSTTYP_MASK (0x7u << 8)
#define _EBI_RSTC_SR_URSTS        (0x1u << 0)

#define _EBI_RSTC_RSTTYP_GENERAL  (0x0u)
#define _EBI_RSTC_RSTTYP_BACKUP   (0x1u)
#define _EBI_RSTC_RSTTYP_WATCHDOG (0x2u)
#define _EBI_RSTC_RSTTYP_SOFTWARE (0x3u)
#define _EBI_RSTC_RSTTYP_USER     (0x4u)

void boot_info()
{
  Serial.println("-----------boot_info-------------");
  uint32_t sr = RSTC->RSTC_SR;
  unsigned rsttyp = (sr & _EBI_RSTC_SR_RSTTYP_MASK) >> 8;
  Serial.print("RSTC->SR (raw): 0x");
  Serial.println(sr, HEX);
  Serial.print("Reset type: ");
  Serial.println(rsttyp);
  switch(rsttyp)
  {
    case _EBI_RSTC_RSTTYP_GENERAL:  Serial.println("RSTTYP GENERAL = general reset (power-on)"); break;
    case _EBI_RSTC_RSTTYP_BACKUP:   Serial.println("RSTTYP BACKUP = reset after backup (sleep) wake-up"); break;
    case _EBI_RSTC_RSTTYP_WATCHDOG: Serial.println("RSTTYP WATCHDOG = watchdog reset");                    break;
    case _EBI_RSTC_RSTTYP_SOFTWARE: Serial.println("RSTTYP SOFTWARE = software reset");                    break;
    case _EBI_RSTC_RSTTYP_USER:     Serial.println("RSTTYP USER = reset by NRST button");                  break;
    default:                        Serial.println("RSTTYP unknown");
  }
  if(sr & _EBI_RSTC_SR_URSTS) Serial.println("User reset request on NRST detected (URSTS)");
  Serial.println("---------------------------------");
}

#elif defined(ARDUINO_ARCH_RP2040)

// Bloc WATCHDOG RP2040 (adresses fixes) : WATCHDOG_REASON offset 0x18
#define _EBI_RP2040_WATCHDOG_BASE 0x40058000u
#define _EBI_RP2040_REASON_OFFSET 0x18u
#define _EBI_RP2040_REASON_TIMEOUT (0x1u << 0)
#define _EBI_RP2040_REASON_FORCE   (0x1u << 1)

void boot_info()
{
  Serial.println("-----------boot_info-------------");
  volatile uint32_t* reason = (volatile uint32_t*)(_EBI_RP2040_WATCHDOG_BASE + _EBI_RP2040_REASON_OFFSET);
  uint32_t r = *reason;
  Serial.print("WATCHDOG_REASON (raw): 0x");
  Serial.println(r, HEX);
  if(r & _EBI_RP2040_REASON_TIMEOUT) Serial.println("Watchdog reset (timeout)");
  if(r & _EBI_RP2040_REASON_FORCE)   Serial.println("Watchdog reset (force)");
  if((r & (_EBI_RP2040_REASON_TIMEOUT | _EBI_RP2040_REASON_FORCE)) == 0)
    Serial.println("Power-on / normal reset");
  *reason = 0; // écriture 0 = effacement des drapeaux
  Serial.println("---------------------------------");
}

#else
#error "boot_info: unsupported platform (ESP8266, ESP32, STM32, SAM/Due or RP2xxx required)"
#endif

#endif // _boot_info_