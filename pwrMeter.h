/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerSwrMeter

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

//#ifdef TEENSY40
//#define SAMPLE_INTERVAL 200						    // ADC sample timer interval (microsecs)
//#define MAXBUF          10000						// maximum averaging buffer/samples size
//
//#else
//#define SAMPLE_INTERVAL 400						    // ADC sample timer interval (microsecs)
//#define MAXBUF          2000						// maximum averaging buffer/samples size
//
//#endif
//

/*---------- Teensy pin assignments (use for wiring) --------------------------------------
board: referes to ILI9341 + Touch board   */
#define	DIM_PIN         3							// board: 8(LED) analog out pin for display LED & dimming
#define	TFT_DC_PIN      9                           // board: 5(DC) tft data pin
#define	TFT_CS_PIN      10							// board: 10(CS) TFT chip select pin
#define	TS_IRQ_PIN      2							// board: 14(T_IRQ) touch interrupt pin
#define	TS_CS_PIN       6							// board: 6(T_CS) Touch CS. *** do NOT use pin 8...  required for Serial3
#define TFT_SDI         11                          // board: 6(SDI), 12(T_DIN)     *** used in library, not referenced in code
#define TFT_SCK         13                          // board: 7(SCK), 10(T_CLK)     *** used in library, not referenced in code
#define TFT_SDO         12                          // board: 9(SDO), 13(T_DO)      *** used in library, not referenced in code

#define FWD_ADC_PIN     15                          // (A1) ACD input pin - forward power
#define REF_ADC_PIN     16                          // (A2) ACD input pin- reflected power
#define SERIAL_RX1      0                           // Serial1 RX pin
#define SERIAL_TX1      1                           // Serial1 Tx pin
#define SERIAL_RX3      7                           // Serial3 RX pin
#define SERIAL_TX3      8                           // serial3 Tx pin
#define	TEST_PIN        4							// high/low pulse output for timing

#ifdef CIV
/*---------------------------Serial ports -------------------*/
#define	civSerial       Serial1					    // uses serial1 rx/tx pins 0,1
#define	btSerial        Serial3					    // bluetooth serial3 - pins 7,8
#endif

/*------  measure() constants -------------------------------*/
#define	SAMPLE_FREQ		5000						// effective ADC sampling frequency - hertz
#define MAXBUF			SAMPLE_FREQ
#define PEAK_HOLD		2000					// average Peak Pwr hold time (mSecs)
#define PEP_HOLD		250					// average pep hold time (mSecs)
#define PWR_THRESHOLD   .5    						// power on threshold watts
#define	FV_ZEROADJ      -0.0000 				    // ADC zero offset voltage
#define	RV_ZEROADJ      -0.0000				        // ADC zero offset voltage

///*------  measure() constants -------------------------------*/
// Direct power conversion constants
// forward power constants - new coupler
// 03/06/2020
#define	FWD_V_SPLIT_PWR         0.015				// split voltage, direct pwr conversion
#define	FWD_LO_MULT_PWR         0.0969				// LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
#define FWD_LO_ADD_PWR          0.8719
#define	FWD_HI_MULT2_PWR        10.384
#define FWD_HI_MULT1_PWR		6.3687
#define FWD_HI_ADD_PWR			0.4894				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR

//// Direct power conversion constants
//// forward power constants - new coupler
//// 01/06/2020
//#define	FWD_V_SPLIT_PWR         0.0175				// split voltage, direct pwr conversion
//#define	FWD_LO_MULT_PWR         0.068				// LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
//#define FWD_LO_ADD_PWR          0.7459
//#define	FWD_HI_MULT2_PWR        11
//#define FWD_HI_MULT1_PWR       6
//#define FWD_HI_ADD_PWR         .45				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR

//// 29/05/2020
//#define	FWD_V_SPLIT_PWR         0.05				// split voltage, direct pwr conversion
//#define	FWD_LO_MULT_PWR         0.1106				// LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
//#define FWD_LO_ADD_PWR          1.0051
//#define	FWD_HI_MULT2_PWR        10.673
//#define FWD_HI_MULT1_PWR        5.7654
//#define FWD_HI_ADD_PWR          0.567				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR

//#define	FWD_V_SPLIT_PWR         0.07				// split voltage, direct pwr conversion
//#define	FWD_LO_MULT_PWR         0.1949				// LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
//#define FWD_LO_ADD_PWR          1.2297
//#define	FWD_HI_MULT2_PWR        10.381
//#define FWD_HI_MULT1_PWR        5.7381
//#define FWD_HI_ADD_PWR          0.4364				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR
//// reflected power constants
//#define	REF_V_SPLIT_PWR         0.07				// split voltage, direct pwr conversion
//#define	REF_LO_MULT_PWR         0.1949				// LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
//#define REF_LO_ADD_PWR          1.2297
//#define	REF_HI_MULT2_PWR        10.381
//#define REF_HI_MULT1_PWR        5.7381
//#define REF_HI_ADD_PWR          0.4364				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR

// orig coupler
//#define	FWD_V_SPLIT_PWR     0.02				    // split voltage, direct pwr conversion
//#define	FWD_LO_MULT_PWR     0.1308				    // LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
//#define FWD_LO_ADD_PWR      0.9748
//#define	FWD_HI_MULT2_PWR    10.178
//#define FWD_HI_MULT1_PWR    5.7523
//#define FWD_HI_ADD_PWR      0.3202				    // HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR
//// reflected power constants
//#define	REF_V_SPLIT_PWR     0.02				    // split voltage, direct pwr conversion
//#define	REF_LO_MULT_PWR     0.1308				    // LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
//#define REF_LO_ADD_PWR      0.9748
//#define	REF_HI_MULT2_PWR    10.178
//#define REF_HI_MULT1_PWR    5.7523
//#define REF_HI_ADD_PWR      0.3202					// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR


#ifdef CIV
/*----------Icom CI-V Constants------------------------------*/
#define CIVADDR         0xE2			        	// this controller address
#define CIVRADIO        0x94						// Icom IC-7300 CI-V default address
#define CIV_WRITE_DELAY 3					        // CIV write delay = milliseconds
#define CIV_READ_DELAY  1						    // CIV read delay to ensure rx buffer fill
#endif

/*----------ILI9341 TFT display (320x240)-------------------------*/
ILI9341_t3	tft = ILI9341_t3(TFT_CS_PIN, TFT_DC_PIN);		// define tft device
// ILI9341_t3(uint8_t _CS, uint8_t _DC, uint8_t _RST = 255, uint8_t _MOSI = 11, uint8_t _SCLK = 13, uint8_t _MISO = 12);

#define		TFT_FULL 255							// tft display full brightness
#define		TFT_DIM	20								// tft display dim value
#define		TFT_OFF	0								// tft display off
#define		SPLASH_DELAY 4 * 1000						// splash screen dealy, mSecs.. At power on, allow time for radio to boot

/*----------XPT2046 touchscreen	-----------------------------*/
XPT2046_Touchscreen ts(TS_CS_PIN, TS_IRQ_PIN);		// allows touch screen interrupts
#if			TOUCH_REVERSED
int xMapL = 3850, xMapR = 500;                      // reversed touch screen mapping
int yMapT = 3800, yMapB = 300;
#else
int xMapL = 320, xMapR = 3850;                      // touch screen mapping
int yMapT = 300, yMapB = 3800;
#endif
#define		MAPX map(p.x, xMapL, xMapR, 0, 320)		// touch screen mapping expresssion
#define		MAPY map(p.y, yMapT, yMapB, 0, 240)

const int SHORTTOUCH = 1;
const int LONGTOUCH = 2;

/*----------Metro timers-----------------------------------------*/
Metro heartBeatTimer = Metro(250);			        // heartbeat timer
Metro longTouchTimer = Metro(750);			        // long touch timer
Metro dimTimer = Metro(15 * 60 * 1000);				// dimmer timer (mins)
Metro netPwrPkTimer = Metro(1000);					// peak power hold timer
Metro pepTimer = Metro(1000);				   		// pep hold timer
//Metro holdPwrTimer = Metro(20);						// hold time - signal processing

#ifdef CIV
Metro civTimeOut = Metro(100);						// civ read/write watchdog timer
Metro aBandTimer = Metro(1000);						// autoband time milliseconds, auto reset
#endif


// constant expression to convert BCD to decimal
constexpr int getBCD(int n)
{
	return (n / 16 * 10 + n % 16);
}

// constant expression to convert decimal to BCD
constexpr int putBCD(int n)
{
	return (n / 10 * 16 + n % 10);
}

/* Teensy code from forum
*/

/*---------- Teensy restart code (long press on Peak Power frame)--------*/
#define		CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define		CPU_RESTART_VAL 0x5FA0004
#define		CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

/*----------- code to get Teeny type -----------------------------------*/
//uint32_t cpuid = ((*(volatile uint32_t*)0xE000ED00) & 0xfff0);
//
//uint16_t dev_id;
//if (cpuid == 0xC240) dev_id = 1;		//T3.1	(M4)
//else if (cpuid == 0xC600) dev_id = 2;	//TLC	(M0+)
//else if (cpuid == 0xC270) dev_id = 3;	//T4.0	(M7)
//else dev_id = 0;

#define     CPU_ID ((*(volatile uint32_t*)0xE000ED00) & 0xfff0);

/*----------Teensy - enable printf functions------------------------------------*/
/* this doesn't seem to be required
 this is the Teensy magic trick for  printf to support float
asm(".global _printf_float");
 this is the magic trick for scanf to support float
asm(".global _scanf_float");
 */

 // set up default arguements for functions
void displayValue(int posn, float curr, bool isUpdate = false);
void displayLabel(int posn, char* = NULL);
