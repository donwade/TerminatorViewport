
#include "Arduino.h"
#define LED_BUILTIN 2

#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>

#include "watchdogs.h"
#include "addin-ota.h"

#include "common.h"

//#include "esp_brownout_detector.h" // Include the brownout detector header

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "soc/soc.h"
//#include "soc/cpu.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_cntl.h"


//-------------------------------------------------------------
//void app_main()
void setup_ota()
{

	int32_t val;

    WiFi.mode(WIFI_STA);
    WiFi.begin(MY_SSID, MY_SSID_PASSWORD);

    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        //Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); // This prints the IP address


  /*normal values.
  TRACE ota_setup:460   Xtal frequency = 40 mHz 								  
  TRACE ota_setup:461   CpuFrequency = 240 mHz
  TRACE ota_setup:462   APB bus frequency = 80 mHz
  */

  // moving clock to CPU clock to 40 
  //  look at XTAL table below, find the XTAL we have (its 40)
  //  find the divider value (its 10 for us)
  //function takes the following frequencies as valid values:
  //  (240 160) = 80  <<< For all XTAL types
  //  ( 40  20) = 10  <<< For 40MHz XTAL
  //  ( 26)     = 13  <<< For 26MHz XTAL
  //  ( 24)     = 12  <<< For 24MHz XTAL


	//setCpuFrequencyMhz(CPU_FREQ);  // 10 thru 40 ... fails on wifi

	Serial.println();
	TRACE ("-------- CPU FREQ %d/240 -------------------\n", CPU_FREQ);
	TRACE ("Xtal frequency = %d mHz (normally 40)\n", getXtalFrequencyMhz());
	TRACE ("CpuFrequency = %d mHz (normally 240)\n", getCpuFrequencyMhz());
	TRACE ("APB bus frequency = %d mHz (normally 80)\n",  getApbFrequency()/1000000);


	// Port defaults to 3232
	ArduinoOTA.setPort(3232);
	ArduinoOTA.setTimeout(3000);
	
	// Hostname defaults to esp3232-[MAC]
	ArduinoOTA.setHostname(REMOTE_HOSTNAME);
	Serial.printf("remote hostname = %s.local\n", REMOTE_HOSTNAME);
	
	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA
	.onStart([]() {
	  String type;
	  if (ArduinoOTA.getCommand() == U_FLASH) {
	    type = "sketch";
	  } else {  // U_SPIFFS
	    type = "filesystem";
	  }

	  // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
	  Serial.println("Start updating " + type);
	})
	.onEnd([]() {
	  Serial.println("\nEnd");
	})
	.onProgress([](unsigned int progress, unsigned int total) {
	  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	})
	.onError([](ota_error_t error) {
	  Serial.printf("Error[%u]: ", error);
	  if (error == OTA_AUTH_ERROR) {
	    Serial.println("Auth Failed");
	  } else if (error == OTA_BEGIN_ERROR) {
	    Serial.println("Begin Failed");
	  } else if (error == OTA_CONNECT_ERROR) {
	    Serial.println("Connect Failed");
	  } else if (error == OTA_RECEIVE_ERROR) {
	    Serial.println("Receive Failed");
	  } else if (error == OTA_END_ERROR) {
	    Serial.println("End Failed");
	  }
	});

	ArduinoOTA.begin();

	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

}

void loop_ota() 
{

#if 0
	// blinky
	static uint32_t ticker;
	static bool val;

	uint32_t diff = millis() - ticker;
	if (diff > 250)
	{
		digitalWrite(LED_BUILTIN,val);
		val = !val;
		ticker = millis();
	}
#endif

	ArduinoOTA.handle();
}

//-------------------------------------------------------------

void reportWakeUpCause()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {

    case ESP_SLEEP_WAKEUP_UNDEFINED:
		Serial.println(	"Wakeup:In case of deep sleep, reset was not:  exit from deep sleep");
	break;
		
    case ESP_SLEEP_WAKEUP_ALL:
		Serial.println("Wakeup:Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source");
	break;
		
    case ESP_SLEEP_WAKEUP_EXT0:
		Serial.println("Wakeup:external signal using RTC_IO");
	break;
		
    case ESP_SLEEP_WAKEUP_EXT1:
		Serial.println("Wakeup:external signal using RTC_CNTL");
	break;
		
    case ESP_SLEEP_WAKEUP_TIMER:
		Serial.println("Wakeup:timer");
	break;
		
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
		Serial.println("Wakeup:touchpad");
	break;
		
    case ESP_SLEEP_WAKEUP_ULP:
		Serial.println("Wakeup:ULP program");
	break;
		
    case ESP_SLEEP_WAKEUP_GPIO:
		Serial.println("Wakeup:GPIO (light sleep only on ESP32, S2 and S3)");
	break;
		
    case ESP_SLEEP_WAKEUP_UART:
		Serial.println("Wakeup:UART (light sleep only)");
	break;
		
    case ESP_SLEEP_WAKEUP_WIFI:
		Serial.println("Wakeup:WIFI (light sleep only)");
	break;
		
    case ESP_SLEEP_WAKEUP_COCPU:
		Serial.println("Wakeup:COCPU");
	break;
		
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
		Serial.println("Wakeup:COCPU crash");
	break;
		
    case ESP_SLEEP_WAKEUP_BT:
		Serial.println("Wakeup:BT (light sleep only)");
	break;
		
    case ESP_SLEEP_WAKEUP_VAD:
		Serial.println("Wakeup:VAD");
	break;

    default : 
		Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason);
	break;
  }
}

//-------------------------------------------------------------
// home/dwade/.arduino15/packages/m5stack/tools
// /esp32-arduino-libs/idf-release_v5.4-858a988d-v1
// /esp32s3/include/esp_hw_support/include/esp_sleep.h

#include "esp_sleep.h"
#include "driver/gpio.h"

uint8_t doNothing;

// ISR for wakeupPin
void IRAM_ATTR handleInterrupt1() 
{
    doNothing++;  // stop optimizer from deleting entire function.
}

void setupLightSleepByTimer(uint32_t timeMs)
{
	esp_sleep_enable_timer_wakeup(timeMs * 1000);
	TRACE("prepared light sleep timer for %d mS\n", timeMs);
}

void enterLightSleepTimer(void)
{
	uint32_t now = millis();
	esp_light_sleep_start();
	TRACE("exit sleep.. time = %d000 mS\n", millis() - now);
}

void setupSleepByGPIO(gpio_num_t wakeupPin) 
{
    //pinMode(wakeupPin, INPUT_PULLUP); // pull-up resistor

    // Configure GPIOs as wake-up source
    gpio_wakeup_enable(wakeupPin, GPIO_INTR_LOW_LEVEL); // Trigger wake-up on high level

    // Enable GPIO wake-up source
    esp_err_t result = esp_sleep_enable_gpio_wakeup();

    if (result == ESP_OK) {
        Serial.println("GPIO Wake-Up set successfully.");
    } else {
        Serial.println("Failed to set GPIO Wake-Up as wake-up source.");
    }

    // Attach interrupts to GPIO pin
    attachInterrupt(digitalPinToInterrupt(wakeupPin), 
    				handleInterrupt1, 
    				FALLING);
	
}

//-------------------------------------------------------------
void enterLightSleepGPIO() 
{
    TRACE("Enter Sleep\n");
	uint32_t ms = millis();
    esp_light_sleep_start();    // Enter light sleep
    Serial.printf("  Woke up at %d ms\n", millis() - ms);
	
    reportWakeUpCause();
}


