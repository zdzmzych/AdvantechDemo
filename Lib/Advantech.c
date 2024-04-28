#include "bdaqctrl.h"
#include "Advantech.h"  
#include "toolbox.h"

AdvErrorCode AdvListDevices(AdvHandle devices[], int maxcount, int* found)
{
	DeviceTreeNode *p = NULL; 
	
	HMODULE module = DNL_Instance();
	if(module == NULL)
		return AdvErrorCode_BIODAQNOTFOUND;
	
	IArray *arr = DeviceCtrl_getInstalledDevices();
	if(arr != NULL)
	{
		*found = Array_getLength(arr);
		for(int i=0; i<*found && i<maxcount; i++)
		{
			p = Array_getItem(arr, i);
			if(p != NULL)
			{
				devices[i].DeviceNumber = p->DeviceNumber;
				WideCharToMultiByte(CP_ACP, 0, p->Description, -1, devices[i].Description, ADV_MAX_DEVICE_DESC_LEN, 0, 0);				
			}
		}	
	
		Array_Dispose(arr);	
		return Success;
	}
	*found = 0;
	return AdvErrorCode_ErrorUndefined;
}

AdvErrorCode AdvOpen(AdvHandle* handle)
{
	return AdvOpenWithDeviceNumber(handle, handle->DeviceNumber);
}

AdvErrorCode AdvOpenWithDeviceNumber(AdvHandle* handle, int deviceNumber)
{
	ErrorCode code = Success;
	DeviceInformation dinfo;	
	memset(&dinfo, 0x0, sizeof(DeviceInformation));	
	
	HMODULE module = DNL_Instance();
	if(module == NULL)
		return AdvErrorCode_BIODAQNOTFOUND;

	if(handle != NULL)
	{
		dinfo.DeviceNumber = deviceNumber;
		dinfo.DeviceMode = ModeWrite;
		
		//todo check if device supports DI and DO
		handle->DiCtrl = InstantDiCtrl_Create();
		handle->DoCtrl= InstantDoCtrl_Create();
		if(handle->DiCtrl != NULL)
		{
			code = InstantDiCtrl_setSelectedDevice(handle->DiCtrl, &dinfo); 
			if(code != Success)
				return code;
			handle->DeviceCtrl = InstantDiCtrl_getDevice(handle->DiCtrl);
		} 	
		if(handle->DoCtrl != NULL)
		{
			code = InstantDoCtrl_setSelectedDevice(handle->DoCtrl, &dinfo); 
			if(code != Success)
				return code;
			if(handle->DeviceCtrl == NULL)
				handle->DeviceCtrl = InstantDiCtrl_getDevice(handle->DoCtrl);			
		} 		
		
		if(handle->DeviceCtrl != NULL)
		{
			handle->DeviceNumber = DeviceCtrl_getDeviceNumber(handle->DeviceCtrl);
			return Success;
		}
	}
	
	return AdvErrorCode_NullHandle;
}


AdvErrorCode AdvClose(AdvHandle* handle)
{
	if(handle->DiCtrl != NULL)
	{
		InstantDiCtrl_Dispose(handle->DiCtrl);
	} 	
	else if(handle->DoCtrl != NULL)
	{
		InstantDoCtrl_Dispose(handle->DoCtrl);
	} 	
	
	handle->DiCtrl = NULL;
	handle->DoCtrl = NULL;
	handle->DeviceCtrl = NULL;
	return Success;
}


AdvErrorCode AdvGetBoardId(AdvHandle* handle, int* boardId)
{
	if(handle != NULL)
	{
		*boardId = DeviceCtrl_getBoardId(handle->DeviceCtrl);   
		return Success;
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvSetBoardId(AdvHandle* handle, int boardId)
{
	if(handle != NULL)
	{
		return DeviceCtrl_setBoardId(handle->DeviceCtrl, boardId);
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvGetBoardVersion(AdvHandle* handle, char *str, int length)
{
	wchar_t buff[128];
	
	if(handle != NULL)
	{
		ErrorCode code = DeviceCtrl_getBoardVersion(handle->DeviceCtrl, 128, buff);	
		if(code == Success)
		{
			WideCharToMultiByte(CP_ACP, 0, buff, -1, str, length, 0, 0);				
			return Success;
		}
		return code;
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvGetDriverVersion(AdvHandle* handle, char *str, int length)
{
	wchar_t buff[128];
	
	if(handle != NULL)
	{
		ErrorCode code = DeviceCtrl_getDriverVersion(handle->DeviceCtrl, 128, buff);	
		if(code == Success)
		{
			WideCharToMultiByte(CP_ACP, 0, buff, -1, str, length, 0, 0);				
			return Success;
		}
		return code;
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvGetDllVersion(AdvHandle* handle, char *str, int length)
{
	wchar_t buff[128];
	
	if(handle != NULL)
	{
		ErrorCode code = DeviceCtrl_getDllVersion(handle->DeviceCtrl, 128, buff);	
		if(code == Success)
		{
			WideCharToMultiByte(CP_ACP, 0, buff, -1, str, length, 0, 0);				
			return Success;
		}
		return code;
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvGetLocation(AdvHandle* handle, char *str, int length)
{
	wchar_t buff[128];
	
	if(handle != NULL)
	{
		ErrorCode code = DeviceCtrl_getLocation(handle->DeviceCtrl, 128, buff);	
		if(code == Success)
		{
			WideCharToMultiByte(CP_ACP, 0, buff, -1, str, length, 0, 0);				
			return Success;
		}
		return code;
	}
	return AdvErrorCode_NullHandle;
}


AdvErrorCode AdvGetPortsTypes(AdvHandle* handle, AdvPortType portsTypes[], int maxCount, int* portsCount)    
{
	if(handle != NULL)
	{
		DioFeatures* features = InstantDiCtrl_getFeatures(handle->DiCtrl); 
		IArray * types = DioFeatures_getPortsType(features);
		if(types != NULL)
		{
			*portsCount = Array_getLength(types);
			for(int i=0; i<*portsCount && i<maxCount; i++)
			{
				uint8 portType = *((uint8*)Array_getItem(types, i));
				portsTypes[i] = (AdvPortType)portType;
			}
	
			Array_Dispose(types);
			return Success;
		}
		*portsCount = 0;
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvGetPortsDirections(AdvHandle* handle, AdvPortDir portsDirs[], int maxCount)
{
	if(handle->DiCtrl != NULL)
	{
		IArray *ports = InstantDiCtrl_getPorts(handle->DiCtrl);
		if(ports != NULL)
		{
			int count = Array_getLength(ports);
			for(int i=0; i<count && i<maxCount; i++)
			{
				DioPort *port = Array_getItem(ports, i);
				if(port != NULL)
				{
					portsDirs[i] = (AdvPortDir)DioPort_getDirectionMask(port);
				}
			}	
	
			Array_Dispose(ports);
			return Success;
		}
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvSetPortDirection(AdvHandle* handle, AdvPortDir portDirs[], int portsCount)
{
	ErrorCode code = Success;
			
	if(handle->DiCtrl != NULL)
	{
		IArray *ports = InstantDiCtrl_getPorts(handle->DiCtrl);
		if(ports != NULL)
		{
			int count = Array_getLength(ports);
			for(int i=0; i<count; i++)
			{
				DioPort *port = Array_getItem(ports, i);
				if(port != NULL)
				{
					code = DioPort_setDirectionMask(port, portDirs[i]);
				}
			}	
	
			Array_Dispose(ports);
			return code;
		}
	}
	return AdvErrorCode_NullHandle;
}

AdvPortDir AdvGetChannelDir(AdvPortDir dir, int channel)
{
	int val = (dir & (1 << channel)) == 0;
	return val ? AdvPortDir_Input : AdvPortDir_Output;
}

int AdvGetChannelState(uint8 port, int channel)
{
	return (port & (1 << channel)) != 0;
}


AdvErrorCode AdvWriteAny(AdvHandle* handle, int portStart, int portCount, uint8 data[])
{
	if(handle != NULL && handle->DoCtrl != NULL)
	{
		return InstantDoCtrl_WriteAny(handle->DoCtrl, portStart, portCount, data);  	
	}
	return AdvErrorCode_NullHandle;	
}

AdvErrorCode AdvReadAny(AdvHandle* handle, int portStart, int portCount, uint8 data[])
{
	if(handle != NULL && handle->DiCtrl != NULL)
	{
		return InstantDiCtrl_ReadAny(handle->DiCtrl, portStart, portCount, data);  	
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvWriteBit(AdvHandle* handle, int port, int bit, uint8 data)
{
	ErrorCode code = Success;
	if(handle != NULL && handle->DoCtrl != NULL)
	{
		return InstantDoCtrl_WriteBit(handle->DoCtrl, port, bit, data);  	
	}
	return AdvErrorCode_NullHandle;
}

AdvErrorCode AdvReadBit(AdvHandle* handle, int port, int bit, uint8* data)
{
	ErrorCode code = Success;
	if(handle != NULL && handle->DiCtrl != NULL)
	{
		return InstantDiCtrl_ReadBit(handle->DoCtrl, port, bit, data);  	
	}
	return AdvErrorCode_NullHandle;
}

void AdvGetErrorText(AdvErrorCode code, char* buff, int bufflen)
{
	wchar_t *strBuffer;	
	ErrorCode result;
	if(code > ErrorUndefined)
	{
		switch(code)
		{
			case AdvErrorCode_BIODAQNOTFOUND:
				StringCopyMax(buff, "BIODAQ.DLL NOTFOUND(NOT INSTALLED)", bufflen);
			case AdvErrorCode_NullHandle:
				StringCopyMax(buff, "Device is not open", bufflen);
			default:
				StringCopyMax(buff, "Unknown error", bufflen);
		}
		return;
	}

	strBuffer = calloc(bufflen, sizeof(wchar_t));
	result = AdxEnumToString(L"ErrorCode", code, bufflen, strBuffer);	
	if(result == Success)
		WideCharToMultiByte(CP_ACP, 0, strBuffer, -1, buff, bufflen, 0, 0);			
	else
		StringCopyMax(buff, "AdxEnumToString error", bufflen);		
	
	free(strBuffer);
}

