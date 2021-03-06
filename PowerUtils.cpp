/* ==========================================================================
 *
 *			THESIS: Design of a Wireless Sensor Networking test-bed
 * 
 * ==========================================================================
 *
 *       Filename:  RTCUtils.cpp
 *    Description:  Extra functionality for time settings
 *
 * ======================================================================= */
 
#ifndef __WPROGRAM_H__
	#include "BjornClasses.h"
	#include "WaspClasses.h"
#endif

#include <inttypes.h>

/**************************************************************************************
  *
  * DEEP SLEEP / HIBERNATE UTILITIES
  *
  *************************************************************************************/
  
/// ///////////////////////////////////////////////////////////////
///  INDOOR NODES - GASSES SENSOR BOARDS  
/// ///////////////////////////////////////////////////////////////
#ifndef WEATHER_STATION  
  
void PowerUtils::enterLowPowerMode(SleepMode sm, uint16_t howLong)
{
	if(sm == HIBERNATE)
	{
		RTCUt.getTime();
		RTCUt.setNextTimeWhenToWakeUpViaOffset(howLong);
		PWR.hibernate(RTCUt.nextTime2WakeUpChar, RTC_ABSOLUTE, RTC_ALM1_MODE3);
	}
	else
	{	
		for(uint16_t i=0; i<howLong; i++) 
		{
			PWR.sleep(WTD_8S, ALL_OFF);
			PWR.sleep(WTD_2S, ALL_OFF);
		}
		
		if( intFlag & WTD_INT )  //"can be used as out of sleep interrupt"
        {
			intFlag &= ~(WTD_INT);
        }
		
		xbeeZB.ON();
      	USB.begin();
			#ifdef HIBERNATE_DEBUG_V2
				USB.print("\n\n\n-");  for(uint8_t i=0; i<60; i++){  USB.print("-"); }
				USB.print("\nout sleep");
			#endif
	}	
}

 
//! This function REQUIRES that the correct time to sleep (absolute for hibernate and deep sleep)
//! or sleep time (offset for sleep) is known in either RTCUt.nextTime2WakeUpChar (hibernate) or in
//! time2wValuesArray[posArray] (sleep/deep). 
void PowerUtils::enterLowPowerMode(SleepMode sm, XBeeSleepMode xbs) ///AT END OF LOOP()
{
	if( !RTCUt.isStillNextTimeToWakeUp() )
	{
		COMM.sendError(SLEEP_TIME_EXCEEDED);
		skipThisTime2Wake(sm);
	}

	if(sm == HIBERNATE)
	{
			#ifdef FINAL_USB_DEBUG
				USB.print("HIBERNATE at "); USB.print(RTC.getTime());
				USB.print(" till "); USB.println( RTCUt.nextTime2WakeUpChar );
			#endif
		
		/// Store important parameters
		xbeeZB.storeProgramParametersToEEPROM();	
	
		PWR.hibernate(RTCUt.nextTime2WakeUpChar, RTC_ABSOLUTE, RTC_ALM1_MODE3);
		/// After hibernate we start in WaspXBeeZBNode::hibernateInterrupt()
	}
	else if(sm == DEEPSLEEP)  /// CANNOT BE COMBINED WITH HIBERNATE
	{
			#ifdef FINAL_USB_DEBUG
				USB.print("DEEPSLEEP at "); USB.print(RTC.getTime());
				USB.print(" till "); USB.println( RTCUt.nextTime2WakeUpChar );
			#endif
			
		if(xbs == XBEE_SLEEP_DISABLED)
		{
			PWR.deepSleep(RTCUt.nextTime2WakeUpChar, RTC_ABSOLUTE, RTC_ALM1_MODE3, ALL_OFF);	
			xbeeZB.ON();
		}
		else
		{
			PWR.deepSleep(RTCUt.nextTime2WakeUpChar, RTC_ABSOLUTE, RTC_ALM1_MODE3, SENS_OFF | UART1_OFF | BAT_OFF | RTC_OFF);
			//xbeeZB.ON();
			xbeeZB.wake();
		}
		
		RTC.ON();
		RTCUt.getTime();
		RTC.setMode(RTC_OFF,RTC_NORMAL_MODE);  //this will save power	
		USB.begin();
		
			#ifdef FINAL_USB_DEBUG
				USB.print("\n-");  for(uint8_t i=0; i<60; i++){  USB.print("-"); }
				USB.print("\nAwake at "); USB.println(RTC.getTime());
			#endif
			
		if(xbeeZB.defaultOperation)
		{
			RTCUt.setNextTimeWhenToWakeUpViaOffset(xbeeZB.defaultTime2WakeInt);
		}
		else
		{
				#ifdef DEEPSLEEP_DEBUG_V2
					USB.print("out of sleep, posInArray = ");
					USB.println( (int) xbeeZB.posInArray );
					USB.print("next t2s = ");
					USB.println( (int) xbeeZB.time2wValuesArray[xbeeZB.posInArray]);
					USB.println( (int) xbeeZB.time2wValuesArray[xbeeZB.posInArray + 1]);
					USB.println( (int) (xbeeZB.time2wValuesArray[xbeeZB.posInArray + 1] - xbeeZB.time2wValuesArray[xbeeZB.posInArray]) );
				#endif				
				
			xbeeZB.findNextTime2Wake(DEEPSLEEP);
			xbeeZB.updatePosInArray();	
		}
		/// GOTO "device enters loop"
	}
	else if(sm == SLEEP)
	{
			#ifdef FINAL_USB_DEBUG
				USB.print("SLEEP at "); USB.println(RTC.getTime());
			#endif
		sleepTillNextTime2Wake(xbs);

		RTC.ON();
		RTCUt.getTime();
		RTC.setMode(RTC_OFF,RTC_NORMAL_MODE);  //this will save power
		RTCUt.setAwakeAtTime();
		
		USB.begin();
			#ifdef FINAL_USB_DEBUG
				USB.print("\n\n\n-");  for(uint8_t i=0; i<60; i++){  USB.print("-"); }
				USB.print("\nAwake at");  USB.println(RTC.getTime());
			#endif		
	
		if( intFlag & WTD_INT )  //"can be used as out of sleep interrupt"
		{
			intFlag &= ~(WTD_INT);
			if(!xbeeZB.defaultOperation) xbeeZB.updatePosInArray();
				#ifdef SLEEP_DEBUG
					USB.print("posInArray ");
					USB.println( (int) xbeeZB.posInArray );
				#endif
			/// GOTO "device enters loop"
		}
	}	
}


void PowerUtils::enterLowPowerMode(SleepMode sm)
{
	enterLowPowerMode(sm, XBEE_SLEEP_DISABLED);
}


void PowerUtils::enterLowPowerMode() ///AT END OF LOOP()   AUTO SELECTION SLEEP MODE 
{
	if(RTCUt.nextTime2WakeUpHoursMinsSecsInt <= 2)
		enterLowPowerMode(SLEEP, XBEE_SLEEP_ENABLED); /// Then we can spare 2.5 seconds in the next loop
	else
		enterLowPowerMode(HIBERNATE, XBEE_SLEEP_DISABLED);
} 


void PowerUtils::hibernate()
{
		#ifdef HIBERNATE_DEBUG_V1
			USB.print("\n\nEntering HIBERNATE at ");
			USB.println(RTC.getTime());
			USB.print("for ");
			USB.println(xbeeZB.defaultTime2WakeStr);
		#endif
		
		#ifdef HIBERNATE_DEBUG_V2
			USB.print("\n\nEntering HIBERNATE at ");
			USB.println(RTC.getTime());
			USB.print("till ");
			USB.println(RTCUt.nextTime2WakeUpChar);
		#endif
			
	xbeeZB.storeProgramParametersToEEPROM();
	
	PWR.hibernate(RTCUt.nextTime2WakeUpChar, RTC_ABSOLUTE, RTC_ALM1_MODE3);
		/// PWR.hibernate(RTC.getAlarm1(),RTC_ABSOLUTE,RTC_ALM1_MODE3);					NOT WORKING
		/// PWR.hibernate(xbeeZB.defaultTime2WakeStr, RTC_OFFSET, RTC_ALM1_MODE2);		NOT WORKING
}


void PowerUtils::sleepTillNextTime2Wake(XBeeSleepMode xbs)
{
	uint8_t secondsBeenAwake;		/// Sleep will only be used for max 59 seconds
	uint8_t time2sleep;
	uint16_t time = 0;
		#ifdef SLEEP_DEBUG
			uint8_t count = 0;
		#endif
	RTC.getTime();
	if( RTC.second > RTCUt.RTCAwakeAtSeconds )
	{
		secondsBeenAwake = RTC.second - RTCUt.RTCAwakeAtSeconds;
			#ifdef SLEEP_DEBUG
				USB.print("\nsecAwake: "); USB.println( (int) secondsBeenAwake);
			#endif
	}
	else
	{
		secondsBeenAwake = RTC.second + (60 - RTCUt.RTCAwakeAtSeconds);
			#ifdef SLEEP_DEBUG
				USB.print("\nsecAwake: "); USB.println( (int) secondsBeenAwake);
			#endif
	}
	if(xbeeZB.defaultOperation) time = xbeeZB.defaultTime2WakeInt;
	else time = xbeeZB.time2wValuesArray[xbeeZB.posInArray+1] - xbeeZB.time2wValuesArray[xbeeZB.posInArray];
	
	time2sleep = time *	10 - secondsBeenAwake;
			#ifdef SLEEP_DEBUG
				USB.print("time2sleep: "); USB.println( (int) time2sleep );
			#endif
	if(xbs == XBEE_SLEEP_DISABLED)
	{
		while(time2sleep > 10)
		{
			PWR.sleep(WTD_8S, ALL_OFF);
			PWR.sleep(WTD_2S, ALL_OFF);
			time2sleep -= 10;
				#ifdef SLEEP_DEBUG
					count+=10;
				#endif
		}
		
		while(time2sleep > 1)
		{
			RTC.ON();
			RTCUt.getTime();
			if(RTC.second%60 == 0)
				break;
			PWR.sleep(WTD_1S, ALL_OFF);
			time2sleep -= 1;	
				#ifdef SLEEP_DEBUG
					count+=1;
				#endif
		}
		xbeeZB.ON();
	}
	else	/// with ZigBee sleep
	{
		while(time2sleep > 10)
		{
			PWR.sleep(WTD_8S, SENS_OFF | UART1_OFF | BAT_OFF | RTC_OFF);
			PWR.sleep(WTD_2S, SENS_OFF | UART1_OFF | BAT_OFF | RTC_OFF);
			time2sleep -= 10;
				#ifdef SLEEP_DEBUG
					count+=10;
				#endif
		}
		
		while(time2sleep > 1)
		{
			RTC.ON();
			RTCUt.getTime();
			if(RTC.second%60 == 0)
				break;
			PWR.sleep(WTD_1S, SENS_OFF | UART1_OFF | BAT_OFF | RTC_OFF);
			time2sleep -= 1;
				#ifdef SLEEP_DEBUG
					count+=1;
				#endif
		}
		xbeeZB.wake();		
	}
		#ifdef SLEEP_DEBUG
			USB.begin();
			USB.print("\n\n\n-");  for(uint8_t i=0; i<60; i++){  USB.print("-"); } 
			USB.print("\nslept for "); USB.println( (int) count );
		#endif
}


/// ///////////////////////////////////////////////////////////////
///  WEATHER STATION - AGRICULTURE SENSOR BOARD V2  
/// ///////////////////////////////////////////////////////////////
#else

//! This function REQUIRES that the correct time to sleep (absolute for hibernate and deep sleep)
//! or sleep time (offset for sleep) is known in either RTCUt.nextTime2WakeUpChar (hibernate) or in
//! time2wValuesArray[posArray] (sleep/deep). 
void PowerUtils::enterLowPowerModeWeatherStation(XBeeSleepMode xbs) ///AT END OF LOOP()
{
	/// SleepMode = DEEPSLEEP (from the moment we implement hibernate we can no longer re-enable pluviometer interrupts
			#ifdef WASPMOTE_WEATHER_SLEEP_DEBUG
				USB.print("\nentering weather deepsleep at "); 
				USB.print(RTC.getTime());
				USB.print("till "); 
				USB.print( RTCUt.nextTime2WakeUpChar );
			#endif
			
	if( !RTCUt.isStillNextTimeToWakeUp() )
	{
		USB.print("\n!!SLEEP TIME EXCEEDED!!\n");
		
		/// SEND ERROR TO GATEWAY 
		COMM.sendError(SLEEP_TIME_EXCEEDED);
		
		skipThisTime2Wake(DEEPSLEEP);
	}		
	
	USB.print("\n!!SLEEP TIME OK!!\n");
	if(xbs == XBEE_SLEEP_DISABLED)
	{
		//Put the mote to sleep with pluviometer interruptions enabled
		SensorAgrV20.sleepAgr(RTCUt.nextTime2WakeUpChar, RTC_ABSOLUTE, RTC_ALM1_MODE3, UART0_OFF | UART1_OFF | BAT_OFF | RTC_OFF, SENS_AGR_PLUVIOMETER);
			
		xbeeZB.ON();
	}
	else
	{
		//Put the mote to sleep with pluviometer interruptions enabled and ZigBee sleep
		SensorAgrV20.sleepAgr(RTCUt.nextTime2WakeUpChar, RTC_ABSOLUTE, RTC_ALM1_MODE3, UART1_OFF | BAT_OFF | RTC_OFF, SENS_AGR_PLUVIOMETER);
		xbeeZB.wake();
		//xbeeZB.wake();
	}
	
	//Detach pluviometer interruptions
	SensorAgrV20.detachPluvioInt();
	
	RTC.ON();
	RTCUt.getTime();
	RTC.setMode(RTC_OFF,RTC_NORMAL_MODE);  //this will save power


	//In case a pluviometer interruption arrived
	if(intFlag & PLV_INT)
	{			
			#ifdef WASPMOTE_WEATHER_SLEEP_DEBUG
				USB.print("\nRain INTERRUPT received at "); USB.println(RTC.getTime());
			#endif
		SensUtils.rainfall_ISR();
				
		// Clearing the interruption flag before coming back to sleep
		clearIntFlag();
		enterLowPowerModeWeatherStation(xbs);
	}
	
	//In case an RTC interruption arrived
	else if(intFlag & RTC_INT)
	{
		USB.begin();
			#ifdef FINAL_USB_DEBUG
				USB.print("\n\n\n-");  for(uint8_t i=0; i<60; i++){  USB.print("-"); }
			#endif 		
			#ifdef WASPMOTE_WEATHER_SLEEP_DEBUG
				USB.print("\nAwake at "); USB.println(RTC.getTime());
			#endif			
			
		if(xbeeZB.defaultOperation)
		{
			RTCUt.setNextTimeWhenToWakeUpViaOffset(xbeeZB.defaultTime2WakeInt);
		}
		else
		{
				#ifdef DEEPSLEEP_DEBUG
					USB.print("out of sleep, posInArray = ");
					USB.println( (int) posInArray );
					USB.print("next t2s = ");
					USB.println( (int) time2wValuesArray[posInArray]);
					USB.println( (int) time2wValuesArray[posInArray + 1]);
					USB.println( (int) (time2wValuesArray[posInArray + 1] - time2wValuesArray[posInArray]) );
				#endif
			xbeeZB.findNextTime2Wake(DEEPSLEEP);
			xbeeZB.updatePosInArray();	
		}
		
		// Clearing the interruption flag before coming back to sleep
		clearIntFlag();
		/// GOTO "device enters loop"
	}
}
#endif


void PowerUtils::skipThisTime2Wake(SleepMode sm)
{
	switch(sm)
	{
		case	HIBERNATE : 
					if(xbeeZB.defaultOperation)
						RTCUt.setNextTimeWhenToWakeUpViaOffset(xbeeZB.defaultTime2WakeInt);
					else
						xbeeZB.findNextTime2Wake(HIBERNATE);
				break;
		
		case	DEEPSLEEP :
					if(xbeeZB.defaultOperation)
					{
						RTCUt.setNextTimeWhenToWakeUpViaOffset(xbeeZB.defaultTime2WakeInt);
					}
					else
					{
							//#ifdef DEEPSLEEP_DEBUG
								USB.print("out of sleep, posInArray = ");
								USB.println( (int) xbeeZB.posInArray );
								USB.print("next t2s = ");
								USB.println( (int) xbeeZB.time2wValuesArray[xbeeZB.posInArray]);
								USB.println( (int) xbeeZB.time2wValuesArray[xbeeZB.posInArray + 1]);
								USB.println( (int) (xbeeZB.time2wValuesArray[xbeeZB.posInArray + 1] - xbeeZB.time2wValuesArray[xbeeZB.posInArray]) );
							//#endif
						xbeeZB.findNextTime2Wake(DEEPSLEEP);
						xbeeZB.updatePosInArray();	
					}		
				break;
				
		case	SLEEP	  : 
					/// handled in sleepTillNextTime2Wake(xbs)
				break;
	}
}


PowerUtils PWRUt = PowerUtils();



