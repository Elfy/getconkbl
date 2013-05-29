;;
;; AHK script to switch keyboard layout
;;
;; RAlt switches to en/qwerty
;; RWin switches to ru
;; Temporarily switches to english layout while CapsLock is pressed
;;

GetConsoleKbLayoutModule := DllCall("LoadLibrary", "Str", "getconkbl.dll")
;result := DllCall("GetLastError", "UInt")
;MsgBox %result%

GetConsoleKbLayoutInit := DllCall("getconkbl\Initialize", Int, 0)
;MsgBox %GetConsoleKbLayoutInit%

CapsDown := 0

; These two are needed to allow RAlt+ and RWin+ combinations to be properly sent to apps
~RAlt & RWin::
return

~RWin & RAlt::
return

RWin::
ifWinActive ahk_class #32770
{
; various dialogs (?), openfile dialogs, run's dialog, miranda's message window
; these don't react to WM_INPUTLANGCHANGEREQUEST, need to send it to main window
; PostThreadMessage doesn't work here either

    WinGet, Active_Window_PID, PID, A ; get PID of active window
	SendMessage, 0x50, 0x1, 0x4190419, , ahk_pid %Active_Window_PID% ; ru
}
else
{
	SendMessage, 0x50, 0x1, 0x4190419, , A ; ru
}
return

RAlt::
ifWinActive ahk_class #32770
{
    WinGet, Active_Window_PID, PID, A ; get PID of active window
	SendMessage, 0x50, 0x1, 0x4090409, , ahk_pid %Active_Window_PID% ; en/qwerty
}
else
{
	SendMessage, 0x50, 0x1, 0x4090409, , A ; en/qwerty
}
return

*CapsLock::
	if not CapsDown {
		CapsDown := 1
		SetKeyDelay -1
		ifWinActive ahk_class ConsoleWindowClass
		{
			WinGet, Active_Caps_Window_PID, PID
			Lang_In_Caps_Window := DllCall("getconkbl\GetConsoleAppKbLayout", "UInt", Active_Caps_Window_PID)
		}
		else
		{
			WinGet, Active_Window_ID, ID, A
			Active_Window_Thread := DllCall("GetWindowThreadProcessId", "UInt", Active_Window_ID, "UInt*", Active_Caps_Window_PID)
			Lang_In_Caps_Window := DllCall("GetKeyboardLayout", "UInt", Active_Window_Thread)
		}
		SendMessage, 0x50, 0x1, 0x4090409, , ahk_pid %Active_Caps_Window_PID% ; switch to en/qwerty

	}
return

*CapsLock up::
    SetKeyDelay -1

	SendMessage, 0x50, 0x1, %Lang_In_Caps_Window%, , ahk_pid %Active_Caps_Window_PID% ; switch to saved layout
	CapsDown := 0
return
