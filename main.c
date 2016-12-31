/**
*Ejects USB's and CD roms
*TODO:
*update for new devices added - works, used two while loops, and threads :)
*Dont attempt disconnect USB program is running on. - fixed, will try to disconnect usb, then fail if cant create lock. like using python all over again
*Close CD tray	- done, used mci tho.
**/

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <mmsystem.h> // for MCI functions
#include <WinIoCtl.h>
//#include <stdio.h>

#define MAX_THREADS 26

#pragma comment(lib,"Winmm")
#pragma comment(lib,"Kernel32")
#pragma comment(lib,"User32")


bool ejectDiskWINAPI(TCHAR driveLetter[]);
bool actionDiskMCI(TCHAR driveLetter[], DWORD action);
DWORD WINAPI cd_trol(LPVOID args);
DWORD WINAPI usb_trol(LPVOID args);

int main(int argc, char * argv[])
{
	//ditch console :)
	FreeConsole();

	while (true) {
		//initiall delay, prevents too much taskbar flashing
		Sleep(5000);
		//threads arrays
		HANDLE hThreadArray[MAX_THREADS];
		DWORD dwThreadIdArray[MAX_THREADS];
		int t_c = 0;
		//size of and buffer to store drive letters 
		DWORD buffsize;
		TCHAR* buff;
		
		//size needed for buffer
		buffsize = GetLogicalDriveStrings(0, NULL);
		//allocate memory
		buff = (TCHAR*)malloc((buffsize + 1) * sizeof(TCHAR));
		//populate buffer with drives
		GetLogicalDriveStrings(buffsize, buff);
		//iterate through buffer
		while (*buff) {
//			printf("[%s]\n", buff);
			switch (GetDriveType(buff))
			{
			//case DRIVE_FIXED:
			//	printf("hdd,ignore\n");
			//	break;
			case DRIVE_CDROM:
				hThreadArray[t_c] = CreateThread(NULL, 0, cd_trol, buff, 0, &dwThreadIdArray[t_c]);
				t_c += 1;
				break;
			case DRIVE_REMOVABLE:
//				printf("boring old usb. kill it\n");
				hThreadArray[t_c] = CreateThread(NULL, 0, usb_trol, buff, 0, &dwThreadIdArray[t_c]);
				t_c += 1;
				break;
			//case DRIVE_REMOTE:
			//	printf("network drive. blag\n");
			//	break;
			default:
			//	printf("dunno.\n");
				break;
			}
			buff += lstrlen(buff) + 1;
		}
		WaitForMultipleObjects(
			t_c,           // number of objects in array
			hThreadArray,     // array of objects
			TRUE,       // wait for any object
			INFINITE);       // five-second wait
	}
}

bool ejectDiskWINAPI(TCHAR driveLetter[])
{
	//printf("%.1s\n", driveLetter);
	//TCHAR tmp[10]; 
	
	TCHAR tmp[10];
	_stprintf(tmp, _T("\\\\.\\%.1s:"), driveLetter);
	//sprintf(tmp, "\\\\.\\%c", (char)driveLetter);
	//wprintf(L"%s\n", tmp);
	HANDLE handle = CreateFile(tmp, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (!handle) {
		//printf("did not get handle\n%d\n",GetLastError());
		return 1;
	}

	DWORD bytes = 0;
	if(DeviceIoControl(handle, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &bytes, 0) == 0){
		//printf("%s ejection failed :(\n%d\n",driveLetter, GetLastError());
		return 1;
	}
	DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &bytes, 0);
	DeviceIoControl(handle, IOCTL_STORAGE_EJECT_MEDIA, 0, 0, 0, 0, &bytes, 0);
	CloseHandle(handle);
	return 0;
}

bool actionDiskMCI(TCHAR driveLetter[], DWORD action) {
	// Not used here, only for debug
	MCIERROR mciError = 0;

	// Flags for MCI command
	DWORD mciFlags = MCI_WAIT | MCI_OPEN_SHAREABLE |
		MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT;

	// Open drive device and get device ID
	TCHAR* elementName = { driveLetter };
	MCI_OPEN_PARMS mciOpenParms = { 0 };
	mciOpenParms.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_CD_AUDIO;
	mciOpenParms.lpstrElementName = elementName;
	mciError = mciSendCommand(0,
		MCI_OPEN, mciFlags, (DWORD_PTR)&mciOpenParms);

	// Eject or close tray using device ID
	MCI_SET_PARMS mciSetParms = { 0 };
	mciFlags = MCI_WAIT | action; // command is sent by caller
	mciError = mciSendCommand(mciOpenParms.wDeviceID,
		MCI_SET, mciFlags, (DWORD_PTR)&mciSetParms);

	// Close device ID
	mciFlags = MCI_WAIT;
	MCI_GENERIC_PARMS mciGenericParms = { 0 };
	mciError = mciSendCommand(mciOpenParms.wDeviceID,
		MCI_CLOSE, mciFlags, (DWORD_PTR)&mciGenericParms);
	return true;
}

DWORD WINAPI cd_trol(LPVOID args) {
	//while (true) {
//		printf("%s", args);
//		printf("sexy cdrom,pop out!\n");
		actionDiskMCI((TCHAR *) args, MCI_SET_DOOR_CLOSED);
		Sleep(500);
//		printf("sexy cdrom,hide away!\n");
		actionDiskMCI((TCHAR *) args, MCI_SET_DOOR_OPEN);
	//}
		return 0;
}

DWORD WINAPI usb_trol(LPVOID args) {
	TCHAR* buff;
	buff = (TCHAR *)args;
	//while (true) {
	//	Sleep(500);
		if (ejectDiskWINAPI(buff) == 0) {
//			printf("%s killed. no problemo\n", buff);
		}
		else {
			return 1;
} } //}
