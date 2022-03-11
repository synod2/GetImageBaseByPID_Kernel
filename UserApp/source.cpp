#include "header.h"

VOID sendBuffer(PVOID buf) {

	BOOL DICret;
	WCHAR inputBuffer[100];
	WCHAR outputBuffer[100];
	ULONG bytesret;
	DWORD64 mLen;

	DICret = DeviceIoControl(hHandle,
		IOCTL_IMGBASE,
		buf,
		sizeof(buf),
		&outputBuffer,
		sizeof(outputBuffer),
		&bytesret,
		NULL);

	if (!DICret) {
		printf("Device Io Control Error \n");
	}
	printf("outdata : %016llx\n", *(DWORD64*)outputBuffer);
	

};


int main() {
	BOOL DICret;
	WCHAR inputBuffer[100];
	DWORD64 inputTmp;
	WCHAR outputBuffer[100];
	ULONG bytesret;
	int mLen;

	hHandle = CreateFile(path,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hHandle == INVALID_HANDLE_VALUE) {
		printf("Open Handle ERROR\n");
		return 0;
	}
	/*wcscpy(inputBuffer, L"0x123123");
	mLen = (DWORD)wcslen(inputBuffer) * 2 + 1;*/
	inputTmp = 2076;
	sendBuffer(&inputTmp);

	return 0;
}