/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2024. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  MAIN                             1       /* callback function: PanelCallback */
#define  MAIN_BTNCLOSE                    2       /* callback function: CallbackCloseDevice */
#define  MAIN_BTNOPEN                     3       /* callback function: CallbackOpenDevice */
#define  MAIN_TABDEVICE                   4
#define  MAIN_TABSELDEV                   5

     /* tab page panel controls */
#define  TABINFO_TBOXINFO                 2

     /* tab page panel controls */
#define  TABLIST_LISTDEVICES              2
#define  TABLIST_BTNREFRESH               3       /* callback function: CallbackRefreshDevicList */

     /* tab page panel controls */
#define  TABSETT_CBPORTCFGWRITE           2       /* callback function: CallbackSetPortsDirection */
#define  TABSETT_CBPORTCFGREAD            3       /* callback function: CallbackReadPortsDirection */
#define  TABSETT_TABLEPORTCFG             4       /* callback function: CallbackTablePortCfg */

     /* tab page panel controls */
#define  TABSINGLE_DEVNUM                 2

     /* tab page panel controls */
#define  TABTEST_TABLETEST                2       /* callback function: CallbackTableTest */
#define  TABTEST_TIMER                    3       /* callback function: TimerCallback */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK CallbackCloseDevice(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallbackOpenDevice(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallbackReadPortsDirection(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallbackRefreshDevicList(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallbackSetPortsDirection(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallbackTablePortCfg(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK CallbackTableTest(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK PanelCallback(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TimerCallback(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
