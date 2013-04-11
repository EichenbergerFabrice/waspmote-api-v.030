/* ==========================================================================
 *
 *			THESIS: Design of a Wireless Sensor Networking test-bed
 * 
 * ==========================================================================
 *
 *       Filename:  CommUtils.cpp
 *    Description:  Extra functionality for setting up the ZigBee network
 *					and sending packets.
 *
 * ======================================================================= */
 
#ifndef __WPROGRAM_H__
	#include "BjornClasses.h"
	#include "WaspClasses.h"
#endif

#include <inttypes.h>

#ifdef COMM_DEBUG
	void CommUtils::testPrinting()
	{
		USB.print("testCommUtils\n");
	}
#endif

uint8_t CommUtils::setupXBee()
{
	uint8_t error = 2;
	
  ///////////////////////////////
  // A. XBEE
  ///////////////////////////////
	
	xbeeZB.init(ZIGBEE,FREQ2_4G,NORMAL);
	xbeeZB.ON();
	xbeeZB.wake();  // Must do this for end devices to work
	delay(1000);

	// 1.1 Set PANID: 0x0000000000001302 
		if(!xbeeZB.setPAN(xbeeZB.panid))
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setPAN ok");
			#endif
		}
		else 
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setPAN error");
			#endif 
		}
	// 1.2 set all possible channels to scan, channels from 0x0B to 0x18 (0x19 and 0x1A are excluded)
		if(!xbeeZB.setScanningChannels(0x3F,0xFF))
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setScanningChannels ok");
			#endif
		}
		else 
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setScanningChannels error");
			#endif
		}
	// 1.3 It sets the time the Energy Scan will be performed
		if(!xbeeZB.setDurationEnergyChannels(3)) 
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setDurationEnergyChannels ok");
			#endif
		}
		else
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setDurationEnergyChannels error");
			#endif
		}
	// 1.4 Set channel verification JV=1 in order to make the XBee module to scan new coordinator
		if(!xbeeZB.setChannelVerification(1))
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setChannelVerification ok");
			#endif
		}
		else
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setChannelVerification error");
			#endif
		}
	
	// 1.5 Write values to XBEE memory	
		xbeeZB.writeValues();	
	
	// 1.6 Reboot the XBee module 
		xbeeZB.OFF(); 
		delay(3000); 
		xbeeZB.ON(); 
		xbeeZB.wake();  // For end devices: SM = 1!
		//delay(3000); 
	
	
	// 2. WAIT for association:
		if( !checkNodeAssociation(SETUP) )
		{
			error = 0;
			#ifdef ASSOCIATION_DEBUG
				USB.println("checkNodeAssociation() ok");
			#endif
		}
		else
		{
			error = 1;
			#ifdef ASSOCIATION_DEBUG
				USB.println("ERROR CHECKING NODE ASSOCIATION");  
			#endif
		}
		
  ///////////////////////////////
  // B. RTC
  ///////////////////////////////
	
	// Powers RTC up, init I2C bus and read initial values
	RTC.ON();	

	// Saving battery (!!!in combination with Hibernate, RTC may only be used to set
	// the wake! Other usage can cause internal collisions!!!)
	RTC.setMode(RTC_OFF,RTC_NORMAL_MODE); 
		
		#ifdef NODE_MEMORY_DEBUG
			USB.print("FREE MEM"); USB.println(freeMemory());
		#endif
	  
	return error;
}


uint8_t CommUtils::setupXBee(uint8_t pan[8], uint8_t gateway[8], char * nodeID)
{
	uint8_t error = 2;

  ///////////////////////////////
  // A. XBEE
  ///////////////////////////////
  
	xbeeZB.init(ZIGBEE,FREQ2_4G,NORMAL);
	xbeeZB.ON();
	xbeeZB.wake();  // Must do this for end devices to work
	delay(1000);

	// 1.1 Set PANID: 
		if(!xbeeZB.setPAN(pan))
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setPAN ok");
			#endif
		}
		else 
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setPAN error");
			#endif 
		}
	// 1.2 set all possible channels to scan, channels from 0x0B to 0x18 (0x19 and 0x1A are excluded)
		if(!xbeeZB.setScanningChannels(0x3F,0xFF))
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setScanningChannels ok");
			#endif
		}
		else 
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setScanningChannels error");
			#endif
		}
	// 1.3 It sets the time the Energy Scan will be performed
		if(!xbeeZB.setDurationEnergyChannels(3)) 
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setDurationEnergyChannels ok");
			#endif
		}
		else
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setDurationEnergyChannels error");
			#endif
		}
	// 1.4 Set channel verification JV=1 in order to make the XBee module to scan new coordinator
		if(!xbeeZB.setChannelVerification(1))
		{
			error = 0;
			#ifdef COMM_DEBUG
				USB.println("setChannelVerification ok");
			#endif
		}
		else
		{
			error = 1;
			#ifdef COMM_DEBUG
				USB.println("setChannelVerification error");
			#endif
		}
	
	// 1.5 Set node_ID
		xbeeZB.setNodeIdentifier(nodeID);
	
	// 1.6 Write values to XBEE memory	
		xbeeZB.writeValues();	
	
	// 1.7 Reboot the XBee module 
		xbeeZB.OFF(); 
		delay(3000); 
		xbeeZB.ON(); 
		xbeeZB.wake();  // For end devices: SM = 1!
		//delay(3000); 
	
	
	// 2. WAIT for association:
		if( !checkNodeAssociation(SETUP) )
		{
			error = 0;
			#ifdef ASSOCIATION_DEBUG
				USB.println("checkNodeAssociation() ok");
			#endif
		}
		else
		{
			error = 1;
			#ifdef ASSOCIATION_DEBUG
				USB.println("ERROR CHECKING NODE ASSOCIATION");  
			#endif
		}
		
  ///////////////////////////////
  // B. RTC
  ///////////////////////////////
	
	// Powers RTC up, init I2C bus and read initial values
	RTC.ON();	
	
	// Saving battery (!!!in combination with Hibernate, RTC may only be used to set
	// the wake! Other usage can cause internal collisions!!!)
	RTC.setMode(RTC_OFF,RTC_NORMAL_MODE); 
	
	  #ifdef NODE_MEMORY_DEBUG
			USB.print("FREE MEM "); USB.println(freeMemory());
	  #endif

  ///////////////////////////////
  // C. STORE DEFAULT GATEWAY
  ///////////////////////////////
	
	xbeeZB.setGatewayMacAddress(gateway);
	
    return error;
}


uint8_t CommUtils::checkNodeAssociation(AssociationMode mode)
{
	uint8_t error = 2;
	long previous = millis();
	
	 xbeeZB.getAssociationIndication();
	//if(xbeeZB.getAssociationIndication()){
		while( xbeeZB.associationIndication != 0 && (millis()-previous) < 20000)
		{
			#ifdef ASSOCIATION_DEBUG
				USB.println("\n\n-----> not associated <----------");
				printCurrentNetworkParams();
				Utils.setLED(LED0, LED_ON);  
				delay(300);
				Utils.setLED(LED0, LED_OFF);
				delay(300);
				Utils.setLED(LED0, LED_ON);   
				delay(300);
				Utils.setLED(LED0, LED_OFF);
			#endif
			
			//! After a full setup the XBee will connect faster with this delay. 
			//! In normal operation mode this is just a waste of time.
			if( mode == SETUP )
				delay(6000);
				
			xbeeZB.getAssociationIndication();
			
			#ifdef ASSOCIATION_DEBUG_EXTENDED
				printAssociationState();	
			#endif
		}
		
			#ifdef ASSOCIATION_DEBUG
				printCurrentNetworkParams();
			#endif
		
		if(!xbeeZB.associationIndication) 
		{
			xbeeZB.getExtendedPAN();
			if( xbeeZB.extendedPAN[7] == 0 )
			{
				error = 1;
				USB.println("\nNo XBee detected on Waspmote");
			}
			else
			{
				error =0;
			
				xbeeZB.setChannelVerification(0);
				xbeeZB.writeValues();
				
				#ifdef ASSOCIATION_DEBUG
					USB.println("\n\nSuccessfully joined a coordinator or router!"); 
					
					Utils.setLED(LED1, LED_ON);   // If joined, lighten green LED
					delay(2000);
					Utils.setLED(LED1, LED_OFF);
					
					printCurrentNetworkParams();
				#endif
			}
		}
		else
		{
			error = 2;
			#ifdef ASSOCIATION_DEBUG
				USB.println("Failed to join a network!");
				
				Utils.setLED(LED0, LED_ON);   // If failed, blink red LED twice fast
				delay(300);
				Utils.setLED(LED0, LED_OFF);
				delay(300);
				Utils.setLED(LED0, LED_ON);   
				delay(300);
				Utils.setLED(LED0, LED_OFF);
			#endif
		}
		
		#ifdef 	ASSOCIATION_DEBUG
		USB.print("\nerror: ");
		USB.println( (int) error);
		#endif
		
	return error;
	//}
	/*else
	{
	
		GENERATES AN ERROR ANYWAY, RETURN VALUE IS NOT CORRECT!
	
		error = 1;
		#ifdef COMM_DEBUG
			USB.println("getAssociationIndication error");
		#endif	
	}*/
}
	

void CommUtils::printCurrentNetworkParams()
{
      USB.print("operatingPAN: ");            // get operating PAN ID 
      xbeeZB.getOperatingPAN();
      USB.print(xbeeZB.operatingPAN[0],HEX);
      USB.println(xbeeZB.operatingPAN[1],HEX);
    
      USB.print("extendedPAN: ");              // get operating 64-b PAN ID 
      xbeeZB.getExtendedPAN();
      USB.print(xbeeZB.extendedPAN[0],HEX);
      USB.print(xbeeZB.extendedPAN[1],HEX);
      USB.print(xbeeZB.extendedPAN[2],HEX);
      USB.print(xbeeZB.extendedPAN[3],HEX);
      USB.print(xbeeZB.extendedPAN[4],HEX);
      USB.print(xbeeZB.extendedPAN[5],HEX);
      USB.print(xbeeZB.extendedPAN[6],HEX);
      USB.println(xbeeZB.extendedPAN[7],HEX);
    
      USB.print("channel: ");
      xbeeZB.getChannel();
      USB.println(xbeeZB.channel,HEX);  
}


#ifdef ASSOCIATION_DEBUG_EXTENDED
	void CommUtils::printAssociationState()
	{
	  switch(xbeeZB.associationIndication)
	  {
		case 0x00  :  USB.println("Successfully formed or joined a network");
					  break;
		case 0x21  :  USB.println("Scan found no PANs");
					  break;   
		case 0x22  :  USB.println("Scan found no valid PANs based on current SC and ID settings");
					  break;   
		case 0x23  :  USB.println("Valid Coordinator or Routers found, but they are not allowing joining (NJ expired)");
					  break;   
		case 0x24  :  USB.println("No joinable beacons were found");
					  break;   
		case 0x25  :  USB.println("Unexpected state, node should not be attempting to join at this time");
					  break;
		case 0x27  :  USB.println("Node Joining attempt failed");
					  break;
		case 0x2A  :  USB.println("Coordinator Start attempt failed");
					  break;
		case 0x2B  :  USB.println("Checking for an existing coordinator");
					  break;
		case 0x2C  :  USB.println("Attempt to leave the network failed");
					  break;
		case 0xAB  :  USB.println("Attempted to join a device that did not respond.");
					  break;
		case 0xAC  :  USB.println("Secure join error  :  network security key received unsecured");
					  break;
		case 0xAD  :  USB.println("Secure join error  :  network security key not received");
					  break;
		case 0xAF  :  USB.println("Secure join error  :  joining device does not have the right preconfigured link key");
					  break;
		case 0xFF  :  USB.println("Scanning for a ZigBee network (routers and end devices)");
					  break;
		default    :  USB.println("Unkown associationIndication");
					  break;
	  }
	}
#endif

uint8_t CommUtils::receiveMessages()
{
	uint8_t error = 2;

	long previous = 0;
    previous=millis();
	
	//uint8_t temp = 0;
	
		#ifdef RECEIVE_DEBUG
			USB.println("checking for received messages");
		#endif
		#ifdef NODE_MEMORY_DEBUG
			//USB.println("receivedMessages");
			USB.print("FREE MEM"); USB.println(freeMemory());
		#endif
	
	//Waiting ... sec on a message
	while( (millis()-previous) < 20000 )
	{
		  if( XBee.available() )
		  {
				error = 1;
				xbeeZB.treatData();
				
				#ifdef COMM_DEBUG
					USB.print("start printing xbeeZB.error_RX:");
					USB.println(xbeeZB.error_RX);
				#endif
				
				if( !xbeeZB.error_RX )
				{
					//read ALL available packets (pos = 0 -> no packets available)
					while( xbeeZB.pos > 0 )
					{	
						error = 0;
						// Available information in 'xbeeZB.packet_finished' structure
						// HERE it should be introduced the User's packet treatment        
						// For example: show DATA field:
						
						#ifdef RECEIVE_DEBUG
							USB.print("\nData: ");
							for(int f=0;f<xbeeZB.packet_finished[xbeeZB.pos-1]->data_length;f++)
							{
								receivedData[f] = xbeeZB.packet_finished[xbeeZB.pos-1]->data[f];
									USB.print(xbeeZB.packet_finished[xbeeZB.pos-1]->data[f],BYTE);
									
							}
							USB.print("\n\n");
							USB.print(xbeeZB.packet_finished[xbeeZB.pos-1]->data[0],BYTE);
							USB.print(xbeeZB.packet_finished[xbeeZB.pos-1]->data[1],BYTE);
							USB.print( (char) xbeeZB.packet_finished[xbeeZB.pos-1]->data[0]);
							USB.print( (char) xbeeZB.packet_finished[xbeeZB.pos-1]->data[1]);
						#endif  
						
						#ifdef NODE_MEMORY_DEBUG
							USB.print("FREE MEM"); USB.println(freeMemory());
						#endif
						
						#ifdef RECEIVE_DEBUG
							USB.print("ID = "); USB.println( (int) xbeeZB.packet_finished[xbeeZB.pos-1]->packetID);
						#endif
						
						(*myTreatPacket[xbeeZB.packet_finished[xbeeZB.pos-1]->packetID])
							(xbeeZB.packet_finished[xbeeZB.pos-1]);
						
						
						#ifdef RECEIVE_DEBUG_2
							USB.print("receivedData(MAX_DATA) = ");
							USB.println(receivedData);
						#endif

						// AFTER THE DATA IS TREATED BY OWN FUNCTIONS:
						// FREE MEM
						free(xbeeZB.packet_finished[xbeeZB.pos-1]); 
						
						// FREE POINTER
						xbeeZB.packet_finished[xbeeZB.pos-1]=NULL; 
						
						xbeeZB.pos--;
					}
					break;
				}
		  }
		  else
		  {
				error = 1;
				#ifdef RECEIVE_DEBUG_2
					USB.println("XBEE not available\r\n");
				#endif
		  }   
	}

	return error;
}


uint8_t CommUtils::sendMessage(uint8_t * destination, const char * message)
{
      uint8_t error = 2;
	  packetXBee * paq_sent;
      paq_sent = (packetXBee*) calloc(1,sizeof(packetXBee)); 
      paq_sent->mode=UNICAST;
      paq_sent->MY_known=0;
      paq_sent->packetID = SEND_ERROR;  //APPLICATION_ID = 12
      paq_sent->opt=0; 
      xbeeZB.hops=0;
	  
      xbeeZB.setOriginParams(paq_sent, MY_TYPE);
      xbeeZB.setDestinationParams(paq_sent, destination, message, MAC_TYPE, DATA_ABSOLUTE);
      xbeeZB.sendXBee(paq_sent);
	  
	  #ifdef COMM_DEBUG
		  USB.print("start printing xbeeZB.error_TX:");
		  USB.println(xbeeZB.error_TX);
	  #endif
      if( !xbeeZB.error_TX )
      {
			#ifdef COMM_DEBUG
				USB.println("End device sends out a challenge ok");
				Utils.setLED(LED1, LED_ON);   // If transmission OK, blink green LED
				delay(500);
				Utils.setLED(LED1, LED_OFF);
			#endif
			error = 0;
      }
      else
      { 
			error = xbeeZB.error_TX;
	  		#ifdef COMM_DEBUG
		        USB.println("\nchallenge transmission error\n");
				Utils.setLED(LED0, LED_ON);   // If transmission error, blink red LED
				delay(500);
				Utils.setLED(LED0, LED_OFF);
			#endif
      }
	  
      free(paq_sent);
      paq_sent=NULL;
      return error;
}


uint8_t CommUtils::sendMessage(uint8_t * destination, uint8_t type, const char * message)
{
	#ifdef COMM_DEBUG
		USB.print("strlen in sendMessage: "); USB.print( strlen(message) );
		USB.print("\n");
		USB.println("sendMessage: data in const char * message = "); 
		for(int j=0; j<10; j++)
			USB.println( (int) message[j]);
		USB.print("dest = "); USB.println( (int) destination[1] );
		USB.print("type = "); USB.println( (int) type );	
	#endif COMM_DEBUG
	
      uint8_t error = 2;
	  packetXBee * paq_sent;
      paq_sent = (packetXBee*) calloc(1,sizeof(packetXBee)); 
	  
	  #ifdef NODE_MEMORY_DEBUG
			USB.print("FREE MEM"); USB.print("send"); USB.println(freeMemory());
	  #endif
	  
      paq_sent->mode=UNICAST;
      paq_sent->MY_known=0;
      paq_sent->packetID = type;
	  //paq_sent->packetID = 0x52;
      paq_sent->opt=0; 
      xbeeZB.hops=0;
	  
      //xbeeZB.setOriginParams(paq_sent, "5678", MY_TYPE);
	  xbeeZB.setOriginParams(paq_sent, MY_TYPE);
      xbeeZB.setDestinationParams(paq_sent, destination, message, MAC_TYPE, DATA_ABSOLUTE);
	  //xbeeZB.setDestinationParams(paq_sent, "0013A2004069737A", message, MAC_TYPE, DATA_ABSOLUTE);
	  
	  		#ifdef TIME_DEBUG
				USB.print("start sending"); USB.println( millis() );
			#endif
			
      xbeeZB.sendXBee(paq_sent);
	  
	  	  	#ifdef TIME_DEBUG
				USB.print("end sending"); USB.println( millis() );
			#endif
	 
	  #ifdef COMM_DEBUG
	 	  USB.print("start printing xbeeZB.error_TX:");
		  USB.println(xbeeZB.error_TX);
	  #endif
      if( !xbeeZB.error_TX )
      {
			#ifdef COMM_DEBUG
				USB.println("End device sends out a challenge ok");
				Utils.setLED(LED1, LED_ON);   // If transmission OK, blink green LED
				delay(500);
				Utils.setLED(LED1, LED_OFF);
			#endif
			error = 0;
      }
      else
      { 
			error = xbeeZB.error_TX;
	  		#ifdef COMM_DEBUG
		        USB.println("challenge transmission error\n\n");
				Utils.setLED(LED0, LED_ON);   // If transmission error, blink red LED
				delay(500);
				Utils.setLED(LED0, LED_OFF);
			#endif
      }
	  
      free(paq_sent);
      paq_sent=NULL;
	  
			#ifdef NODE_MEMORY_DEBUG
			USB.print("FREE MEM"); USB.print("send"); USB.println(freeMemory());
			#endif
	  
      return error;
}

CommUtils COMM = CommUtils();
