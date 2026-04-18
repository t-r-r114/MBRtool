#pragma once

#include <windows.h>
#include <winioctl.h>

// {1facfca1-9f67-49e8-a9a8-bf86e97d13a3}
DEFINE_GUID (GUID_DEVINTERFACE_Writer,
            0x1facfca1,0x9f67,0x49e8,0xa9,0xa8,0xbf,0x86,0xe9,0x7d,0x13,0xa3);

#define IOCTL_DISK_WRITE_COMMAND \
CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack(push, 8)
typedef struct _DISK_WRITE_PARAMS {
    ULONG     DiskNumber;
    ULONGLONG ByteOffset;
    ULONG     WriteLength;
    UCHAR     Data[ANYSIZE_ARRAY]; // 变长数组，适配内核层
} DISK_WRITE_PARAMS, *PDISK_WRITE_PARAMS;
#pragma pack(pop)
