#include "PCIDTable.h"

MAL_GLOBAL g_MalGlobal;


void DriverInit(PDRIVER_OBJECT pDriver) {
	UNICODE_STRING	 NtQuerySystemInformationName;
	NTSTATUS retStatus = STATUS_SUCCESS;
	ULONG BufferSize, BackwardPID = 0;
	
	/*DWORD64 Tpid = 0x81c;*/
	UNICODE_STRING targetFName;
	UNICODE_STRING   deviceSymLink;
	UNICODE_STRING   deviceName;
	PDEVICE_OBJECT deviceObject = NULL;

	//디버그 메시지 출력
	dmsg("Driver Loaded!");

	//전역변수 구조체 초기화 
	RtlZeroMemory(&g_MalGlobal, sizeof(MAL_GLOBAL));

	//구조체에 드라이버 오브젝트 저장 
	g_MalGlobal.DriverObject = pDriver;

	//유니코드 타입 문자열 초기화  -> IoCreateSymbolicLink 함수 인자로 유니코드 타입 문자열 넘어가야함 
	RtlInitUnicodeString(&deviceSymLink, SYMLINK);
	RtlInitUnicodeString(&deviceName, L"\\Device\\testName");

	//PDEVICE_OBJECT 생성 
	retStatus = IoCreateDevice(
		pDriver,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&deviceObject);

	//NTSTATUS 결과 SUCCESS 인지 검사 
	if (!NT_SUCCESS(retStatus)) {
		dmsg("IoCreateDevice Fail!");
		return ;
	}

	//구조체에 디바이스 오브젝트 저장 
	g_MalGlobal.DeviceObject = deviceObject;

	//심볼릭 링크 생성 
	retStatus = IoCreateSymbolicLink(&deviceSymLink, &deviceName);

	//NTSTATUS 결과 SUCCESS 인지 검사 
	if (!NT_SUCCESS(retStatus)) {
		dmsg("IoCreateSymLink Fail!");
		return ;
	}

	//KeAttachProcess 함수 포인터 가져오기 
	RtlInitUnicodeString(&targetFName, L"KeAttachProcess");
	KeAttachProcess = (KeAttachProcess_t)MmGetSystemRoutineAddress(&targetFName);

	//KeDetachProcess 함수 포인터 가져오기 
	RtlInitUnicodeString(&targetFName, L"KeDetachProcess");
	KeDetachProcess = (KeDetachProcess_t)MmGetSystemRoutineAddress(&targetFName);

};


NTSTATUS myIOCTL(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp) {
	PIO_STACK_LOCATION pStack;
	NTSTATUS retStatus = STATUS_SUCCESS;
	ULONG ControlCode;
	PCWSTR  RawBuf;
	UNICODE_STRING InBuf;
	DWORD64 BufAddr = 0;
	PWCHAR OutBuf;
	PWCHAR OutMsg = L"Message from driver\n";
	ULONG InBufLength, OutBufLength, OutMsgLength;
	dmsg("start IOCTL");
	//IRP 내부에 존재하는 IO스택 포인터위치 반환 
	pStack = IoGetCurrentIrpStackLocation(irp);

	InBufLength = pStack->Parameters.DeviceIoControl.InputBufferLength;
	OutBufLength = pStack->Parameters.DeviceIoControl.OutputBufferLength;

	//유저 어플리케이션에서 전달한 컨트롤 코드 찾아옴
	ControlCode = pStack->Parameters.DeviceIoControl.IoControlCode;
	if (!ControlCode)
	{ 
		dmsg("Get Control Code Error!");
		return STATUS_UNSUCCESSFUL;
	}

	DWORD64* SysBuf = (DWORD64*)irp->AssociatedIrp.SystemBuffer;
	PVOID Outbuf = irp->AssociatedIrp.SystemBuffer;;
	DWORD64 ImageBase = 0;
	/*__debugbreak();*/
	
	switch (ControlCode) {
	case IOCTL_IMGBASE :
		dmsg("IOCTL get ImgBase : %d", *SysBuf);
		ImageBase = GetImageBase(*SysBuf);
		RtlCopyBytes(Outbuf,&ImageBase, sizeof(ImageBase));
		dmsg("outbytes : %llx", ImageBase);
		dmsg("outbytes : %p", &ImageBase);
		dmsg("outbytes : %llx", *(DWORD64*)Outbuf);
		irp->IoStatus.Information = sizeof(ImageBase);

		break;
	case IOCTL_READMEM:
		dmsg("Get ReadProcMem");		
		break;
	
	}

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return retStatus;
}


//Create 대응 함수 - 함수동작 성공시 STATUS_SUCESS를 반환해주어야 한다. 
NTSTATUS createHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp) {
	//I/O 상태 변경 
	irp->IoStatus.Status = STATUS_SUCCESS;

	//IRP 동작 완료 처리 함수 
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	dmsg("CREATE EVENT!");
	return STATUS_SUCCESS;
}


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pRegPath) {
	UNREFERENCED_PARAMETER(pRegPath);
	UNREFERENCED_PARAMETER(pDriver);

	NTSTATUS ret = STATUS_SUCCESS;

	dmsg("Hello World!");
	DriverInit(pDriver);

	pDriver->DriverUnload = UnloadDriver;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = myIOCTL;
	pDriver->MajorFunction[IRP_MJ_CREATE] = createHandler;

	return ret;
}

VOID UnloadDriver(IN PDRIVER_OBJECT pDriver) {
	UNREFERENCED_PARAMETER(pDriver);
	dmsg("====Driver Unloaded====");
	if (g_MalGlobal.DeviceObject != nullptr)
	{
		//심볼릭 링크 값 초기화 해주고 변수 할당 해제
		UNICODE_STRING Symlink = { 0, };
		RtlInitUnicodeString(&Symlink, SYMLINK);
		IoDeleteSymbolicLink(&Symlink);

		//디바이스 오브젝트 삭제 
		IoDeleteDevice(g_MalGlobal.DeviceObject);
	}
	return;
}
