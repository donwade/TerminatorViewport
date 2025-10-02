#define LOCAL_HOSTNAME REMOTE_HOSTNAME

extern void setupLightSleepByTimer(uint32_t timeMs);
extern void enterLightSleepTimer(void);

//Options are: 240, 160, 120, 80, 40, 20 and 10 MHz
const unsigned int CPU_FREQ = 80;  // can be 240 (default)
const unsigned int CPU_SCALE=240/CPU_FREQ;

extern void setup_ota();
extern void loop_ota();



