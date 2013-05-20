#ifndef SENSORUTILS_H
#define SENSORUTILS_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <inttypes.h>

/******************************************************************************
 * Definitions & Declarations
 ******************************************************************************/
//#define SENS_DEBUG
#define SENS_DEBUG_V2

typedef enum {TEMPERATURE = 0x0001, HUMIDITY = 0x0002, PRESSURE = 0x0004, 
	BATTERY = 0x0008, CO2 = 0x0010, ANEMO = 0x0020, VANE = 0x0040, PLUVIO = 0x0080,
	LUMINOSITY = 0x0100, SOLAR_RADIATION = 0x0200} 
	SensorType;
	
typedef enum {VANE_N, VANE_NNE, VANE_NE, VANE_ENE, 
			  VANE_E, VANE_ESE, VANE_SE, VANE_SSE, 
			  VANE_S, VANE_SSW, VANE_SW, VANE_WSW, 
			  VANE_W, VANE_WNW, VANE_NW, VANE_NNW} 
	VaneDirection;

/*
#define SENS_AGR_VANE_N		0
#define SENS_AGR_VANE_NNE	1
#define SENS_AGR_VANE_NE	2
#define SENS_AGR_VANE_ENE	4
#define SENS_AGR_VANE_E		8
#define SENS_AGR_VANE_ESE	16
#define SENS_AGR_VANE_SE	32
#define SENS_AGR_VANE_SSE	64
#define SENS_AGR_VANE_S		128
#define SENS_AGR_VANE_SSW	256
#define SENS_AGR_VANE_SW	512
#define SENS_AGR_VANE_WSW	1024
#define SENS_AGR_VANE_W		2048
#define SENS_AGR_VANE_WNW	4096
#define SENS_AGR_VANE_NW	8192
#define SENS_AGR_VANE_NNW	16384
*/	
	
#define NUM_MEASUREMENTS 10
#define NUM_SENSORS 10  //MUST BE <=16 FOR CURRENT EEPROM CONFIGURATION


//! Function pointers to save sensor data
/*! Since the size of the data to insert is constant there's no reason to
/*! make these functions non-static / member functions.
 *	\return	error=SensorType --> EEPROM FULL, request to send the values
 *			error=0	--> The command has been executed with no errors
 */	
typedef uint16_t StoreSensorData();
	extern uint16_t saveTemperature();
	extern uint16_t saveHumidity();
	extern uint16_t savePressure();
	extern uint16_t saveBattery();
	extern uint16_t saveCO2();
	


/******************************************************************************
 * Class
 ******************************************************************************/
 
class SensorUtils
{
	private:
		void (SensorUtils::*reader[8])(void);
	
	public:
		//! class constructor
		/*!
		  It does nothing
		  \param void
		  \return void
		*/
		SensorUtils();
		
		
		//! It gets the current SENS_TEMPERATURE value
		/*!
		It stores in global variable 'temperature' the currently measured temperature.
		This value is actually the average of 10 values read from the sensor.
		*/
		void measureTemperature();
		
		
		//! It gets the current SENS_HUMIDITY value
		/*!
		It stores in global 'humidity' variable the currently measured humidity.
		This value is actually the average of 10 values read from the sensor.
		*/
		void measureHumidity();
		
		
		//! It gets the current SENS_PRESSURE value
		/*!
		It stores in global variable 'pressure' the currently measured atm pressure.
		This value is actually the average of 10 values read from the sensor.
		!!!!! Also had to do a "callibration" in WaspSensorGas_v20::pressureConversion(int readValue)
		!!!!! According to libelium-dev this is the "correct" way to obtain an acceptable result:
		!!!!!  "you may find some offset because we adjusted the formula to fit the output value of the 
		!!!!!  sensors we used to test the board and had to correct a small error, and yours may differ 
		!!!!!  a little bit, so adjust this value (in the formula in the API is a 270) may help to have 
		!!!!!  a more accurate value."
		!!!!!		=> Changed +270 into -130 in the formula
		*/
		void measurePressure();
		
		
		//! It gets the current BATTERY level
		/*!
		It stores in global variable 'battery' the currently measured battery level.
		This value is actually the average of 10 values read from the sensor.
		*/
		void measureBattery();
		
		
		//! It gets the current SENS_CO2 value
		/*!
		It stores in global variable 'co2' the currently measured CO2 level.
		The sensor will be warming up (30 sec) first. 
		*/		  
		void measureCO2();  
		
		/// WEATHER_STATION //////////////////////////////////////////////////
		#ifdef WEATHER_STATION
			void measureAnemo();
			void measureVane();
			
			
			//! It gets the current SENS_AGR_PLUVIOMETER value
			/*!
			It stores in global variable 'current_rainfall' the currently measured 
			rainfall in mm/min.  (not very accurate)
			*/				
			void measureCurrentRainfall();
			
			
			//! Interrupt Service Routine called when a PluvioInt has been generated
			/*!
			It stores the summative rainfall since the previous resetPluviometer(),
			not yet in mm. Use getSummativeRainfall() to get the value in mm.
			*/
			void rainfall_ISR();
			
			
			//! It stores the summative rainfall in mm since the previous resetPluviometer()
			//! in global 'summativeRainfallInMM'
			void getSummativeRainfall();
			
			
			void resetPluviometer();
			
			void measureLuminosity();
			void measureSolarRadiation();
			
			void convertVaneDirection();
		#endif /*WEATHER_STATION*/ 
		/// #endif /*WEATHER_STATION*/ //////////////////////////////////////////////	
		
		  
		//! Measures all sensors found as arguments
		//!!!!! The first argument must be the number of sensor types followed
		/*!!!!! This function takes care of all turning ON/OFF board and sensor requirements !!!!!
		  \return  	error=2 --> The command has not been executed
					error=1 --> The MASK was 0, no sensors measured
					error=0 --> The command has been executed with no errors
		*/
		uint8_t measureSensors(int, ...);
		
		  
		//! Measures all sensors found in the mask argument
		/*!!!!! This function takes care of all turning ON/OFF board and sensor requirements !!!!!
		  \return  	error=2 --> The command has not been executed
					error=1 --> The MASK was 0, no sensors measured
					error=0 --> The command has been executed with no errors
		*/
		uint8_t measureSensors(uint16_t);
		
		  
		//! Converts a float sensor value to its most compact binary value according
		/*! to the corresponding decimals per sensor type.
		  \param float: the sensor value to convert
		  \param SensorType: the SensorType
		  \return  	error=2 --> The command has not been executed
					error=1 --> There has been an error while executing the command
					error=0 --> The command has been executed with no errors
		*/ 
		uint8_t sensorValue2Chars(float, SensorType);
		
		
		//! Measures and stores all sensors found in the mask argument
		/*!!!!! This function takes care of all turning ON/OFF board and sensor requirements !!!!!
		  \return  	error=3 --> EEPROM FULL, values must be sent
					error=2 --> The command has not been executed
					error=1 --> The MASK was 0, no sensors measured
					error=0 --> The command has been executed with no errors
		*/		
		uint8_t measureAndstoreSensorValues(uint16_t);
		void storeMeasuredSensorValues(uint16_t);
		
		
		//! It enters the sensor's measuring interval time to the correct position in 
		//! the measuringInterval[NUM_SENSORS] array
		uint8_t registerSensorMeasuringIntervalTime(SensorType, uint16_t);
		
		
		//! It saves the values in measuringInterval[NUM_SENSORS] to the correct positions
		//! in EEPROM
		void saveSensorMeasuringIntervalTimes();
		
		//! It reads the values of the currently active sensors from EEPROM and stores them
		//! into 'uint16_t measuringInterval[NUM_SENSORS]'
		void readSensorMeasuringIntervalTimesFromEEPROM();
		
		//! Variable : the averaged temperature value
		/*!
		 */
		float temperature;
		
		//! Variable : the temperature value in bytes
		/*! TEMPERATURE SENSOR:   RANGE: -40� -> +125� 
		/*!  offset += 40� for negative values,
		/*!  *= 100 for 2 decimals accuracy
		/*!	 --> fits in 2 bytes
		 */		
		unsigned char temp[2];

		//! Variable : the averaged humidity value
		/*!
		 */
		float humidity;
		
		//! Variable : the humidity value in bytes
		/*! HUMIDITY SENSOR:   RANGE: 0 -> 100%
		 */
		unsigned char hum;
		
		//! Variable : the averaged atm pressure value
		/*!
		 */
		float pressure;
		
		//! Variable : the pressure value in bytes, 
		/*! PRESSURE SENSOR:   RANGE: 15 - 115kPa
		/*!   force in 2 bytes:  115000 - 2^16 = 50000  --> RANGE: 50kPa - 115kPa
		/*!	  offset -= 50000
		 */
		unsigned char pres[2];
		
		//! Variable : the averaged battery level
		/*!
		 */		
		uint16_t battery;
		
		//! Variable : the battery level in bytes
		/*! BATTERY LEVEL:   RANGE: 0 -> 100%
		 */
		unsigned char bat;
		
		//! Variable : the CO2 value
		/*!
		 */
		float co2;
		
		//! Variable : the CO2 value in bytes, 
		/*! CO2 SENSOR:   RANGE: 350 - 10000 ppm
		/*!   normal outdoor level: 350 - 450 ppm; acceptable levels: < 1500 ppm (UK standard for 
		/*!   teaching and learning spaces) 
		/*!	  --> 2B
		 */
		unsigned char co_2[2];		
		
		//! Variable : the anemo value
		/*! RANGE: 0 - 240 km/h
		 */			
		float anemo;
		
		unsigned char an;
		
		
		//! Variable : the anemo value
		/*!
		 */			
		float vane;
		
		VaneDirection vaneDirection;
		
		
		float pluvio;
		float summativeRainfallInMM;
		unsigned char sum_rain[2];
		uint16_t pluviometerCounter;
		bool startedRaining;
		uint16_t startedRainingTime;
		
		float luminosity;
	
		float solar_radiation;

		//!
		/*! Stores the individual measuring interval times of the sensors
		 */
		uint16_t measuringInterval[NUM_SENSORS];
		
		//!
		/*! Stores the biggest measuring interval time of all the active sensors
		 */
		uint16_t maxTime;
		
		//!
		/*! Stores the smallest measuring interval times of all the active sensors
		 */
		uint16_t minTime;
		
		uint16_t acceptedSensorMask;
		
		long previous;
	
 };
 
 extern SensorUtils SensUtils;


#endif /*SENSORUTILS_H*/
