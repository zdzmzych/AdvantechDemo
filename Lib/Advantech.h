//==============================================================================
//
// Title:       AdvLib
// Purpose:     A short description of the interface.
//
// Created on:  02.11.2020 at 11:52:10 by mzych.
// Copyright:   Free
//
//==============================================================================

#ifndef __Advantech_H__
#define __Advantech_H__

#ifdef __cplusplus
    extern "C" {
#endif

#define ADV_MAX_DIO_PORT_COUNT 32
#define ADV_MAX_DEVICE_DESC_LEN 128		

typedef unsigned char  uint8;
typedef enum AdvErrorCode
{
	AdvErrorCode_Success = 0,
	AdvErrorCode_BIODAQNOTFOUND = 0xF0000001,
	AdvErrorCode_NullHandle = 0xF0000002,
	AdvErrorCode_ErrorUndefined = 0xFFFFFFFF,
}AdvErrorCode;

typedef enum AdvPortType {
   AdvPortType_PortDi = 0,        /*the port number references to a DI port*/
   AdvPortType_PortDo,            /*the port number references to a DO port*/
   AdvPortType_PortDio,           /*the port number references to a DI port and a DO port*/
   AdvPortType_Port8255A,         /*the port number references to a PPI port A mode DIO port.*/
   AdvPortType_Port8255C,         /*the port number references to a PPI port C mode DIO port.*/
   AdvPortType_PortIndividual,     /*the port number references to a port whose each channel can be configured as in or out.*/

   /*----------------------------------------------------------------------*/
   /*Dummy ID, to ensure the type is compiled as 'int' by various compiler */
   AdvPortType_PortUnknown = 0xffffffff,
} AdvPortType;

typedef enum AdvPortDir {
   AdvPortDir_Input   = 0x00,
   AdvPortDir_LoutHin = 0x0F,
   AdvPortDir_LinHout = 0xF0,
   AdvPortDir_Output  = 0xFF,

   /*----------------------------------------------------------------------*/
   /*Dummy ID, to ensure the type is compiled as 'int' by various compiler */
   AdvPortDir_Unknown = 0xffffffff,
} AdvPortDir;

typedef struct AdvHandle 
{
   int		DeviceNumber;
   char     Description[ADV_MAX_DEVICE_DESC_LEN];
   void* 	DeviceCtrl;
   void*	DiCtrl;
   void*	DoCtrl;
} AdvHandle;

//list
AdvErrorCode AdvListDevices(AdvHandle devices[], int devicesLen, int* count);

//info
AdvErrorCode AdvGetProductId(AdvHandle* handle, int* productId);
AdvErrorCode AdvGetBoardId(AdvHandle* handle, int* boardId);
AdvErrorCode AdvSetBoardId(AdvHandle* handle, int boardId);
AdvErrorCode AdvGetBoardVersion(AdvHandle* handle, char *str, int length);
AdvErrorCode AdvGetDriverVersion(AdvHandle* handle, char *str, int length);
AdvErrorCode AdvGetDllVersion(AdvHandle* handle, char *str, int length);
AdvErrorCode AdvGetLocation(AdvHandle* handle, char *str, int length);
void AdvGetErrorText(AdvErrorCode code, char* str, int length);

//device
AdvErrorCode AdvOpen(AdvHandle* handle);
AdvErrorCode AdvOpenWithDeviceNumber(AdvHandle* handle, int deviceNumber);
AdvErrorCode AdvClose(AdvHandle* handle);
AdvErrorCode AdvWriteAny(AdvHandle* handle, int portStart, int portCount, uint8 data[]);               
AdvErrorCode AdvReadAny(AdvHandle* handle, int portStart, int portCount, uint8 data[]);                
AdvErrorCode AdvWriteBit(AdvHandle* handle, int port, int bit, uint8 data);                            
AdvErrorCode AdvReadBit(AdvHandle* handle, int port, int bit, uint8* data); 

//device configuration
AdvErrorCode AdvGetPortsTypes(AdvHandle* handle, AdvPortType portsTypes[], int maxCount, int* portsCount);
AdvErrorCode AdvGetPortsDirections(AdvHandle* handle, AdvPortDir portsDirs[], int maxCount);
AdvErrorCode AdvSetPortDirection(AdvHandle* handle, AdvPortDir portDirs[], int portsCount);
int AdvGetChannelState(uint8 port, int channel);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __AdvLib_H__ */

