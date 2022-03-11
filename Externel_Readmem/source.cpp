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

	//����� �޽��� ���
	dmsg("Driver Loaded!");

	//�������� ����ü �ʱ�ȭ 
	RtlZeroMemory(&g_MalGlobal, sizeof(MAL_GLOBAL));

	//����ü�� ����̹� ������Ʈ ���� 
	g_MalGlobal.DriverObject = pDriver;

	//�����ڵ� Ÿ�� ���ڿ� �ʱ�ȭ  -> IoCreateSymbolicLink �Լ� ���ڷ� �����ڵ� Ÿ�� ���ڿ� �Ѿ���� 
	RtlInitUnicodeString(&deviceSymLink, SYMLINK);
	RtlInitUnicodeString(&deviceName, L"\\Device\\testName");

	//PDEVICE_OBJECT ���� 
	retStatus = IoCreateDevice(
		pDriver,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&deviceObject);

	//NTSTATUS ��� SUCCESS ���� �˻� 
	if (!NT_SUCCESS(retStatus)) {
		dmsg("IoCreateDevice Fail!");
		return ;
	}

	//����ü�� ����̽� ������Ʈ ���� 
	g_MalGlobal.DeviceObject = deviceObject;

	//�ɺ��� ��ũ ���� 
	retStatus = IoCreateSymbolicLink(&deviceSymLink, &deviceName);

	//NTSTATUS ��� SUCCESS ���� �˻� 
	if (!NT_SUCCESS(retStatus)) {
		dmsg("IoCreateSymLink Fail!");
		return ;
	}

	//KeAttachProcess �Լ� ������ �������� 
	RtlInitUnicodeString(&targetFName, L"KeAttachProcess");
	KeAttachProcess = (KeAttachProcess_t)MmGetSystemRoutineAddress(&targetFName);

	//KeDetachProcess �Լ� ������ �������� 
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
	//IRP ���ο� �����ϴ� IO���� ��������ġ ��ȯ 
	pStack = IoGetCurrentIrpStackLocation(irp);

	InBufLength = pStack->Parameters.DeviceIoControl.InputBufferLength;
	OutBufLength = pStack->Parameters.DeviceIoControl.OutputBufferLength;

	//���� ���ø����̼ǿ��� ������ ��Ʈ�� �ڵ� ã�ƿ�
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


//Create ���� �Լ� - �Լ����� ������ STATUS_SUCESS�� ��ȯ���־�� �Ѵ�. 
NTSTATUS createHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp) {
	//I/O ���� ���� 
	irp->IoStatus.Status = STATUS_SUCCESS;

	//IRP ���� �Ϸ� ó�� �Լ� 
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
		//�ɺ��� ��ũ �� �ʱ�ȭ ���ְ� ���� �Ҵ� ����
		UNICODE_STRING Symlink = { 0, };
		RtlInitUnicodeString(&Symlink, SYMLINK);
		IoDeleteSymbolicLink(&Symlink);

		//����̽� ������Ʈ ���� 
		IoDeleteDevice(g_MalGlobal.DeviceObject);
	}
	return;
}
