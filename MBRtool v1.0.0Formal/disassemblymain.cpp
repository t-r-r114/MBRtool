#include "disassembler.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

int disassemblermain(int argc, char *argv[]) {

    DisassemblermodePtr -> ReadDisasmmode();
    DisassemblermodePtr-> WriteDisasmmode();

    printf("==== 反汇编参数 ====\n");
    printf("参数数量: %d\n", argc);
    printf("输入文件: %s\n", argv[1]);
    printf("输出文件: %s\n", argv[2]);
    printf("当前反汇编模式: %s\n", DisassemblermodePtr->g_disasm_mode == 16 ? "16位模式" : "32位模式");
    printf("====================\n\n");
    if (argc < 2) {
        printf("错误：缺少输入文件\n");
        printf("用法: %s <输入文件.bin> [输出文件.trrsw]\n", argv[0]);
        printf("Usage: %s <input.bin>\n", argv[0]);
        return 1;
    }

    // 读取二进制文件
    FILE *bin_file = fopen(argv[1], "rb");
    if (!bin_file) {
        fprintf(stderr, "无法打开输入文件: %s\n", argv[1]);
        perror("详细错误");
        fprintf(stderr, "Error opening input file: %s\n", strerror(errno));
        return 2;
    }

    fseek(bin_file, 0, SEEK_END);
    long file_size = ftell(bin_file);
    rewind(bin_file);

    uint8_t *buffer = static_cast<uint8_t*>(malloc(file_size));
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed for %ld bytes\n", file_size);
        fclose(bin_file);
        return 1;
    }

    if (fread(buffer, 1, file_size, bin_file) != static_cast<size_t>(file_size)) {
        fprintf(stderr, "Error reading file: %s\n", strerror(errno));
        free(buffer);
        fclose(bin_file);
        return 1;
    }
    fclose(bin_file);

    // 创建输出文件
    FILE *out_file = fopen("Disassembly.trrsw", "w");
    if (!out_file) {
        fprintf(stderr, "Error creating output file: %s\n", strerror(errno));
        free(buffer);
        return 1;
    }

    size_t post = 0;
    while (post < file_size) {
        size_t inst_size = file_size - post;
        disassemble_instruction(post, buffer + post, &inst_size);
        post += inst_size;
    }

    // 反汇编主循环
    size_t pos = 0;

    const size_t total_size = static_cast<size_t>(file_size);

    fprintf(out_file, "; Disassembly of %s\n", argv[1]);
    fprintf(out_file, "; Size: %ld bytes\n\n", file_size);

    while (pos < total_size) {

        if (labelManager.has_label(pos)) {
            // 输出标签和引用信息
            fprintf(out_file, "\n%s: ; Referenced from: ", labelManager.get_label(pos).c_str());

            auto refs = labelManager.get_references_to(pos);
            for (auto ref : refs) {
                fprintf(out_file, "0x%08zX ", ref);
            }
            fprintf(out_file, "\n");
        }

        if (pos >= 0x1BE && pos < 0x1FE) {
            if (pos == 0x1BE) {
                fprintf(out_file, "; Partition Table\n");
            }

            // 解析分区表项
            uint8_t status = buffer[pos];
            uint8_t start_head = buffer[pos+1];
            uint16_t start_sector_cylinder = *(uint16_t*)(buffer+pos+2);
            uint8_t type = buffer[pos+4];
            uint8_t end_head = buffer[pos+5];
            uint16_t end_sector_cylinder = *(uint16_t*)(buffer+pos+6);
            uint32_t lba_start = *(uint32_t*)(buffer+pos+8);
            uint32_t num_sectors = *(uint32_t*)(buffer+pos+12);

            fprintf(out_file, "%08zX: ", pos);
            for (int i = 0; i < 16; i++) {
                fprintf(out_file, "%02X ", buffer[pos+i]);
            }
            fprintf(out_file, "; Partition %zu: Type=0x%02X, Start=%u, Sectors=%u\n",
                    (pos - 0x1BE)/16 + 1, type, lba_start, num_sectors);

            pos += 16;
            continue;
        }

        // MBR签名 (0x1FE-0x1FF)
        if (pos == 0x1FE) {
            uint16_t signature = *(uint16_t*)(buffer+pos);
            fprintf(out_file, "%08zX: %02X %02X ; MBR Signature: 0x%04X\n",
                    pos, buffer[pos], buffer[pos+1], signature);
            pos += 2;
            continue;
        }

        // 错误消息字符串 (0x164-0x1BD)
        if (pos >= 0x164 && pos < 0x1BE && std::isprint(buffer[pos])) {
            size_t str_start = pos;
            while (pos < 0x1BE && buffer[pos] != 0) {
                pos++;
            }

            fprintf(out_file, "%08zX: ", str_start);
            for (size_t i = str_start; i < pos; i++) {
                fprintf(out_file, "%02X ", buffer[i]);
            }
            fprintf(out_file, "; \"%.*s\"\n", (int)(pos - str_start), buffer + str_start);

            if (buffer[pos] == 0) pos++; // 跳过null终止符
            continue;
        }

        size_t remaining = total_size - pos;
        size_t inst_size = remaining;  // 初始化为剩余大小

        // 修复：添加 base_addr 参数 (pos)
        const char *assembly = disassemble_instruction(pos, buffer + pos, &inst_size);

        // 确保不会读取超出缓冲区
        if (inst_size > remaining) {
            inst_size = remaining;
        }

        // 写入地址和汇编代码
        fprintf(out_file, "%08zX: ", pos);
        for (size_t i = 0; i < inst_size && i < MAX_INSTR_LEN; i++) {
            fprintf(out_file, "%02X ", buffer[pos + i]);
        }
        // 对齐
        for (size_t i = inst_size; i < MAX_INSTR_LEN; i++) {
            fputs("   ", out_file);
        }

        fprintf(out_file, "%-20s", assembly);

        // 添加注释解释跳转目标
        if (strstr(assembly, "J") == assembly || strstr(assembly, "CALL") != nullptr) {
            // 查找跳转目标地址
            const char *target_str = strstr(assembly, "0x");
            if (target_str) {
                char *endptr;
                uint64_t target = strtoull(target_str + 2, &endptr, 16);
                if (endptr != target_str + 2 && target < total_size) {
                    fprintf(out_file, " ; -> %08zX", (size_t)target);
                }
            }
        }

        fputc('\n', out_file);
        pos += inst_size;
    }

    fclose(out_file);
    free(buffer);
    printf("Disassembly complete. Output saved to Disassembly.trrsw\n");
    return 0;
}
