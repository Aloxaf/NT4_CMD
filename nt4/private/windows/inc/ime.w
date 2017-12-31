/*++ BUILD Version: 0003    // Increment this if a change has global effects ;both
                                                                         ;both
Copyright (c) 1985-1996, Microsoft Corporation                           ;both
                                                                         ;both
Module Name:                                                             ;both
                                                                         ;both
    ime.h
    imep.h                                                               ;internal
                                                                         ;both
Abstract:                                                                ;both
                                                                         ;both
    Private                                                              ;internal
    Procedure declarations, constant definitions and macros for the IME  ;both
    component.                                                           ;both
                                                                         ;both
--*/                                                                     ;both

#ifndef _IME_
#define _IME_

#ifndef _IMEP_                        ;internal
#define _IMEP_                        ;internal
                                      ;internal
#ifdef __cplusplus                      ;both
extern "C" {                            ;both
#endif /* __cplusplus */                ;both
                                        ;both

#ifndef _WINDEF_
typedef unsigned int UINT;
#endif // _WINDEF_

#define IME_MAXPROCESS 32

LRESULT WINAPI SendIMEMessageEx%(HWND, LPARAM);

//
// IMESTRUCT structure for SendIMEMessageEx
// IMESTRUCT structure for SendIMEMessage(Ex)                            ;internal
//
typedef struct tagIMESTRUCT {
    UINT     fnc;        // function code
    WPARAM   wParam;     // word parameter
    UINT     wCount;     // word counter
    UINT     dchSource;  // offset to Source from top of memory object
    UINT     dchDest;    // offset to Desrination from top of memory object
    LPARAM   lParam1;
    LPARAM   lParam2;
    LPARAM   lParam3;
} IMESTRUCT,*PIMESTRUCT,NEAR *NPIMESTRUCT,FAR *LPIMESTRUCT;



#ifdef KOREA     // BeomOh - 10/05/92
#define CP_HWND                 0
#define CP_OPEN                 1
#define CP_DIRECT               2
#define CP_LEVEL                3

#define lpSource(lpks) (LPSTR)((LPSTR)lpks+lpks->dchSource)
#define lpDest(lpks)   (LPSTR)((LPSTR)lpks+lpks->dchDest)
#endif // KOREA

//
//      Virtual Keys
//
#ifdef KOREA    // BeomOh - 9/29/92
#define VK_FINAL        0x18    /* dummy VK to make final on mouse down */
#define VK_CONVERT      0x1C
#define VK_NONCONVERT   0x1D
#define VK_ACCEPT       0x1E
#define VK_MODECHANGE   0x1F
#else // KOREA
#define VK_KANA         0x15
#define VK_KANJI        0x19

#define VK_CONVERT      0x1C
#define VK_NONCONVERT   0x1D
#define VK_ACCEPT       0x1E
#define VK_MODECHANGE   0x1F

#define VK_DBE_ALPHANUMERIC              0x0f0
#define VK_DBE_KATAKANA                  0x0f1
#define VK_DBE_HIRAGANA                  0x0f2
#define VK_DBE_SBCSCHAR                  0x0f3
#define VK_DBE_DBCSCHAR                  0x0f4
#define VK_DBE_ROMAN                     0x0f5
#define VK_DBE_NOROMAN                   0x0f6
#define VK_DBE_ENTERWORDREGISTERMODE     0x0f7
#define VK_DBE_IME_WORDREGISTER          VK_DBE_ENTERWORDREGISTERMODE    ;internal
#define VK_DBE_ENTERIMECONFIGMODE        0x0f8
#define VK_DBE_IME_DIALOG                VK_DBE_ENTERIMECONFIGMODE       ;internal
#define VK_DBE_FLUSHSTRING               0x0f9
#define VK_DBE_FLUSH                     VK_DBE_FLUSHSTRING              ;internal
#define VK_DBE_CODEINPUT                 0x0fa
#define VK_DBE_NOCODEINPUT               0x0fb
#define VK_DBE_DETERMINESTRING           0x0fc
//#define VK_DBE_ENTERPENCONVERSIONMODE  0x0fd                           ;internal
#define VK_DBE_ENTERDLGCONVERSIONMODE    0x0fd
#endif // KOREA

#define VK_OEM_SEMICLN          0x0ba  //   ;  ** :             ;internal
#define VK_OEM_EQUAL            0x0bb  //   =  ** +             ;internal
#define VK_OEM_SLASH            0x0bf  //   /  ** ?             ;internal
#define VK_OEM_LBRACKET         0x0db  //   [  ** {             ;internal
#define VK_OEM_BSLASH           0x0dc  //   \  ** |             ;internal
#define VK_OEM_RBRACKET         0x0dd  //   ]  ** |             ;internal
#define VK_OEM_QUOTE            0x0de  //   '  ** "             ;internal

//
//     switch for wParam of IME_SETCONVERSIONWINDOW
//     switch for wParam of IME_MOVECONVERTWINDOW                        ;internal
//
#define MCW_DEFAULT             0x00
#define MCW_RECT                0x01
#define MCW_WINDOW              0x02
#define MCW_SCREEN              0x04
#define MCW_VERTICAL            0x08
#define MCW_HIDDEN              0x10
#define MCW_CMD                 0x16        // command mask              ;internal
#define MCW_CONSOLE_IME         0x8000                                   ;internal

//
//    switch for wParam of IME_SETCONVERSIONMODE
//       and IME_GETCONVERSIONMODE
//    switch for wParam of IME_SET_MODE(IME_SETCONVERSIONMODE)           ;internal
//       and IME_GET_MODE(IME_GETCONVERSIONMODE)                         ;internal
//
#define IME_MODE_ALPHANUMERIC   0x0001
#ifdef KOREA    // BeomOh - 9/29/92
#define IME_MODE_SBCSCHAR       0x0002
#define IME_MODE_HANJACONVERT   0x0004
#else  // !KOREA
#define IME_MODE_KATAKANA       0x0002
#define IME_MODE_HIRAGANA       0x0004
#define IME_MODE_SBCSCHAR       0x0008
#define IME_MODE_DBCSCHAR       0x0010
#define IME_MODE_ROMAN          0x0020
#define IME_MODE_NOROMAN        0x0040
#define IME_MODE_CODEINPUT      0x0080
#define IME_MODE_NOCODEINPUT    0x0100
//
// ;internal is added by JAPAN, CWIN user should ignore it ;Internal
// 0x1, 0x2, 0x4, 0x20, 0x40, 0x80, 0x100 is not for CWIN  ;Internal
// 0x8, 0x10, and below IME_MODE_??? will be use by CWIN   ;Internal
//
#define IME_MODE_LHS                0x00200                                   ;internal
#define IME_MODE_NOLHS              0x00400                                   ;internal
#define IME_MODE_SK                 0x00800                                   ;internal
#define IME_MODE_NOSK               0x01000                                   ;internal
#define IME_MODE_XSPACE             0x02000                                   ;internal
#define IME_MODE_NOXSPACE           0x04000                                   ;internal
#endif // !KOREA

//
//     IME APIs
//     Subfunctions for WM_CONVERTREQUEST or WM_CONVERTREQUESTEX         ;internal
//
#define IME_GETIMECAPS            0x03
#define IME_QUERY                 IME_GETIMECAPS                         ;internal
#define IME_SETOPEN               0x04
#define IME_GETOPEN               0x05
#define IME_ENABLEDOSIME          0x06                                   ;internal
#define IME_ENABLE                IME_ENABLEDOSIME                       ;internal
#define IME_GETVERSION            0x07
#define IME_SETCONVERSIONWINDOW   0x08
#ifdef  KOREA
#define IME_MOVEIMEWINDOW         IME_SETCONVERSIONWINDOW
#else
#define IME_MOVECONVERTWINDOW     IME_SETCONVERSIONWINDOW                ;internal
#endif
#define IME_SETCONVERSIONMODE     0x10
#ifdef KOREA    // BeomOh - 10/23/92
#define IME_SET_MODE              0x12
#else
#define IME_SET_MODE              IME_SETCONVERSIONMODE                  ;internal
#endif
#define IME_GETCONVERSIONMODE     0x11
#define IME_GET_MODE              IME_GETCONVERSIONMODE                  ;internal
#define IME_SETCONVERSIONFONT     0x12                                   ;internal
#define IME_SETFONT               IME_SETCONVERSIONFONT                  ;internal
#define IME_SENDVKEY              0x13
#define IME_SENDKEY               IME_SENDVKEY                           ;internal
#define IME_DESTROYIME            0x14                                   ;internal
#define IME_DESTROY               IME_DESTROYIME                         ;internal
#define IME_PRIVATE               0x15                                   ;internal
#define IME_WINDOWUPDATE          0x16                                   ;internal
#define IME_SELECT                0x17                                   ;internal
#define IME_ENTERWORDREGISTERMODE 0x18
#define IME_WORDREGISTER          IME_ENTERWORDREGISTERMODE              ;internal
#define IME_SETCONVERSIONFONTEX   0x19
#define IME_DBCSNAME              0x1A                                   ;internal
#define IME_MAXKEY                0x1B                                   ;internal
#define IME_WINNLS_SK             0x1C                                   ;internal
#define IME_CODECONVERT           0x20                                   ;internal
#define IME_SETUSRFONT            0x20                                   ;internal
#define IME_CONVERTLIST           0x21                                   ;internal
#define IME_QUERYUSRFONT          0x21                                   ;internal
#define IME_INPUTKEYTOSEQUENCE    0x22                                   ;internal
#define IME_SEQUENCETOINTERNAL    0x23                                   ;internal
#define IME_QUERYIMEINFO          0x24                                   ;internal
#define IME_DIALOG                0x25                                   ;internal
#define IME_AUTOMATA              0x30                                   ;internal
#define IME_HANJAMODE             0x31                                   ;internal
#define IME_GETLEVEL              0x40                                   ;internal
#define IME_SETLEVEL              0x41                                   ;internal
#ifdef  KOREA   // 01/12/93 KDLee MSCH
#define IME_GETMNTABLE            0x42  // reserved for HWIN ;Internal
#endif

//#ifdef PEN                                    ;Internal
#define IME_SETUNDETERMINESTRING  0x50                                   ;internal
#define IME_SETCAPTURE            0x51                                   ;internal
//#endif                                        ;Internal
#define IME_CONSOLE_GET_PROCESSID     0x80                  // Win32     ;internal
#define IME_CONSOLE_CREATE            0x81                  // Win32     ;internal
#define IME_CONSOLE_DESTROY           0x82                  // Win32     ;internal
#define IME_CONSOLE_SETFOCUS          0x83                  // Win32     ;internal
#define IME_CONSOLE_KILLFOCUS         0x84                  // Win32     ;internal
#define IME_CONSOLE_BUFFER_SIZE       0x85                  // Win32     ;internal
#define IME_CONSOLE_WINDOW_SIZE       0x86                  // Win32     ;internal
#define IME_CONSOLE_SET_IME_ON_WINDOW 0x87                  // Win32     ;internal
#define IME_CONSOLE_MODEINFO          0x88                               ;internal
#define IME_PRIVATEFIRST          0x0100                                 ;internal
#define IME_PRIVATELAST           0x04FF                                 ;internal

#ifdef KOREA    // BeomOh - 9/29/92
//
// IME_CODECONVERT subfunctions
//
#define IME_BANJAtoJUNJA        0x13
#define IME_JUNJAtoBANJA        0x14
#define IME_JOHABtoKS           0x15
#define IME_KStoJOHAB           0x16

//
// IME_AUTOMATA subfunctions
//
#define IMEA_INIT               0x01
#define IMEA_NEXT               0x02
#define IMEA_PREV               0x03

//
// IME_HANJAMODE subfunctions
//
#define IME_REQUEST_CONVERT     0x01
#define IME_ENABLE_CONVERT      0x02

//
// IME_MOVEIMEWINDOW subfunctions
//
#define INTERIM_WINDOW          0x00
#define MODE_WINDOW             0x01
#define HANJA_WINDOW            0x02
#endif // KOREA

//
// ;internal is needed by JAPAN, CWIN user should ignore it  ;Internal
//
#if defined(TAIWAN)                                             ;internal

#ifdef CWIN_SK                                                  ;internal
#define SK_KEY_MAX              46                              ;internal
#endif                                                          ;internal

#define IMEPROC_SWITCH          0x0001                          ;internal
/* the IMEPROC_SK is from 0x0010 ~ 0x001F */                    ;internal
#define IMEPROC_SK              0x0010                          ;internal
#define IMEPROC_SK0             0x0010                          ;internal
#define IMEPROC_SK1             0x0011                          ;internal
#define IMEPROC_SK2             0x0012                          ;internal

#define PROC_INFO               0x0001                          ;internal
#define PROC_SHOW               0x0002                          ;internal
#define PROC_HIDE               0x0004                          ;internal

#define BY_IME_HWND             0x0000                          ;internal
#define BY_IME_NAME             0x0001                          ;internal
#define BY_IME_DESCRIPTION      0x0002                          ;internal
#define BY_IME_DEFAULT          0x000F                          ;internal

/* Those bits are used by ControlIMEMessage() */                ;internal
/* 0x0030 - the two bits are for post/send messages control */  ;internal
#define CTRL_MSG_MASK           0x0030                          ;internal
/* CTRL_SEND is default one, because it is usually used */      ;internal
#define CTRL_SEND               0x0000                          ;internal
#define CTRL_POST               0x0010                          ;internal
/* CTRL_NONE - don't send and post */                           ;internal
#define CTRL_NONE               0x0030                          ;internal

#define CTRL_USER_ALLOC         0x0040                          ;internal

/* CTRL_MODIFY_??? - modify imepro of specified IME */          ;internal
#define CTRL_MODIFY_USR_DIC     0x0080                          ;internal
// CTRL_MODIFY is "or" all modify bits, but now only one
#define CTRL_MODIFY             CTRL_MODIFY_USR_DIC             ;internal
#endif                                                          ;internal

//
//    error code
//
#define IME_RS_ERROR            0x01    // genetal error
#define IME_RS_NOIME            0x02    // IME is not installed
#define IME_RS_TOOLONG          0x05    // given string is too long
#define IME_RS_ILLEGAL          0x06    // illegal charactor(s) is string
#define IME_RS_NOTFOUND         0x07    // no (more) candidate
#define IME_RS_NOROOM           0x0a    // no disk/memory space
#define IME_RS_DISKERROR        0x0e    // disk I/O error
#define IME_RS_CAPTURED         0x10    // IME is captured               ;internal
#define IME_RS_INVALID          0x11    // Win3.1/NT
#define IME_RS_NEST             0x12    // called nested
#define IME_RS_SYSTEMMODAL      0x13    // called when system mode

//
//   report messge from IME to WinApps
//
#define WM_IME_REPORT       0x0280

//
//   report message parameter for WM_IME_REPORT
//
#define IR_STRINGSTART      0x100
#define IR_STRINGEND        0x101
#define IR_MOREROOM             0x110                                    ;internal
#define IR_OPENCONVERT      0x120
#define IR_CHANGECONVERT    0x121
#define IR_CLOSECONVERT     0x122
#define IR_FULLCONVERT      0x123
#define IR_IMESELECT        0x130
#define IR_STRING       0x140
#define IR_IMERELEASED          0x150                                    ;internal
#define IR_DBCSCHAR             0x160
#define IR_UNDETERMINE          0x170
#define IR_STRINGEX             0x180   // New for 3.1
#define IR_MODEINFO             0x190

//#define WM_CONVERTREQUESTEX     0x0109
#define WM_WNT_CONVERTREQUESTEX 0x0109 /* WM_CONVERTREQUESTEX: 109 for NT, 108 for OT */
#define WM_CONVERTREQUEST       0x010A
#define WM_CONVERTRESULT        0x010B
#define WM_INTERIM              0x010C

#define WM_IMEKEYDOWN           0x290
#define WM_IMEKEYUP             0x291

#define IMEVER_31               0x0a03                                   ;internal



//
// UNDETERMINESTRING structure for IR_UNDETERMINE
//
typedef struct tagUNDETERMINESTRUCT {
    DWORD    dwSize;
    UINT     uDefIMESize;
    UINT     uDefIMEPos;
    UINT     uUndetTextLen;
    UINT     uUndetTextPos;
    UINT     uUndetAttrPos;
    UINT     uCursorPos;
    UINT     uDeltaStart;
    UINT     uDetermineTextLen;
    UINT     uDetermineTextPos;
    UINT     uDetermineDelimPos;
    UINT     uYomiTextLen;
    UINT     uYomiTextPos;
    UINT     uYomiDelimPos;
} UNDETERMINESTRUCT,*PUNDETERMINESTRUCT,NEAR *NPUNDETERMINESTRUCT,FAR *LPUNDETERMINESTRUCT;


typedef struct tagSTRINGEXSTRUCT {
    DWORD    dwSize;
    UINT     uDeterminePos;
    UINT     uDetermineDelimPos;
    UINT     uYomiPos;
    UINT     uYomiDelimPos;
} STRINGEXSTRUCT,NEAR *NPSTRINGEXSTRUCT,FAR *LPSTRINGEXSTRUCT;


#if 0
#define IMEMODE_CCHMAXTEXT 256
typedef struct tagIMEMODEINFO {
     DWORD     cchSystemTextLength;
     CHAR_INFO aciSystemText[IMEMODE_CCHMAXTEXT];
     DWORD     cchModeTextLength;
     CHAR_INFO aciModeText[IMEMODE_CCHMAXTEXT];
} IMEMODEINFO,*PIMEMODEINFO,*LPIMEMODEINFO;
#endif

#if defined(TAIWAN)                                             ;internal
LONG WINAPI WINNLSIMEControl(HWND,HWND,LPIMESTRUCT);            ;internal
#define STATUSWINEXTRA  10                                      ;internal
#endif                                                          ;internal

;begin_both
#ifdef __cplusplus
}
#endif  /* __cplusplus */
;end_both

#endif  /* !_IMEP_ */     ;internal
#endif // _IME_
