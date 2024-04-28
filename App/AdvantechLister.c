#include <utility.h>
#include <toolbox.h>
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include "_MAIN.h"
#include "..\lib\Advantech.h"

#define ADV_MAX_DEVICE_DESC_LEN 128
#define ADV_MAX_DEVICES_COUNT 5
#define ADV_MAX_PORTS_COUNT 12

static int g_mainPanelHandle, g_tabInfoHandle, g_tabCfgHandle, g_tabTestHandle;
static int g_tabDeviceListHandle, g_tabSingleDeviceHandle;
static int g_devicesCount = 0;
static int g_portCount = 0;
static AdvHandle g_devices[ADV_MAX_DEVICES_COUNT];
static AdvPortType g_portsTypes[ADV_MAX_PORTS_COUNT];
static AdvPortDir g_portsDirs[ADV_MAX_PORTS_COUNT];
static AdvHandle* g_DevHandle;

int InitPanels(void);

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((g_mainPanelHandle = LoadPanel (0, "_MAIN.uir", MAIN)) < 0)
		return -1;
	
	if (InitPanels() < 0)
		return -2;
	
	DisplayPanel (g_mainPanelHandle);
	RunUserInterface ();
	DiscardPanel (g_mainPanelHandle);
	return 0;
}

int InitPanels(void)
{
	GetPanelHandleFromTabPage(g_mainPanelHandle, MAIN_TABSELDEV, 0, &g_tabDeviceListHandle);
	GetPanelHandleFromTabPage(g_mainPanelHandle, MAIN_TABSELDEV, 1, &g_tabSingleDeviceHandle);
	
	GetPanelHandleFromTabPage(g_mainPanelHandle, MAIN_TABDEVICE, 0, &g_tabInfoHandle);
	GetPanelHandleFromTabPage(g_mainPanelHandle, MAIN_TABDEVICE, 1, &g_tabCfgHandle);
	GetPanelHandleFromTabPage(g_mainPanelHandle, MAIN_TABDEVICE, 2, &g_tabTestHandle);

	ClearListCtrl (g_tabDeviceListHandle, TABLIST_LISTDEVICES);	
	SetCtrlVal(g_tabInfoHandle, TABINFO_TBOXINFO, "");
	DeleteTableRows(g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, -1);
	DeleteTableRows(g_tabTestHandle, TABTEST_TABLETEST, 1, -1);
	SetCtrlAttribute(g_mainPanelHandle, MAIN_BTNOPEN, ATTR_DIMMED, 0);
	SetCtrlAttribute(g_mainPanelHandle, MAIN_BTNCLOSE, ATTR_DIMMED, 1);
	SetCtrlAttribute(g_tabCfgHandle, TABSETT_CBPORTCFGWRITE, ATTR_DIMMED, 1);
	SetCtrlAttribute(g_tabCfgHandle, TABSETT_CBPORTCFGREAD, ATTR_DIMMED, 1);
	SetActiveTabPage(g_mainPanelHandle, MAIN_TABDEVICE, 0);    
	return 0;
}

void ShowAdvErrorPopup(AdvErrorCode error)
{
	char msg[1024];
	AdvGetErrorText(error, msg, sizeof(msg));
	MessagePopup("Advantech", msg);
}
 
const char* AdvPortTypeToStr(AdvPortType portType)
{
	switch(portType)
	{
   		case AdvPortType_PortDi: return "Di";
		case AdvPortType_PortDo: return "Do";
		case AdvPortType_PortDio: return "Dio";  
		case AdvPortType_Port8255A: return "8255A";
		case AdvPortType_Port8255C: return "8255C";
		case AdvPortType_PortIndividual: return "Individual";
	}
	return "Unknown";
}

AdvPortType StrToAdvPortType(char* str)
{
	if(strcmp(str, "Di")==0) return AdvPortType_PortDi;
	if(strcmp(str, "Do")==0) return AdvPortType_PortDo;
	if(strcmp(str, "Dio")==0) return AdvPortType_PortDio;
	if(strcmp(str, "8255A")==0) return AdvPortType_Port8255A;
	if(strcmp(str, "8255C")==0) return AdvPortType_Port8255C;
	if(strcmp(str, "Individual")==0) return AdvPortType_PortIndividual;
	return AdvPortType_PortUnknown;
}

AdvHandle* GetSelectedDevice(void)
{
	int selected;
	GetCtrlIndex(g_tabDeviceListHandle, TABLIST_LISTDEVICES, &selected);
	if(selected >= 0)
	{
		return &g_devices[selected];
	}
	return NULL;
}

void PopulateTabInfo(void)
{
	char buff[128];
	char linebuff[256];
	AdvErrorCode code;  	
	int boardId;
	
	ResetTextBox (g_tabInfoHandle, TABINFO_TBOXINFO, "");
	
	sprintf(linebuff,"DeviceNumber: %i", g_DevHandle->DeviceNumber);
	InsertTextBoxLine(g_tabInfoHandle, TABINFO_TBOXINFO, -1, linebuff);

	AdvGetBoardId(g_DevHandle, &boardId);
	sprintf(linebuff,"BoardId: %i", boardId);
	InsertTextBoxLine(g_tabInfoHandle, TABINFO_TBOXINFO, -1, linebuff);
	
	code = AdvGetBoardVersion(g_DevHandle, buff, sizeof(buff));
	if(code == AdvErrorCode_Success)
	{
		sprintf(linebuff, "Board Version: %s", buff);
		InsertTextBoxLine(g_tabInfoHandle, TABINFO_TBOXINFO, -1, linebuff);
	}
	else
	{
		AdvGetErrorText(code, buff, sizeof(buff));
		sprintf(linebuff, "Board Version: %s", buff);
		InsertTextBoxLine(g_tabInfoHandle, TABINFO_TBOXINFO, -1, linebuff);
	}
	
	AdvGetDriverVersion(g_DevHandle, buff, sizeof(buff));
	sprintf(linebuff, "DriverVersion: %s", buff);
	InsertTextBoxLine(g_tabInfoHandle, TABINFO_TBOXINFO, -1, linebuff);

	AdvGetDllVersion(g_DevHandle, buff, sizeof(buff));
	sprintf(linebuff, "DllVersion: %s", buff);
	InsertTextBoxLine(g_tabInfoHandle, TABINFO_TBOXINFO, -1, linebuff);

	AdvGetLocation(g_DevHandle, buff, sizeof(buff));
	sprintf(linebuff, "Location: %s", buff);
	InsertTextBoxLine(g_tabInfoHandle, TABINFO_TBOXINFO, -1, linebuff);
}

void BuildTabPortCfg(void)
{
	char buff[8];
	
	SetCtrlAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, ATTR_VISIBLE, 0);		
	DeleteTableRows(g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, -1);
	DeleteTableColumns(g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, -1);

	if(g_portsTypes != NULL)
	{
		InsertTableRows	(g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, g_portCount, VAL_USE_MASTER_CELL_TYPE);
		InsertTableColumns (g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, 1, VAL_CELL_STRING);
		InsertTableColumns (g_tabCfgHandle, TABSETT_TABLEPORTCFG, 2, 8, VAL_CELL_RING);

		InsertTableCellRangeRingItem (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakeRect(1, 2, g_portCount, 8), -1, "Input");
		InsertTableCellRangeRingItem (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakeRect(1, 2, g_portCount, 8), -1, "Output");
		
		SetTableColumnAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, ATTR_USE_LABEL_TEXT, 1);			
		SetTableColumnAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, ATTR_LABEL_TEXT, "Type");
		for(int i=0;i<8;i++)
		{
			sprintf(buff,"CH%d", 7-i);
			SetTableColumnAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, i+2, ATTR_USE_LABEL_TEXT, 1);			
			SetTableColumnAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, i+2, ATTR_LABEL_TEXT, buff);
		}
	}
	SetCtrlAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, ATTR_VISIBLE, 1);			
}

void BuildTabPortTest(void)
{
	int status;
	char buff[8];
	
	SetCtrlAttribute(g_tabTestHandle, TABTEST_TABLETEST, ATTR_VISIBLE, 0);	
	DeleteTableRows(g_tabTestHandle, TABTEST_TABLETEST, 1, -1);
	DeleteTableColumns(g_tabTestHandle, TABTEST_TABLETEST, 1, -1);
	
	if(g_portsTypes != NULL)
	{
		InsertTableRows	(g_tabTestHandle, TABTEST_TABLETEST, 1, g_portCount, VAL_USE_MASTER_CELL_TYPE);
		status = InsertTableColumns (g_tabTestHandle, TABTEST_TABLETEST, 1, 8, VAL_CELL_BUTTON);
		status = InsertTableColumns (g_tabTestHandle, TABTEST_TABLETEST, 9, 1, VAL_USE_MASTER_CELL_TYPE);
		SetTableCellRangeAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakeRect(1, 9, g_portCount, 1), ATTR_CELL_TYPE, VAL_CELL_NUMERIC);
		SetTableCellRangeAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakeRect(1, 9, g_portCount, 1), ATTR_DATA_TYPE , VAL_UNSIGNED_CHAR);
		SetTableCellRangeAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakeRect(1, 9, g_portCount, 1), ATTR_FORMAT , VAL_HEX_FORMAT);
		SetTableCellRangeAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakeRect(1, 9, g_portCount, 1), ATTR_PADDING , 2);

		SetTableColumnAttribute(g_tabTestHandle, TABTEST_TABLETEST, 9, ATTR_USE_LABEL_TEXT, 1);			
		SetTableColumnAttribute(g_tabTestHandle, TABTEST_TABLETEST, 9, ATTR_LABEL_TEXT, "HEX");			
		for(int i=0;i<8;i++)
		{
			sprintf(buff,"CH%d", 7-i);
			SetTableColumnAttribute(g_tabTestHandle, TABTEST_TABLETEST, i+1, ATTR_USE_LABEL_TEXT, 1);			
			SetTableColumnAttribute(g_tabTestHandle, TABTEST_TABLETEST, i+1, ATTR_LABEL_TEXT, buff);			
		}		
	}
	SetCtrlAttribute(g_tabTestHandle, TABTEST_TABLETEST, ATTR_VISIBLE, 1);
}

void PopulateTabPortCfg(void)
{
	AdvErrorCode code;  
	AdvPortDir portDir;
	int status;

	SetCtrlAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, ATTR_VISIBLE, 0);
	
	for (int i=0; i<g_portCount; i++)
	{
		status = SetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (1, i+1), AdvPortTypeToStr(g_portsTypes[i]));			
		status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (9, i+1), ATTR_CELL_MODE, VAL_HOT);
		switch(g_portsDirs[i])
		{
			case AdvPortDir_Input:
				status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (9, i+1), ATTR_CELL_MODE, VAL_INDICATOR);
				for (int j=0; j<8; j++)
				{
					status = SetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (j+2, i+1), "Input");
					status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (j+1, i+1), ATTR_CELL_MODE, VAL_INDICATOR);
				}
				break;
				
   			case AdvPortDir_LoutHin:
				for (int j=0; j<4; j++)
				{
					status = SetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (j+2, i+1), "Input");
					status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (j+1, i+1), ATTR_CELL_MODE, VAL_INDICATOR);
				}
				for (int j=4; j<8; j++)
				{
					status = SetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (j+2, i+1), "Output");
					status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (j+1, i+1), ATTR_CELL_MODE, VAL_HOT);
				}
				
				break;
				
   			case AdvPortDir_LinHout:
				for (int j=0; j<4; j++)
				{
					status = SetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (j+2, i+1), "Output");
					status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (j+1, i+1), ATTR_CELL_MODE, VAL_HOT);
				}
				for (int j=4; j<8; j++)
				{
					status = SetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (j+2, i+1), "Input");
					status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (j+1, i+1), ATTR_CELL_MODE, VAL_INDICATOR);
				}
				break;
				
   			case AdvPortDir_Output:
				for (int j=0; j<8; j++)
				{
					status = SetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (j+2, i+1), "Output");
					status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (j+1, i+1), ATTR_CELL_MODE, VAL_HOT);
				}
				break;
				
			default:
				break;
		}
	}
	SetCtrlAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, ATTR_VISIBLE, 1);	
}


void PopulateTabPortTest(void)
{
	int status;
	uint8 portdata[ADV_MAX_PORTS_COUNT] = {0};	
	
	if(g_DevHandle != NULL && g_portsTypes != NULL)
	{
		SetCtrlAttribute(g_tabTestHandle, TABTEST_TABLETEST, ATTR_VISIBLE, 0);	
		status = AdvReadAny(g_DevHandle, 0, g_portCount, portdata);
		for (int i=0; i<g_portCount; i++)
		{
			SetTableCellAttribute (g_tabTestHandle, TABTEST_TABLETEST, MakePoint (9, i+1), ATTR_CTRL_VAL, portdata[i]);
			for (int j=0; j<8; j++)
			{
				int val = AdvGetChannelState(portdata[i], 7-j);
				status = SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint (j+1, i+1), ATTR_CMD_BUTTON_COLOR, val ? VAL_GREEN : VAL_TRANSPARENT);
			}
		}	
		
		SetCtrlAttribute(g_tabTestHandle, TABTEST_TABLETEST, ATTR_VISIBLE, 1);
	}
}

void SetPortsDirection(void)
{
	int status;
	AdvErrorCode code;
	AdvPortDir dir;
	uint8 mask = 0;
	char tmp[10];
	
	for (int i=0; i<g_portCount; i++)
	{
		mask = 0;
		for(int j=0; j<8; j++)
		{
			status = GetTableCellVal (g_tabCfgHandle, TABSETT_TABLEPORTCFG, MakePoint (j+2, i+1), tmp);	
			if(strcmp(tmp, "Input") == 0)
				mask |= 1 << j;
		}
		
		dir = AdvPortDir_Output;
		if(mask & 0x80 && mask &0x01)
			dir = AdvPortDir_Input;
		else if(mask & 0x80 && !(mask &0x01))
			dir = AdvPortDir_LinHout;
		else if(!(mask & 0x80) && mask &0x01)
			dir = AdvPortDir_LoutHin;
			
		g_portsDirs[i] = dir;
		
	}
	code = AdvSetPortDirection(g_DevHandle, g_portsDirs, g_portCount);
}

void GetDeviceCfg(void)
{
	AdvErrorCode code;
	if(g_DevHandle != NULL)
	{
		code = AdvGetPortsTypes(g_DevHandle, g_portsTypes, ADV_MAX_PORTS_COUNT, &g_portCount);    				
		if(code != AdvErrorCode_Success)
		{
			ShowAdvErrorPopup(code);
			return;
		}
		code = AdvGetPortsDirections(g_DevHandle, g_portsDirs, ADV_MAX_PORTS_COUNT);	
		if(code != AdvErrorCode_Success)
		{
			ShowAdvErrorPopup(code);
			return;
		}
		
		BuildTabPortCfg();
		BuildTabPortTest();
		PopulateTabInfo();
		PopulateTabPortCfg();	
		PopulateTabPortTest();
	
		SetCtrlAttribute(g_mainPanelHandle, MAIN_TABSELDEV, ATTR_DIMMED, 1);
		SetCtrlAttribute(g_mainPanelHandle, MAIN_BTNCLOSE, ATTR_DIMMED, 0);
		SetCtrlAttribute(g_mainPanelHandle, MAIN_BTNOPEN, ATTR_DIMMED, 1);
		SetActiveTabPage(g_mainPanelHandle, MAIN_TABDEVICE, 0);
		SetCtrlAttribute(g_tabCfgHandle, TABSETT_CBPORTCFGWRITE, ATTR_DIMMED, 0);
		SetCtrlAttribute(g_tabCfgHandle, TABSETT_CBPORTCFGREAD, ATTR_DIMMED, 0);
	}	
}

void OpenDevice(void)
{
	AdvErrorCode code;
	int activetab, deviceNumber;
	
	GetActiveTabPage(g_mainPanelHandle, MAIN_TABSELDEV, &activetab);
	if(activetab < 0)
		return;
	
	if(activetab == 0)
	{
	 	g_DevHandle = GetSelectedDevice();
		code = AdvOpen(g_DevHandle);		
	}
	else
	{
		GetCtrlVal(g_tabSingleDeviceHandle, TABSINGLE_DEVNUM, &deviceNumber);		
		g_DevHandle = &(g_devices[0]);
		code = AdvOpenWithDeviceNumber(g_DevHandle, deviceNumber);	
	}
	if(code != AdvErrorCode_Success)
	{
		g_DevHandle = NULL;
		ShowAdvErrorPopup(code);
	}
	else
	{
		GetDeviceCfg();
	}
}

void CloseDevice(void)
{
	AdvErrorCode code;
	if(g_DevHandle != NULL)
	{
		AdvClose(g_DevHandle);
		g_DevHandle = NULL;
		SetCtrlAttribute(g_mainPanelHandle, MAIN_BTNCLOSE, ATTR_DIMMED, 1);
		SetCtrlAttribute(g_mainPanelHandle, MAIN_BTNOPEN, ATTR_DIMMED, 0);
		SetCtrlAttribute(g_mainPanelHandle, MAIN_TABSELDEV, ATTR_DIMMED, 0);
		SetActiveCtrl(g_tabDeviceListHandle, TABLIST_LISTDEVICES);
		SetCtrlAttribute(g_tabCfgHandle, TABSETT_CBPORTCFGWRITE, ATTR_DIMMED, 1);
		SetCtrlAttribute(g_tabCfgHandle, TABSETT_CBPORTCFGREAD, ATTR_DIMMED, 1);
		
		ResetTextBox (g_tabInfoHandle, TABINFO_TBOXINFO, "");
		DeleteTableRows(g_tabTestHandle, TABTEST_TABLETEST, 1, -1);
		DeleteTableRows(g_tabCfgHandle, TABSETT_TABLEPORTCFG, 1, -1);
	}
}

int CVICALLBACK PanelCallback (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_CLOSE:
			CloseDevice();
			QuitUserInterface (0);
			break;
	}
	return 0;
}

int CVICALLBACK CallbackOpenDevice (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			OpenDevice();
			break;
	}
	return 0;
}

int CVICALLBACK CallbackCloseDevice (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			CloseDevice();			
			break;
	}
	return 0;
}

int CVICALLBACK CallbackRefreshDevicList (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	AdvErrorCode code;
	
	switch (event)
	{
		case EVENT_COMMIT:
			
			ClearListCtrl (g_tabDeviceListHandle, TABLIST_LISTDEVICES);	
			code = AdvListDevices(g_devices, ADV_MAX_DEVICES_COUNT,  &g_devicesCount);
			for(int i=0; i<g_devicesCount; i++)
			{
				AdvHandle* handle = &g_devices[i];
				InsertListItem (g_tabDeviceListHandle, TABLIST_LISTDEVICES, -1, handle->Description, 0);
			}
			
			break;
	}
	return 0;
}

int CVICALLBACK CallbackReadPortsDirection (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			PopulateTabPortCfg();
			break;
	}
	return 0;
}

int CVICALLBACK CallbackSetPortsDirection (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
 			SetPortsDirection();
			PopulateTabPortCfg();
			break;
	}
	return 0;
}



int CVICALLBACK CallbackTablePortCfg (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char value[128];
	char type[128];
	AdvPortType advType;
	int start;
	switch (event)
	{
		case EVENT_COMMIT:
			SetCtrlAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, ATTR_VISIBLE, 0);		
			GetTableCellVal (panel, control, MakePoint (1, eventData1), type);
			GetTableCellVal (panel, control, MakePoint (eventData2, eventData1), value);
			advType = StrToAdvPortType(type);
			if(advType == AdvPortType_Port8255C)
			{
				start = 4*((eventData2-2) / 4);
				for(int i=0; i<4; i++)
				{
					SetTableCellVal (panel, control, MakePoint (start+i+2, eventData1), value);	
				}
			}
			else
			{
				for(int i=0; i<8; i++)
				{
					SetTableCellVal (panel, control, MakePoint (i+2, eventData1), value);	
				}
			}
			SetCtrlAttribute(g_tabCfgHandle, TABSETT_TABLEPORTCFG, ATTR_VISIBLE, 1);		
			break;
	}
	return 0;
}

int CVICALLBACK CallbackTableTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	AdvErrorCode code;
	int value, status;
	uint8 data;
	
	switch (event)
	{
		case EVENT_COMMIT:
			if(eventData2 == 9)
			{
				GetTableCellVal(g_tabTestHandle, TABTEST_TABLETEST, MakePoint(eventData2, eventData1), &data);
				AdvWriteAny(g_DevHandle, eventData1 - 1, 1, &data); 
				code = AdvReadAny(g_DevHandle, eventData1 - 1, 1, &data);
			}
			else
			{
				GetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint(eventData2, eventData1), ATTR_CMD_BUTTON_COLOR, &value);
				value = (value == VAL_GREEN) ? VAL_TRANSPARENT : VAL_GREEN;
				SetTableCellAttribute(g_tabTestHandle, TABTEST_TABLETEST, MakePoint(eventData2, eventData1), ATTR_CMD_BUTTON_COLOR, value);
				AdvWriteBit(g_DevHandle, eventData1 - 1, 8 - eventData2, value == VAL_GREEN ? 1 : 0);
			}
			break;
	}
	return 0;
}

int CVICALLBACK TimerCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
			PopulateTabPortTest();
			break;
	}
	return 0;
}
