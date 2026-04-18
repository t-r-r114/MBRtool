#include "disassemblywindows.h"
#include "disassembler.h"
#include <stdio.h>
#include <vector>
#include <string>

static int g_targetDisk = 1;

DisassemblyWindows::DisassemblyWindows(QObject *parent)
    : QObject{parent}
{}

void DisassemblyWindows::setTargetDisk(int diskNumber) {
    g_targetDisk = diskNumber;
}

int DisassemblyWindows::getTargetDisk() {
    return g_targetDisk;
}

bool DisassemblyWindows::startDualDisassembly(const QString &inputFile){
    // 动态拼接出对应磁盘号的 bin 文件名
    std::string input = "mbr_backup_disk" + std::to_string(g_targetDisk) + ".bin";

    DisassemblermodePtr->g_disasm_mode = 16;
    labelManager.clear();
    std::vector<char*> args16 = {
        (char*)"mbr_tool",
        const_cast<char*>(input.c_str()),
        (char*)"Disassembly_16.trrsw"
    };
    args16.push_back(nullptr);
    disassemblermain(3, args16.data());

    // --- 第二步：执行 32 位反汇编 ---
    DisassemblermodePtr->g_disasm_mode = 32;
    labelManager.clear();

    std::vector<char*> args32 = {
        (char*)"mbr_tool",
        const_cast<char*>(input.c_str()),
        (char*)"Disassembly_32.trrsw"
    };
    args32.push_back(nullptr);
    disassemblermain(3, args32.data());

    printf("双重反汇编完成！目标文件：%s\n", input.c_str());
    return true;
}

int DisassemblyWindows::disassemblyWindows(){
    int argc = 3;
    // 动态拼接单步反汇编的目标文件名
    std::string inputFile = "mbr_backup_disk" + std::to_string(g_targetDisk) + ".bin";

    std::vector<std::string> arguments = {
        "mbr_tool",
        inputFile,           // 使用动态文件名
        "Disassembly.trrsw"
    };

    std::vector<char*> argv;
    for (auto& arg : arguments) {
        argv.push_back(const_cast<char*>(arg.data()));
    }
    argv.push_back(nullptr);

    int result = disassemblermain(argc, argv.data());

    if (result == 0) {
        printf("反汇编成功！结果保存在 Disassembly.trrsw\n");
    } else {
        printf("错误码: %d\n", result);
    }

    return result;
}

void DisassemblyWindows::setMode(int mode) {
    if (mode == 16) {
        DisassemblermodePtr->g_disasm_mode = 16;
        DisassemblermodePtr->WriteDisasmmode(); // 写入 ini 文件保存
        printf("已切换到 16 位模式\n");
    } else if (mode == 32) {
        DisassemblermodePtr->g_disasm_mode = 32;
        DisassemblermodePtr->WriteDisasmmode(); // 写入 ini 文件保存
        printf("已切换到 32 位模式\n");
    }
}
