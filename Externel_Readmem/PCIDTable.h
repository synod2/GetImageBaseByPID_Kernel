#pragma once

#include "header.h"

DWORD64 GetImageBase(DWORD64 Tpid);
DWORD64 ReadProcessMemory(DWORD64* Addr);
PVOID DecodeTableData(PVOID* EncodeAddr);
DWORD64 TableLayer1(PVOID TablePointer, DWORD64 Tpid);
DWORD64 TableLayer2(PVOID* TablePointer, DWORD64 Tpid);
DWORD64 TraversePspCidTable(DWORD64* PspCidTable, DWORD64 Tpid);
DWORD64 GetPspCidTableAddress();
PEPROCESS TargetProcess = nullptr;

DWORD64 GetImageBase(DWORD64 Tpid) {
	dmsg("Start PCID Table Traverse");
	DWORD64 PspCidTableAddress = 0;
	PspCidTableAddress = GetPspCidTableAddress();
	dmsg("PspCidTable : %llx", PspCidTableAddress);
	return TraversePspCidTable((DWORD64*)PspCidTableAddress, Tpid);
}

DWORD64 ReadProcessMemory(DWORD64* Addr) {
	DWORD64 ret;
	if (TargetProcess == nullptr) {
		return 0;
	}
	else {
		KeAttachProcess(TargetProcess);
		ret = *Addr;
		KeDetachProcess(TargetProcess);
		return ret;
	}
}
//���̺� ������ ���ڵ� �Լ� 
PVOID DecodeTableData(PVOID* EncodeAddr) {
	PVOID retResult = nullptr;

	retResult = (PVOID)(((ULONGLONG)((ULONGLONG)*EncodeAddr >> 0x10) + 0xFFFF000000000000) & 0xFFFFFFFFFFFFFFF0);
	return retResult;
};

//TableLayer1�ܰ�
DWORD64 TableLayer1(PVOID TablePointer, DWORD64 Tpid) {
	PVOID* TargetAddress = nullptr;
	/*ULONGLONG PID = 0;*/
	HANDLE PID = nullptr;
	UCHAR* PName = nullptr;
	UNICODE_STRING targetFName;

	//PsGetProcessImageFileName �Լ� ������ �������� 
	RtlInitUnicodeString(&targetFName, L"PsGetProcessImageFileName");
	PsGetProcessImageFileName = (PsGetProcessImageFileName_t)MmGetSystemRoutineAddress(&targetFName);

	//PsGetProcessImageFileName �Լ� ������ �������� 
	RtlInitUnicodeString(&targetFName, L"PsGetProcessPeb");
	PsGetProcessPeb = (PsGetProcessPeb_t)MmGetSystemRoutineAddress(&targetFName);
	
	for (DWORD64 i = 0; i < 256; i++) {
		TargetAddress = (PVOID*)((DWORD64)TablePointer + (i * 16));

		if (*TargetAddress) {
			TargetProcess = (PEPROCESS)DecodeTableData(TargetAddress);
			//������ ���� �ٸ� ������ �̿��ؼ� ã�ƿ� �� �ְ� �籸�� �ʿ��� 

			PID = PsGetProcessId(TargetProcess);

			if ((DWORD64)PID < 0xE0000000 && (PID > 0)) {
				if (Tpid == (DWORD64)PID) {
					PName = PsGetProcessImageFileName(TargetProcess);
					dmsg("**** PNAME : %s - PID : %llx **** \n", PName, PID);
					DWORD64* get_peb = PsGetProcessPeb(TargetProcess);
					DWORD64 ImageBase = ReadProcessMemory((DWORD64*)((DWORD64)get_peb + 0x10));
					dmsg("I.B : %p\n", ImageBase);
					return ImageBase;
				}
			}
		}
	}
	return 0;
};

//TableLayer2�ܰ�
DWORD64 TableLayer2(PVOID* TablePointer, DWORD64 Tpid) {
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "**** TableLayer Level 2 Traverse **** \n");
	DWORD64 ret = 0;
	for (DWORD64 i = 0; i < 512; i++) {
		//��� �ּ� ������ �� �� �����ϴ��� üũ 
		PVOID* TargetAddress = (PVOID*)((DWORD64)TablePointer + (i * 8));
		if (*TargetAddress == 0) {
			break;
		}
		else {
			/*DbgPrintEx(DPFLTR_IHVDRIVER_ID, 0, "----- TableLayer Level 2 Traverse : %d ----- \n",i);*/
			DWORD64 tmp = TableLayer1((PVOID)*TargetAddress, Tpid);
			if (tmp)
				ret = tmp;
		}
	}
	return ret;
};

//TableLayer3�ܰ�
void TableLayer3(PVOID* TablePointer) {

};

//PspCidTable �ּ� ���޹޾� ���̺� ���̾� ������ ������ ��ȸ
DWORD64 TraversePspCidTable(DWORD64* PspCidTable, DWORD64 Tpid) {
	PVOID* TableCode = nullptr;
	PVOID* TableAddr = nullptr;
	DWORD64 ret = 0;
	int LayerLevel = 0;

	//TableCode ã�ƿ���
	TableCode = (PVOID*)((DWORD64)(*PspCidTable) + 0x8);

	//TableCode���� LayerLevel �и�
	LayerLevel = (DWORD64)(*TableCode) & 3;
	TableAddr = (PVOID*)((DWORD64)(*TableCode) - LayerLevel);

	dmsg("**** LayerLevel: %d **** \n", LayerLevel + 1);

	switch (LayerLevel) {
	case 0:
		//TableLayer1(TableAddr);
		break;
	case 1:
		ret = TableLayer2(TableAddr, Tpid);
		break;
	case 2:
		break;
	}
	return ret;
};


//PspCidTable ���� �ּ� ã�ƿ���
DWORD64 GetPspCidTableAddress() {
	UNICODE_STRING targetFName;
	PVOID PsLookupProcessByProcessId = nullptr;
	PVOID PspReferenceCidTableEntry = nullptr;
	PVOID* tmpAddr = nullptr;
	DWORD64 FuncOffset = 0;
	DWORD64 PspCidTableOffset = 0;

	//PsLookupProcessByProcessId �Լ� ������ �������� 
	RtlInitUnicodeString(&targetFName, L"PsLookupProcessByProcessId");
	PsLookupProcessByProcessId = MmGetSystemRoutineAddress(&targetFName);

	//PspReferenceCidTableEntry ã������ ����Ʈ�ڵ忡�� ����ּ� ������ ã�ƿ�.
	tmpAddr = (PVOID*)((DWORD64)PsLookupProcessByProcessId + 0x25);
	FuncOffset = (int)*tmpAddr;

	//PspReferenceCidTableEntry �ּ� ����
	PspReferenceCidTableEntry = (PVOID)((DWORD64)PsLookupProcessByProcessId + 0x29 + FuncOffset);

	//PspCidTable ����ּ� ������ ã�ƿ�
	tmpAddr = (PVOID*)((DWORD64)PspReferenceCidTableEntry + 0x1d);
	FuncOffset = (int)*tmpAddr;

	PspCidTableOffset = (DWORD64)((DWORD64)PspReferenceCidTableEntry + 0x21 + FuncOffset);

	return PspCidTableOffset;
}