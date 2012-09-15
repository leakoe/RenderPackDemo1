//------------------------------------------------------------------------------
// <copyright file="SingleFace.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------
//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by SingleFace.rc
//

#define IDS_APP_TITLE           103

#define IDR_MAINFRAME           128
#define IDD_SINGLEFACE_DIALOG   102
#define IDD_ABOUTBOX            103
#define IDM_ABOUT               104
#define IDM_EXIT                105
#define IDI_SINGLEFACE          107
#define IDC_SINGLEFACE          109
#define IDC_MYICON              2
#ifndef IDC_STATIC
#define IDC_STATIC              -1
#endif
// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS

#define _APS_NO_MFC                 130
#define _APS_NEXT_RESOURCE_VALUE    129
#define _APS_NEXT_COMMAND_VALUE     32771
#define _APS_NEXT_CONTROL_VALUE     1000
#define _APS_NEXT_SYMED_VALUE       110
#endif
#endif

#include "stdafx.h"

#include "EggAvatar.h"
#include <FaceTrackLib.h>
#include "FTHelper.h"


class SingleFace
{
public:
    SingleFace() 
    : m_hInst(NULL)
    , m_hWnd(NULL)
    , m_hAccelTable(NULL)
    , m_pImageBuffer(NULL)
    , m_pVideoBuffer(NULL)
    , m_depthType(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX)
    , m_colorType(NUI_IMAGE_TYPE_COLOR)
    , m_depthRes(NUI_IMAGE_RESOLUTION_320x240)
    , m_colorRes(NUI_IMAGE_RESOLUTION_640x480)
    , m_bNearMode(TRUE)
	, m_bSeatedSkeletonMode(FALSE)
    {}

    int Run(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow);

protected:
    BOOL                        InitInstance(HINSTANCE hInst, PWSTR lpCmdLine, int nCmdShow);
    void                        ParseCmdString(PWSTR lpCmdLine);
    void                        UninitInstance();
    ATOM                        RegisterClass(PCWSTR szWindowClass);
    static LRESULT CALLBACK     WndProcStatic(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK            WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK     About(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL                        PaintWindow(HDC hdc, HWND hWnd);
    BOOL                        ShowVideo(HDC hdc, int width, int height, int originX, int originY);
    BOOL                        ShowEggAvatar(HDC hdc, int width, int height, int originX, int originY);
    static void                 FTHelperCallingBack(LPVOID lpParam);
    static int const            MaxLoadStringChars = 100;

    HINSTANCE                   m_hInst;
    HWND                        m_hWnd;
    HACCEL                      m_hAccelTable;
    EggAvatar                   m_eggavatar;
    FTHelper                    m_FTHelper;
    IFTImage*                   m_pImageBuffer;
    IFTImage*                   m_pVideoBuffer;

    NUI_IMAGE_TYPE              m_depthType;
    NUI_IMAGE_TYPE              m_colorType;
    NUI_IMAGE_RESOLUTION        m_depthRes;
    NUI_IMAGE_RESOLUTION        m_colorRes;
    BOOL                        m_bNearMode;
	BOOL                        m_bSeatedSkeletonMode;
};

