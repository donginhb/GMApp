/*hdr
**
**	Copyright Mox Products, Australia
**
**	FILE NAME:	TalkTimeSetWnd.c
**
**	AUTHOR:		Jeff Wang
**
**	DATE:		03 - Jun - 2009
**
**	FILE DESCRIPTION:
**				
**
**	FUNCTIONS:
**
**	
**
**	NOTES:
** 
*/
/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <string.h>
#include <stdlib.h>

/************** USER INCLUDE FILES ***************************************************/

#include "MXTypes.h"
#include "MXCommon.h"
#include "MenuParaProc.h"

#include "RingTimeSetWnd.h"

/************** DEFINES **************************************************************/

#define MAX_RING_TIME 60
#define MIN_RING_TIME 15

/************** TYPEDEFS *************************************************************/

/************** STRUCTURES ***********************************************************/

/************** EXTERNAL DECLARATIONS ************************************************/

//!!!  It is H/H++ file specific, nothing should be defined here

/************** ENTRY POINT DECLARATIONS *********************************************/

/************** LOCAL DECLARATIONS ***************************************************/

static VOID		RingTimeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static LRESULT	CALLBACK RingTimeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID RingTimeSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static VOID	ShowRingTimeSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

static BOOL bShowPromptInfo = FALSE;
static DWORD StoreRingTimeTime = 0;
static UCHAR RingTimeIndex = 0;

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetWndPaint
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
RingTimeSetWndPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	BYTE RingTimeNum[4] = { 0 };
	CHAR	pBuf[TITLE_BUF_LEN] = { 0 };
	DWORD nTemp			=	0;

	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;		

	nTemp				=	StoreRingTimeTime;

	RingTimeNum[0] = (BYTE)(nTemp/100) + 0x30;
	nTemp				= nTemp % 100;
	RingTimeNum[1] = (BYTE)(nTemp/10) + 0x30;
	nTemp				= nTemp % 10;
	RingTimeNum[2] = (BYTE)(nTemp) + 0x30;	
			
	strcpy(pBuf, (CHAR*)RingTimeNum);

	if (SET_ENGLISH == g_SysConfig.LangSel) 
	{	
		strcat(pBuf, " s");	
	}
	else if (SET_CHINESE == g_SysConfig.LangSel) 
	{	
		strcat(pBuf, " ��");
	}
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{	
		strcat(pBuf, " s");	
	}
	DrawDataParaSet(Hdc, pBuf, RingTimeIndex);
	
	EndPaint(hWnd, &ps);
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetWndProc
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static LRESULT CALLBACK 
RingTimeSetWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{     
	switch (Msg)
	{
	case WM_CREATE:
		RingTimeIndex = 0;
		StoreRingTimeTime = g_SysConfig.RingTime;
		PostMessage(hWnd,  WM_SETTIMER, INTERFACE_SETTLE_TIME, 0);
		break;		
		
	case WM_SETTIMER:
		SetTimer(hWnd, TIMER_RING_TIME, wParam, NULL);
		break;

	case WM_PAINT:
		if (GetFocus() == hWnd)
		{
			if (bShowPromptInfo)
			{
				ShowRingTimeSetPromptInfo(hWnd, Msg, wParam, lParam);
			}
			else
			{
				RingTimeSetWndPaint(hWnd, Msg, wParam, lParam);
			}
		}
		break;
		
	case WM_REFRESHPARENT_WND:
		SetFocus(hWnd);
		PostMessage(hWnd,  WM_PAINT, 0, 0);
		break;

	case WM_CHAR:
		if (GetFocus() != hWnd) 
		{
			SendMessage(GetFocus(),  WM_CHAR, wParam, 0l);
			return;			
		}
		if (!bShowPromptInfo) 
		{
			ResetTimer(hWnd, TIMER_RING_TIME, INTERFACE_SETTLE_TIME, NULL);
			RingTimeSetKeyProcess(hWnd, Msg, wParam, lParam);
			PostMessage(hWnd,  WM_PAINT, 0, 0);
		}
		break;
		
	case WM_TIMER:
		KillTimer(hWnd, TIMER_RING_TIME);
		if (bShowPromptInfo) 
		{
			bShowPromptInfo = FALSE;		
			PostMessage(hWnd,  WM_CLOSE, 0, 0);
		}
		else
		{
			KillAllChildWnd(hWnd);		
		}
		
		break;
		
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_RING_TIME);
		StoreRingTimeTime	=	0;
//		SendMessage(GetFocus(),  WM_REFRESHPARENT_WND, 0, 0);		
		break;
		
	default:
//		if (WM_CLOSE == Msg) 
//		{
//			RemoveOneWnd(hWnd);
//		}
//		return DefWindowProc(hWnd, Msg, wParam, lParam);

		DefWindowProc(hWnd, Msg, wParam, lParam);
		
		if (WM_CLOSE == Msg) 
		{
			RemoveOneWnd(hWnd);
		}
		
	}
	return 0;
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	CreateGateOpenDelaySetWnd
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
VOID
CreateRingTimeSetWnd(HWND hwndParent)
{
	static char szAppName[] = "RingTimeSet";
	HWND		hWnd;
	WNDCLASS	WndClass;
	
	WndClass.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc	= (WNDPROC) RingTimeSetWndProc;
	WndClass.cbClsExtra		= 0;
	WndClass.cbWndExtra		= 0;
	WndClass.hInstance		= 0;
	WndClass.hIcon			= 0;
	WndClass.hCursor		= 0;
	WndClass.hbrBackground	= (HBRUSH)GetStockObject(BACKGROUND_COLOR);
	WndClass.lpszMenuName	= NULL;
	WndClass.lpszClassName	= szAppName;
	
	RegisterClass(&WndClass);
	
	hWnd = CreateWindowEx(
		0L,					// Extend style	(0)
		szAppName,				// Class name	(NULL)
		"RingTimeSet",			// Window name	(Not NULL)
		WS_OVERLAPPED | WS_VISIBLE | WS_CHILD,	// Style		(0)
		0,					// x			(0)
		0,					// y			(0)
		SCREEN_WIDTH,		// Width		
		SCREEN_HEIGHT,		// Height
		g_hMainWnd /*hwndParent*/,		// Parent		(MwGetFocus())
		NULL,				// Menu			(NULL)
		NULL,				// Instance		(NULL)
		NULL);				// Parameter	(NULL)
	
	AddOneWnd(hWnd,WND_MEUN_PRIORITY_1);
	if (hWnd == GetFocus())
	{
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}
	else
	{
		ShowWindow(hWnd, SW_HIDE);
	}
}

/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	GateOpenDelaySetKeyProcess
**	AUTHOR:		   Jeff Wang
**	DATE:		22 - Aug - 2008
**
**	DESCRIPTION:	
**			Process UP,DOWN,RETURM and ENTER	
**
**	ARGUMENTS:		
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			Any thing need to be noticed.
*/

static VOID 
RingTimeSetKeyProcess(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	BYTE	RingTimeNum[4]	= { 0 };
	DWORD	nTemp			=	0;
	
	wParam =  KeyBoardMap(wParam);

	if (KEY_RETURN == wParam) 
	{	
		PostMessage(hWnd,  WM_CLOSE, 0, 0);
	}
	else if (KEY_ENTER == wParam) 
	{
		bShowPromptInfo = TRUE;
		g_SysConfig.RingTime	=	StoreRingTimeTime;
		SaveMenuPara2Mem();
	}
	else if ((wParam >= KEY_NUM_0) && (wParam <= KEY_NUM_9))
	{
		nTemp				=	StoreRingTimeTime;
		RingTimeNum[0] = (BYTE)(nTemp/100) ;
		nTemp				= nTemp % 100;
		RingTimeNum[1] = (BYTE)(nTemp/10) ;
		nTemp				= nTemp % 10;
		RingTimeNum[2] = (BYTE)(nTemp) ;	
		
		RingTimeNum[RingTimeIndex] = ((BYTE)wParam - 0x30);
		
		StoreRingTimeTime =								100* RingTimeNum[0]
													+   10* RingTimeNum[1]
													+       RingTimeNum[2];
		
		if (StoreRingTimeTime > MAX_RING_TIME) 
		{
			StoreRingTimeTime = MAX_RING_TIME;
		}
		else if (StoreRingTimeTime < MIN_RING_TIME) 
		{
			StoreRingTimeTime = MIN_RING_TIME;
		}		
		RingTimeIndex++;
		if (3 == RingTimeIndex)
		{
			RingTimeIndex = 0;
		}
	}
}


/*hdr
**	Copyright Mox Products, Australia
**
**	FUNCTION NAME:	ShowGateOpenDelaySetPromptInfo
**	AUTHOR:			Jeff Wang
**	DATE:			22 - Aug - 2008
**
**	DESCRIPTION:	
**
**
**	ARGUMENTS:	ARGNAME		DRIECTION	TYPE	DESCRIPTION
**				None
**	RETURNED VALUE:	
**				None
**	NOTES:
**			
*/
static VOID
ShowRingTimeSetPromptInfo(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC							Hdc;
	PAINTSTRUCT		ps;
	RECT					   Rect;	
	
	Hdc = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &Rect);
	
	FastFillRect(Hdc, &Rect, BACKGROUND_COLOR);	
	Hdc->bkcolor = BACKGROUND_COLOR;	
	Hdc->textcolor = FONT_COLOR;	
	
	if (SET_CHINESE == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, "����ʱ�����޸�", 1);
	}
	else if (SET_ENGLISH == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc, "Ring Time Modified", 1);
	}
	else if (SET_HEBREW == g_SysConfig.LangSel) 
	{
		MXDrawText_Center(Hdc,GetHebrewStr(HS_ID_RINGTIMEMODFYED), 1);
	}
	
	ResetTimer(hWnd, TIMER_RING_TIME, PROMPT_SHOW_TIME, NULL);

	EndPaint(hWnd, &ps);
}




