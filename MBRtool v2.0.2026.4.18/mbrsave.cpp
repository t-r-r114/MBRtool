#include "mbrsave.h"
#include <stdio.h>
#include <windows.h>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QObject>
#include <qqmlengine.h>
#include <QQmlContext>

MBRsave::MBRsave(QObject *parent)
    : QObject{parent}
{}

int MBRsave::mBRSAVE(int diskNumber){
    // 动态生成硬盘设备路径
    QString pathStr = QString("\\\\.\\PhysicalDrive%1").arg(diskNumber);
    std::string stdPath = pathStr.toStdString();
    LPCSTR diskPath = stdPath.c_str();

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
    QString fileName = QString("mbr_backup_disk%1.bin").arg(diskNumber);
    FILE* file = fopen(fileName.toLocal8Bit().constData(), "wb");
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
