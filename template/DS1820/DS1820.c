#include "DS1820.h"
#include "OneWire.h"
#include "stdio.h"

/* DS1820 specific commands */
#define SCRATCHPAD_READ     0xBE
#define SCRATCHPAD_STORE    0x48
#define SCRATCHPAD_WRITE    0x4E
#define SCRATCHPAD_RECALL   0xB8
#define POWER_SUPPLY_READ   0xB4

/* DS1820 scratchpad length in bytes */
#define SCRATCHPAD_LENGTH   9
#define SCRATCHPAD_CRC_POS  (SCRATCHPAD_LENGTH - 1)

/* Internal functions */



static void TemperatureConvert(void);

static int CRC_Right_flag=0;
static float iTemp_buffer=0;

/**
 * Initalizes and resets OneWire communication.
 */
void DS1820_Init(void) {
    OW_Init();
    OW_Reset();
}

/**
 * Initializes temperature measurement on DS1820 chip.
 * @warning This function sets communication pin in StrongPullUp state.
 * @warning The bus has to be in StrongPullUp state at least for 500 ms.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL for all 
 * devices.
 * @return DS1820_OK if successfull, DS1820_ERROR if failed.
 */
DS1820_State DS1820_TemperatureConvert(uint64_t iAddress) {

    /* Ready bus for communcation */
    OW_WeakPullUp();
    OW_ROMMatch(iAddress,TemperatureConvert);

    return DS1820_OK;
}

/**
 * Reads tepmerature from specific device. You have to use TemperatureConvert 
 * function before calling TemperatureGet.
 * @param iAddress 64bit device address, use DS1820_ADDRESS_ALL to skip 
 * address match (only for single device on the bus).
 * @return Temperature in degrees of Celsius * 10 or DS1820_TEMP_ERROR in case 
 * of an error.
 */
 
static volatile int state_TemperatureGet;
 
void DS1820_TemperatureGet(uint64_t iAddress) {
    /* Ready bus for communcation */
    OW_WeakPullUp();
	state_TemperatureGet = 0;
	OW_ROMMatch(iAddress, CB_TemperatureGet);
}


void CB_TemperatureGet(void)
{
	static int i = 0;
	static volatile uint8_t iCRC = 0;
	static uint8_t iSPad[SCRATCHPAD_LENGTH];
	if(state_TemperatureGet == 0){
		if(OW_ROMMatch_GetResult() == OW_NO_DEV){
			return;
		}else if(OW_ROMMatch_GetResult() == OW_OK){
			state_TemperatureGet = 1;
			OW_ByteWrite_As(SCRATCHPAD_READ,CB_TemperatureGet);
		}
	}else if(state_TemperatureGet == 1){
	    state_TemperatureGet = 2;
	    i=0;
		OW_ByteRead_As(CB_TemperatureGet);


	}else if(state_TemperatureGet == 2){
		iSPad[i] = OW_GetByteReadResult();
		iCRC = OW_CRCCalculate(iCRC, iSPad[i]);
		i++;
		if(i < 9){
			OW_ByteRead_As(CB_TemperatureGet);
		}else{
			state_TemperatureGet = 3;
			if(iCRC == 0){
			    CRC_Right_flag=1;
			}else{
			    CRC_Right_flag=0;
			}
			i=0;
			iTemp_buffer = (float)iBinaryToIntTemperature(iSPad)/10;
		}
	}else{
	    i=0;
	}
}

int iBinaryToIntTemperature(uint8_t *iSPad) {

	//most and least significant bits
	uint8_t lsb = iSPad[0];
	uint8_t msb = iSPad[1];
	//integer part
    int temperature  = ((lsb >> 4) | ((msb << 4) )) & 0b01111111;

    //negative part
    if ( (( msb >> 3 ) & 0b1 ) == 1 )
    	temperature = (~temperature & 0b01111111) * -1;

    //avoid decimal
    temperature *= 10;

    /* "Decimal" part */
    lsb &= 0xF;
    temperature += (lsb == 0) ? 0 : (( lsb ) * 10 / 16) ;

    return temperature;
}
float DS1820_TemperatureResult(uint64_t iAddress){
    static volatile float temp_last = 0;
    static volatile float temp = 0;
    static volatile int t=0;
    if(CRC_Right_flag == 1){
    	if(t>0){
    		if(iTemp_buffer-temp_last>5||temp_last-iTemp_buffer>5){
    			printf("the temperature changs too much:%f and %f \n",temp_last,iTemp_buffer);
    		}
    	}
         t=1;
	     temp=iTemp_buffer;
	}else{
		     printf("CRC is wrong:%f\n",iTemp_buffer);
		     temp=temp_last;  //right temperature
		 }
    temp_last=temp;
    return temp;
}

/**
 * Function searches for DS1820 devices on the bus and stores them in to array.
 * @param Addresses Pointer to array for device addresses to be stored. 
 * @param iMaxDevices Maximum of devices to be searched.
 * @return Number of devices found.
 */
int DS1820_Search(uint64_t *Addresses, int iMaxDevices) {
    int iCount = 0;
    uint64_t iAddress;

    /* Ready bus for communcation */
    OW_WeakPullUp();

    /* Search for first DS1820 device */
    iAddress = OW_SearchFirst(0);
    /* Store all device addresses into a array */
    while ((iAddress) && (iCount < iMaxDevices)) {
        iCount++;
        Addresses[iCount - 1] = iAddress;
        iAddress = OW_SearchNext();
    }

    /* Reset communication */
    OW_Reset();

    return iCount;
}

/**
 * Starts temperature conversion.
 */
void TemperatureConvert(void) {
    OW_ByteWrite_As(0x44,OW_StrongPullUp);
}
