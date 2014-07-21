/**
 *******************************************************************************
 * @file    OneWire.c
 * @author  Vojtěch Vigner <vojtech.vigner@gmail.com>
 * @version V1.0.5
 * @date    12-February-2013
 * @brief   Provides 1-Wire bus support for STM32Fxxx devices.
 * 
 * @section info Additional Information
 *          This library provides functions to manage the following 
 *          functionalities of the 1-Wire bus from MAXIM:           
 *              - Initialization and configuration.
 *              - Low level communication functions.
 *              - 1-Wire specific CRC calculation.
 *              - Device address operations.
 *              - Parasite powered device support.
 *              - 1-Wire advanced device search, based on MAXIM App. Note 126.
 * 
 *          Library requires one USART for communication with 1-Wire devices. 
 *          Current implementation used Half Duplex USART mode. This means that
 *          only one pin is used for communication.  
 *          
 *          Currently supports and has been tested on STM32F2xx and STM32F4xx
 *          devices. 
 * 
 * @section howto How to use this library
 *          1. Modify hardware specific section in OneWire.h file according to
 *          your HW. Decide if you will be using parasite powered device/s and
 *          enable or disable this support.
 *  
 *          2. Initialize bus using OW_Init().
 * 
 *          3. Now you can use all communication functions. 
 * 
 *          4. See Example_OneWire.c for simple example. 
 *  
 * @copyright The BSD 3-Clause License. 
 * 
 * @section License
 *          Copyright (c) 2013, Vojtěch Vigner <vojtech.vigner@gmail.com> 
 *           
 *          All rights reserved.
 * 
 *          Redistribution and use in source and binary forms, with or without 
 *          modification, are permitted provided that the following conditions
 *          are met:
 *              - Redistributions of source code must retain the above 
 *                copyright notice, this list of conditions and the following 
 *                disclaimer.
 *              - Redistributions in binary form must reproduce the above 
 *                copyright notice, this list of conditions and the following
 *                disclaimer in the documentation and/or other materials
 *                provided with the distribution.
 *              - The name of the contributors can not be used to endorse or 
 *                promote products derived from this software without specific
 *                prior written permission.
 *
 *          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *          "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *          LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *          FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
 *          COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *          INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *          BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *          CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 *          LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 *          ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *          POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************
 */

#include "OneWire.h"



/* Search related variables */
static struct {
    uint8_t iLastDeviceFlag;
    uint8_t iLastDiscrepancy;
    uint8_t iLastFamilyDiscrepancy;
    uint64_t ROM;
} stSearch;



/* CRC calculation table */
static uint8_t CRCTable[] = {
    0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
    157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
    35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
    190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
    70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
    219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
    101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
    248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
    140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
    17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
    175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
    50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
    202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
    87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
    233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
    116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

/**
 * Setup the search to skip the current device type on the next call of 
 * OW_SearchNext function.
 */
void OW_FamilySkipSetup(void) {
    /* Set the last discrepancy to last family discrepancy */
    stSearch.iLastDiscrepancy = stSearch.iLastFamilyDiscrepancy;
    stSearch.iLastFamilyDiscrepancy = 0;

    /* Check for end of list */
    if (stSearch.iLastDiscrepancy == 0) stSearch.iLastDeviceFlag = 1;
}

/**
 * Calculate the 1-Wire specific CRC.
 * @param iCRC Input CRC value.
 * @param iValue Value to be added to CRC.
 * @return Resulting CRC value.
 */
uint8_t OW_CRCCalculate(uint8_t iCRC, uint8_t iValue) {
    return CRCTable[iCRC ^ iValue];
}

/**
 * Find the 'first' devices on the 1-Wire bus.
 * @param iFamilyCode Select family code filter or 0 for all. 
 * @return 64-bit device address or 0 if no device found.
 */
uint64_t OW_SearchFirst(uint8_t iFamilyCode) {
    if (iFamilyCode) {
        stSearch.ROM = (uint64_t) iFamilyCode;

        stSearch.iLastDiscrepancy = 64;
        stSearch.iLastFamilyDiscrepancy = 0;
        stSearch.iLastDeviceFlag = 1;
    } else {
        stSearch.ROM = 0;
        stSearch.iLastDiscrepancy = 0;
        stSearch.iLastDeviceFlag = 0;
        stSearch.iLastFamilyDiscrepancy = 0;
    }

    return OW_SearchNext();
}

/**
 * Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
 * search state.
 * @return 64-bit device address or 0 if no device found.
 */
uint64_t OW_SearchNext(void) {
    uint8_t iSearchDirection;
    int iIDBit, iCmpIDBit;

    /* Initialize for search */
    uint8_t iROMByteMask = 1;
    uint8_t iCRC = 0;
    int iIDBitNumber = 1;
    int iLastZero = 0;
    int iROMByteNumber = 0;
    int iSearchResult = 0;

    /* If the last call was not the last one */
    if (!stSearch.iLastDeviceFlag) {
        /* 1-Wire reset */
		OW_Reset();
        if (OW_GetResetResult() == OW_NO_DEV) {
        	/* Reset the search */
            stSearch.iLastDiscrepancy = 0;
            stSearch.iLastDeviceFlag = 0;
            stSearch.iLastFamilyDiscrepancy = 0;
            return 0;
        }
        /* Issue the search command */
        OW_ByteWrite(OW_ROM_SEARCH);

        /* Loop to do the search */
        do {

#ifdef OW_USE_PARASITE_POWER

            OW_StrongPullUp();
            __IO int i;
            for (i = 0; i < 0xFFFF; i++);
            OW_WeakPullUp();
#endif

            /* Read a bit and its complement */
            iIDBit = OW_BitRead();
            iCmpIDBit = OW_BitRead();

            /* Check for no devices on 1-wire */
            if ((iIDBit == 1) && (iCmpIDBit == 1))
                break;
            else {
                /* All devices coupled have 0 or 1 */
                if (iIDBit != iCmpIDBit)
                    /* Bit write value for search */
                    iSearchDirection = iIDBit;
                else {
                    /* if this discrepancy if before the Last Discrepancy
                    on a previous next then pick the same as last time */
                    if (iIDBitNumber < stSearch.iLastDiscrepancy)
                        iSearchDirection = ((((uint8_t*) & stSearch.ROM)[iROMByteNumber] & iROMByteMask) > 0);
                    else
                        /* If equal to last pick 1, if not then pick 0 */
                        iSearchDirection = (iIDBitNumber == stSearch.iLastDiscrepancy);

                    /* If 0 was picked then record its position in iLastZero */
                    if (iSearchDirection == 0) {
                        iLastZero = iIDBitNumber;

                        /* Check for Last discrepancy in family */
                        if (iLastZero < 9)
                            stSearch.iLastFamilyDiscrepancy = iLastZero;
                    }
                }

                /* Set or clear the bit in the ROM byte with mask rom_byte_mask */
                if (iSearchDirection == 1)
                    ((uint8_t*) & stSearch.ROM)[iROMByteNumber] |= iROMByteMask;
                else
                    ((uint8_t*) & stSearch.ROM)[iROMByteNumber] &= ~iROMByteMask;

                /* Set serial number search direction */
                OW_BitWrite(iSearchDirection);
                /* Increment the byte counter and shift the mask */
                iIDBitNumber++;
                iROMByteMask <<= 1;

                /* If the mask is 0 then go to new ROM byte number and reset mask */
                if (iROMByteMask == 0) {
                    /* Accumulate the CRC */
                    iCRC = OW_CRCCalculate(iCRC, ((uint8_t*) & stSearch.ROM)[iROMByteNumber]);
                    iROMByteNumber++;
                    iROMByteMask = 1;
                }
            }
        } while (iROMByteNumber < 8); /* Loop until through all ROM bytes 0-7 */

        /* If the search was successful then */
        if (!((iIDBitNumber < 65) || (iCRC != 0))) {
            stSearch.iLastDiscrepancy = iLastZero;

            /* Check for last device */
            if (stSearch.iLastDiscrepancy == 0)
                stSearch.iLastDeviceFlag = 1;

            iSearchResult = 1;
        }
    }

    /* If no device found then reset counters so next 'search' will be like a first */
    if (!iSearchResult || !((uint8_t*) & stSearch.ROM)[0]) {
        stSearch.iLastDiscrepancy = 0;
        stSearch.iLastDeviceFlag = 0;
        stSearch.iLastFamilyDiscrepancy = 0;
        return 0;
    }

    return stSearch.ROM;
}

/**
 * Read ROM address of device, works only for one device on the bus.
 * @return 64-bit device address.
 */
uint64_t OW_ROMRead(void) {
    uint64_t iRes = 0;
    int i;

    if (OW_Reset() == OW_NO_DEV)
    	return 0;

    OW_ByteWrite(OW_ROM_READ);
    for (i = 0; i < 8; i++){
        ((uint8_t*) & iRes)[i] = OW_ByteRead();
    printf("ID %d\n",((uint8_t*) & iRes)[i] );
    }
    return iRes;
}

/**
 * Issue ROM match command.
 * @param iAddress 64-bit device address.
 * @return OW_OK if device is present or OW_NO_DEV if not.
 */

static void (*RM_callback)(void);  //rommatch callback  pointer
 
static volatile int state_ROMMatch;
static volatile int result_ROMMatch;
static volatile uint64_t Address_i;
 
void OW_ROMMatch(uint64_t iAddress, void (*callback)(void))
{
	state_ROMMatch = 0;
	Address_i=iAddress;
	RM_callback = callback;
	OW_Reset_As(CB_ROMMatch);
}

void CB_ROMMatch(void)
{
	static volatile int i = 0;
	if(state_ROMMatch == 0){
		state_ROMMatch = 1;
		if(OW_GetResetResult() == OW_NO_DEV){
			printf("NO Device or Bus Error\n");
		}else{
			if(Address_i == OW_ADDRESS_ALL){
				OW_ByteWrite_As(OW_ROM_SKIP, CB_ROMMatch);
			}else{
				OW_ByteWrite_As(OW_ROM_MATCH, CB_ROMMatch);
				i = 0;
			}
		}
	}else if(state_ROMMatch == 1){
		if(Address_i == OW_ADDRESS_ALL){
			result_ROMMatch = OW_OK;
			RM_callback();
		}else if(i<8){
			OW_ByteWrite_As(((uint8_t*) & Address_i)[i], CB_ROMMatch);
			i++;
		}
		else{
				result_ROMMatch = OW_OK;
				state_ROMMatch = 2;
				RM_callback();
			}
		}
}


OW_State OW_ROMMatch_GetResult(void)
{
	return result_ROMMatch;
}
