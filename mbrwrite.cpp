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

int MBRSAVE(){
    // 硬盘设备路径（PhysicalDrive0表示第一个物理硬盘）
    LPCSTR diskPath = "\\\\.\\PhysicalDrive0";

    // 用于存储MBR数据的缓冲区（512字节）
    BYTE mbr[512] = {0};
    DWORD bytesRead = 0;

    // 打开硬盘设备（只读模式）
    HANDLE hDisk = CreateFileA(
        diskPath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
        );

    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("无法打开硬盘设备! 错误代码: %d\n", GetLastError());
        return 1;
    }

    // 读取MBR（第一个扇区）
    if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
        printf("读取MBR失败! 错误代码: %d\n", GetLastError());
        CloseHandle(hDisk);
        return 1;
    }

    // 确保读取了完整的512字节
    if (bytesRead != sizeof(mbr)) {
        printf("未读取完整的MBR数据! 只读取了%d字节\n", bytesRead);
        CloseHandle(hDisk);
        return 1;
    }

    // 将MBR数据保存到文件
    FILE* file = fopen("mbr_backup.bin", "wb");
    if (file == NULL) {
        printf("无法创建备份文件!\n");
        CloseHandle(hDisk);
        return 1;
    }

    fwrite(mbr, sizeof(BYTE), sizeof(mbr), file);
    fclose(file);

    CloseHandle(hDisk);

    printf("MBR已成功备份到mbr_backup.bin文件\n");
    return 0;
}

int MBRcheck(){
    if(MBRSAVE() == 0){
        //比较mbr_backup.bin和当前MBR
        LPCSTR diskPath = "\\\\.\\PhysicalDrive0";
        BYTE mbr[512];
        DWORD bytesRead = 0;
        // 优化后的代码片段
        HANDLE hDisk = CreateFileA(diskPath, 
          GENERIC_READ,                    // 只需要读取权限
          FILE_SHARE_READ | FILE_SHARE_WRITE,
           NULL, 
          OPEN_EXISTING, 
          FILE_ATTRIBUTE_NORMAL,
          NULL);
        if (hDisk == INVALID_HANDLE_VALUE) {
            printf("无法打开硬盘设备! 错误代码: %d\n", GetLastError());
            return 1;
        }
        if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
            printf("读取MBR失败! 错误代码: %d\n", GetLastError());
            CloseHandle(hDisk);
            return 1;
        }
        CloseHandle(hDisk);
        // 读取备份文件
        hDisk = CreateFileA(diskPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDisk == INVALID_HANDLE_VALUE) {
            printf("无法打开硬盘设备! 错误代码: %d\n", GetLastError());
            return 1;
        }
        FILE* file = fopen("mbr_backup.bin", "rb");
        if (file == NULL) {
            printf("无法打开备份文件! 错误代码: %d\n", GetLastError());
            return 1;
        }
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* buffer = (char*)malloc(fileSize);
        fread(buffer, 1, fileSize, file);
        fclose(file);
        DWORD bytesWritten;
        // 比较MBR和备份文件
        if (fileSize != sizeof(mbr)) {
            printf("备份文件大小不正确!\n");
            free(buffer);
            return 1;
        }
        for (int i = 0; i < fileSize; i++) {
            if (buffer[i] != mbr[i]) {
                printf("MBR和备份文件不一致!\n");
                free(buffer);
                return 1;
            }
        }
        printf("MBR和备份文件一致!\n");
        free(buffer);
        return 0;
    }else {
        printf("无法打开文件!\n");
        return 1; 
        }
}

int MBRwrite::mBRWRITE(){
    LPCSTR diskPath = "\\\\.\\PhysicalDrive0";
    BYTE mbr[512];
    DWORD bytesWritten = 0;

    // 读取备份文件
    FILE* file = fopen("mbr_backup.bin", "rb");
    if (!file || fread(mbr, 1, sizeof(mbr), file) != sizeof(mbr)) {
        printf("读取备份文件失败!\n");
        return 1;
    }
    fclose(file);

    // 写入硬盘
    HANDLE hDisk = CreateFileA(diskPath, 
        GENERIC_WRITE,                  // 需要写入权限
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("无法打开硬盘设备! 错误代码: %d\n", GetLastError());
        return 1;
    }

    if (!WriteFile(hDisk, mbr, sizeof(mbr), &bytesWritten, NULL)) {
        printf("写入MBR失败! 错误代码: %d\n", GetLastError());
        CloseHandle(hDisk);
        return 1;
    }

    if(MBRcheck() != 0){
        printf("MBR没有正确写入!\n");
        printf("请检查备份文件是否正确!\n");
        CloseHandle(hDisk);
        return 1;
    }

    printf("写入MBR成功! 已写入 %d 个字节\n", bytesWritten);

    CloseHandle(hDisk);
    printf("MBR恢复成功!\n");
    return 0;
}
