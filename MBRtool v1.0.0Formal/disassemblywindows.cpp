#include "disassemblywindows.h"
#include "disassembler.h"
#include <stdio.h>
#include <vector>
#include <string>

DisassemblyWindows::DisassemblyWindows(QObject *parent)
    : QObject{parent}
{}



int DisassemblyWindows::disassemblyWindows(){
    // 准备参数
    int argc = 3;
    std::vector<std::string> arguments = {
        "mbr_tool",          // 程序名称 (argv[0])
        "mbr_backup.bin",     // 输入文件 (argv[1])
        "Disassembly.trrsw"   // 输出文件 (argv[2])
    };

    // 转换为 C 风格参数数组
    std::vector<char*> argv;
    for (auto& arg : arguments) {
        argv.push_back(const_cast<char*>(arg.data()));
    }
    argv.push_back(nullptr); // 结束标记

    // 调用反汇编函数
    int result = disassemblermain(argc, argv.data());

    if (result == 0) {
        printf("反汇编成功！结果保存在 Disassembly.trrsw\n");
    } else if(result == 2){
        printf("错误码: %d\n", result);
    } else if(result == 1){
        printf("错误码: %d\n", result);
    }

    return result;
}
