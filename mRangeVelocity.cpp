/*!
 * @file  mRangeVelocity.ino
 * @brief  radar measurement demo
 * @copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author ZhixinLiu(zhixin.liu@dfrobot.com)
 * @version V1.0
 * @date 2024-02-02
 * @url https://github.com/dfrobot/DFRobot_C4001
 */

#include "mRangeVelocity.h"

#ifdef ARDUINO_M5STACK_CORE2
#include <M5Unified.h>
#include "viewController.h"
#include "watchdogs.h"
#endif

#include "DFRobot_C4001.h"

//#define I2C_COMMUNICATION  //use I2C for communication, but use the serial port for communication if the line of codes were masked

#ifdef  I2C_COMMUNICATION
/*
 * DEVICE_ADDR_0 = 0x2A     default iic_address
 * DEVICE_ADDR_1 = 0x2B
 */
DFRobot_C4001_I2C radar(&Wire, DEVICE_ADDR_0);
#else
/* ---------------------------------------------------------------------------------------------------------------------
 *    board   |             MCU                | Leonardo/Mega2560/M0 |    UNO    | ESP8266 | ESP32 |  microbit  |   m0  |
 *     VCC    |            3.3V/5V             |        VCC           |    VCC    |   VCC   |  VCC  |     X      |  vcc  |
 *     GND    |              GND               |        GND           |    GND    |   GND   |  GND  |     X      |  gnd  |
 *     RX     |              TX                |     Serial1 TX1      |     5     |   5/D6  |  D2   |     X      |  tx1  |
 *     TX     |              RX                |     Serial1 RX1      |     4     |   4/D7  |  D3   |     X      |  rx1  |
 * ----------------------------------------------------------------------------------------------------------------------*/
/* Baud rate cannot be changed */
#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
SoftwareSerial mySerial(4, 5);
DFRobot_C4001_UART radar(&mySerial, 9600);
#elif defined(ESP32)
DFRobot_C4001_UART radar(&Serial1, 9600, /*rx*/ 13, /*tx*/ 14);
#else
DFRobot_C4001_UART radar(&Serial1, 9600);
#endif
#endif


void setup_c4001()
{

    while (!radar.begin())
    {
        Serial.println("NO Deivces !");
        delay(1000);
    }

    Serial.println("Device connected!");

#if 0
    radar.setSensorMode(eExitMode);     // motion detection
#else
    radar.setSensorMode(eSpeedMode);	// speed detection
#endif

    sSensorStatus_t data;
    data = radar.getStatus();
    //  0 stop  1 start
    Serial.printf("work status = %s\n", data.workStatus ?  "running" :"stopped");

    //  0 is exist   1 speed
    Serial.printf("work mode  = %s\n", data.workMode ? "speed" : "motion" );

    //  0 no init    1 init success
    Serial.printf("init status = %s\n", data.initStatus ? "init OK" : "not inited");
    Serial.println();


    /*
     * min Detection range Minimum distance, unit cm, range 0.3~20m (30~2000), not exceeding max, otherwise the function is abnormal.
     * max Detection range Maximum distance, unit cm, range 2.4~20m (240~2000)
     * thres Target detection threshold, dimensionless unit 0.1, range 0~6553.5 (0~65535)
     */
    if (radar.setDetectThres(/*min*/ 33,
                             /*max*/ 2000,
                             /*thres*/ 9.))
    {
        Serial.println("set detect threshold successfully");
	}

	if(radar.setDetectionRange(/*min*/30, /*max*/2000, /*trig*/2000))
	{
		Serial.println("set detection range successfully!");
	}


	// doesn't work if in speed mode
	if (radar.setPwm(1, 50, 3)) // 0=nobody 100=somebody 3*64ms = .180 s
	{
		Serial.println("pmw set ");
	}

    // set Fretting Detection
    radar.setFrettingDetection(eOFF);
    //radar.setFrettingDetection(eON);

	// set trigger sensitivity 0 - 9
	if(radar.setTrigSensitivity(1))
	{
		Serial.println("set trig sensitivity successfully!");
	}
	
	// set keep sensitivity 0 - 9
	if(radar.setKeepSensitivity(2))
	{
		Serial.println("set keep sensitivity successfully!");
	}

	
	/*
	* trig Trigger delay, unit 0.01s, range 0~2s (0~200)
	* keep Maintain the detection timeout, unit 0.5s, range 2~1500 seconds (4~3000)
	*/
	
	if(radar.setDelay(10  /* uint8_t  x .01s trig delay*/,
					   4  /* uint16_t x .5s  delay when detected*/))
	{
		Serial.println("set delay successfully!");
	}

    // get configure params
    Serial.printf("getTMinRange range = %d\n", radar.getTMinRange());
    Serial.printf("getTMaxRange range = %d\n\n", radar.getTMaxRange());
    
    Serial.printf("threshold range = %d\n", radar.getThresRange());
    Serial.printf("fretting detection = %d\n", radar.getFrettingDetection());

	Serial.printf("\nkeepSensitivity = %d\n", radar.getKeepSensitivity());
	Serial.printf("trigSensitivity = %d\n\n", radar.getTrigSensitivity());

    sPwmData_t foo = radar.getPwm();
    Serial.printf("pmw1 quiet = %d\n", foo.pwm1);
    Serial.printf("pmw2 detect = %d\n", foo.pwm2);
    Serial.printf("time = %d mS\n\n", foo.timer * 64);

	Serial.printf("keep time = %d\n", radar.getKeepTimerout());
	Serial.printf("trig delay = %d\n", radar.getTrigDelay());
	
    Serial.printf("c4001 setup done\n--------------------\n\n");
    
}


#define USE_RANGE

void loop_c4001()
{
	kickDog();
    static float fmax = -100;
    static float fmin = 0;
    static bool isIdle = true;
	uint8_t tnum = 0;
	
#ifdef USE_RANGE
    static float stateOld = -1.0;
	// must read target number or any other val req == 0!!!!
	tnum = radar.getTargetNumber();
    float stateNow = radar.getTargetRange();
#else
	static bool stateOld = true;
	bool stateNow = bPersonDetected();
	// must read target number or any other val req == 0!!!!
#endif

    if (stateNow != stateOld)
    {
        if (!stateNow)
        {
            colourBarX(_GREEN, 10);
        }
        else
        {
            setToggleColors(_RED, _BLUE, 10);
		}
        stateOld = stateNow;
    }


	if (stateNow)
    {
		float speedMpS = radar.getTargetSpeed();
		float speedkpH = speedMpS * 60. / 1000.;
		uint32_t energyNow = radar.getTargetEnergy();
		float range = radar.getTargetRange();
		
        if (isIdle)
            Serial.println();

        isIdle = false;

        Serial.print("target number = ");
        Serial.println(tnum);     // must exist
        Serial.printf("target Speed  = %4.1f m/S %4.1f kpH\n", speedMpS, speedkpH);

        Serial.print("target range  = ");
        Serial.print(range);
        Serial.println(" m");

        Serial.print("target energy    = ");
        Serial.println(energyNow);

        if (energyNow)
        {
            float dbNow = radar.getTargetEnergyDb();

            if (dbNow > fmax)
                fmax = dbNow;

            if (dbNow < fmin)
                fmin = dbNow;

            Serial.printf("%5.3f < %5.3f < %5.3f\n\n", fmin, dbNow, fmax);
        }
    }
    else
    {
        //Serial.print('.');
        isIdle = true;
    }

	delay(100);
}


/*
bool bPersonDetected(void) 
{
	//radar.getTargetNumber();

	sSensorStatus_t foo = radar.getStatus();
	Serial.printf("foo.workStatus = %d\n", foo.workStatus);
	Serial.printf("foo.workMode = %d\n", foo.workMode);
	Serial.printf("foo.initStatus = %d\n\n", foo.initStatus);

	return foo.workMode;

	//no it hangs
	//return radar.getIoPolaity();
}
*/
