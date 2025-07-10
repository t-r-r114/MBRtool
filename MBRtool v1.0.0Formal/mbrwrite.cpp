#include "mbrwrite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <QGuiApplication>
#include <mbrwrite.h>
#include <QQmlApplicationEngine>
#include <QObject>
#include <qqmlengine.h>
#include <QQmlContext>

MBRwrite::MBRwrite(QObject *parent)
    : QObject{parent}
{}

int MBRwrite::mBRWRITE() {
    // 1. 显示 Windows 警告提示窗口，让用户确认操作
    int reply = MessageBoxW(
        NULL,
        L"即将写入 MBR，此操作可能导致系统无法启动！\n是否继续？",
        L"警告",
        MB_YESNO | MB_ICONWARNING
        );

    if (reply != IDYES) {
        printf("用户取消操作，未写入 MBR。\n");
        return 1;
    }

    // 2. 读取备份文件
    BYTE backupMBR[512];
    FILE* file = fopen("mbr_backup.bin", "rb");
    if (!file || fread(backupMBR, 1, sizeof(backupMBR), file) != sizeof(backupMBR)) {
        printf("读取备份文件失败!\n");
        if (file) fclose(file);
        MessageBoxW(NULL, L"无法读取备份文件！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }
    fclose(file);

    // 3. 打开硬盘设备（使用ANSI版本）
    HANDLE hDisk = CreateFileA(
        "\\\\.\\PhysicalDrive0",  // ANSI字符串
        GENERIC_READ | GENERIC_WRITE,       // 读写权限
        FILE_SHARE_READ | FILE_SHARE_WRITE,// 共享权限
        NULL,                                 // 安全属性
        OPEN_EXISTING,                        // 存在则打开
        FILE_ATTRIBUTE_NORMAL,                // 普通文件
        NULL                                  // 默认属性
        );

    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("无法打开硬盘设备! 错误代码: %d\n", GetLastError());
        MessageBoxW(NULL, L"无法访问硬盘设备！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 4. 写入 MBR（使用ANSI写入方法）
    DWORD bytesWritten = 0;
    if (!WriteFile(hDisk, backupMBR, sizeof(backupMBR), &bytesWritten, NULL)) {
        printf("写入MBR失败! 错误代码: %d\n", GetLastError());
        CloseHandle(hDisk);
        MessageBoxW(NULL, L"写入 MBR 失败！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 5. 解锁卷（可选操作，增强兼容性）
    DWORD bytesReturned = 0;
    DeviceIoControl(
        hDisk,
        FSCTL_UNLOCK_VOLUME,
        NULL,
        0,
        NULL,
        0,
        &bytesReturned,
        NULL
        );

    // 6. 验证写入内容
    BYTE currentMBR[512];
    DWORD bytesRead = 0;

    // 重置文件指针到磁盘开头
    SetFilePointer(hDisk, 0, NULL, FILE_BEGIN);

    if (!ReadFile(hDisk, currentMBR, sizeof(currentMBR), &bytesRead, NULL)) {
        printf("验证失败: 无法读取MBR! 错误代码: %d\n", GetLastError());
        CloseHandle(hDisk);
        MessageBoxW(NULL, L"验证 MBR 失败！", L"错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    // 7. 比较备份与写入内容
    bool isConsistent = (memcmp(backupMBR, currentMBR, sizeof(backupMBR)) == 0);
    CloseHandle(hDisk);

    // 8. 显示结果
    if (isConsistent) {
        printf("MBR恢复成功! 写入内容已验证一致。\n");
        MessageBoxW(NULL, L"MBR 恢复成功！", L"成功", MB_OK | MB_ICONINFORMATION);
        return 0;
    } else {
        printf("警告: MBR验证失败! 硬盘内容与备份不一致。\n");
        MessageBoxW(
            NULL,
            L"MBR 写入后验证失败！\n"
            L"可能原因：\n"
            L"1) 磁盘损坏\n"
            L"2) 权限不足\n"
            L"3) 其他程序干扰",
            L"警告",
            MB_OK | MB_ICONWARNING
            );
        return 1;
    }
}
