#include "disassembler.h"
#include <cstring>
#include <cstdio>
#include <cctype>
#include <sstream>

struct InstructionDef {
    uint8_t opcode;
    const char *mnemonic_16;
    const char *mnemonic_32;
    int operand_size_16;
    int operand_size_32;
    int has_modrm;
};

// 指令定义表
static const InstructionInfo instructions[] = {
    // 单字节指令
    {0x50, "PUSH EAX", 0, 0}, {0x51, "PUSH ECX", 0, 0},
    {0x52, "PUSH EDX", 0, 0}, {0x53, "PUSH EBX", 0, 0},
    {0x54, "PUSH ESP", 0, 0}, {0x55, "PUSH EBP", 0, 0},
    {0x56, "PUSH ESI", 0, 0}, {0x57, "PUSH EDI", 0, 0},
    {0x58, "POP EAX", 0, 0}, {0x59, "POP ECX", 0, 0},
    {0x5A, "POP EDX", 0, 0}, {0x5B, "POP EBX", 0, 0},
    {0x5C, "POP ESP", 0, 0}, {0x5D, "POP EBP", 0, 0},
    {0x5E, "POP ESI", 0, 0}, {0x5F, "POP EDI", 0, 0},

    // 立即数指令
    {0x68, "PUSH", 4, 0},      // PUSH imm32
    {0x6A, "PUSH", 1, 0},      // PUSH imm8
    {0xB0, "MOV AL,", 1, 0},   {0xB1, "MOV CL,", 1, 0},
    {0xB2, "MOV DL,", 1, 0},   {0xB3, "MOV BL,", 1, 0},
    {0xB4, "MOV AH,", 1, 0},   {0xB5, "MOV CH,", 1, 0},
    {0xB6, "MOV DH,", 1, 0},   {0xB7, "MOV BH,", 1, 0},
    {0xB8, "MOV EAX,", 4, 0},  {0xB9, "MOV ECX,", 4, 0},
    {0xBA, "MOV EDX,", 4, 0},  {0xBB, "MOV EBX,", 4, 0},
    {0xBC, "MOV ESP,", 4, 0},  {0xBD, "MOV EBP,", 4, 0},
    {0xBE, "MOV ESI,", 4, 0},  {0xBF, "MOV EDI,", 4, 0},

    // 跳转指令
    {0x70, "JO", 1, 0},        {0x71, "JNO", 1, 0},
    {0x72, "JB", 1, 0},        {0x73, "JNB", 1, 0},
    {0x74, "JE", 1, 0},        {0x75, "JNE", 1, 0},
    {0x76, "JBE", 1, 0},       {0x77, "JA", 1, 0},
    {0x78, "JS", 1, 0},        {0x79, "JNS", 1, 0},
    {0x7A, "JP", 1, 0},        {0x7B, "JNP", 1, 0},
    {0x7C, "JL", 1, 0},        {0x7D, "JNL", 1, 0},
    {0x7E, "JLE", 1, 0},       {0x7F, "JG", 1, 0},
    {0xE8, "CALL", 4, 0},      // CALL rel32
    {0xE9, "JMP", 4, 0},       // JMP rel32
    {0xEB, "JMP", 1, 0},       // JMP rel8

    // 其他指令
    {0x90, "NOP", 0, 0},
    {0xC3, "RET", 0, 0},
    {0xC2, "RET", 2, 0},

    // 需要ModR/M的指令
    {0x00, "ADD", 0, 1},       {0x01, "ADD", 0, 1},
    {0x02, "ADD", 0, 1},       {0x03, "ADD", 0, 1},
    {0x08, "OR", 0, 1},        {0x09, "OR", 0, 1},
    {0x0A, "OR", 0, 1},        {0x0B, "OR", 0, 1},
    {0x20, "AND", 0, 1},       {0x21, "AND", 0, 1},
    {0x22, "AND", 0, 1},       {0x23, "AND", 0, 1},
    {0x30, "XOR", 0, 1},       {0x31, "XOR", 0, 1},
    {0x32, "XOR", 0, 1},       {0x33, "XOR", 0, 1},
    {0x38, "CMP", 0, 1},       {0x39, "CMP", 0, 1},
    {0x3A, "CMP", 0, 1},       {0x3B, "CMP", 0, 1},
    {0x88, "MOV", 0, 1},       {0x89, "MOV", 0, 1},
    {0x8A, "MOV", 0, 1},       {0x8B, "MOV", 0, 1},
    {0xFF, "CALL/JMP/PUSH", 0, 1},

    {0x8E, "MOV", 0, 1},      // MOV Sreg, r/m
    {0x88, "MOV", 0, 1},      // MOV r/m8, r8
    {0x89, "MOV", 0, 1},      // MOV r/m32, r32
    {0x8B, "MOV", 0, 1},      // MOV r32, r/m32
    {0xA0, "MOV AL,", 4, 0},  // MOV AL, moffs8
    {0xA1, "MOV EAX,", 4, 0}, // MOV EAX, moffs32
    {0xB0, "MOV AL,", 1, 0},  // MOV AL, imm8
    {0xB8, "MOV EAX,", 4, 0}, // MOV EAX, imm32

    // 控制转移指令
    {0xE8, "CALL", 4, 0},     // CALL rel32
    {0xE9, "JMP", 4, 0},      // JMP rel32
    {0xEB, "JMP", 1, 0},      // JMP rel8

    // 条件跳转
    {0x70, "JO", 1, 0},
    {0x71, "JNO", 1, 0},
    // ... 添加所有条件跳转

    // 特殊指令
    {0xCD, "INT", 1, 0},      // INT imm8
    {0xF4, "HLT", 0, 0},

    // 字符串操作
    {0xA4, "MOVSB", 0, 0},
    {0xA5, "MOVSD", 0, 0},
    {0xAA, "STOSB", 0, 0},

    {0x06, "PUSH ES", 0, 0},
    {0x07, "POP ES", 0, 0},
    {0x0E, "PUSH CS", 0, 0},
    {0x16, "PUSH SS", 0, 0},
    {0x17, "POP SS", 0, 0},
    {0x1E, "PUSH DS", 0, 0},
    {0x1F, "POP DS", 0, 0},
    {0x9C, "PUSHFD", 0, 0},
    {0x9D, "POPFD", 0, 0},
    {0x9E, "SAHF", 0, 0},
    {0x9F, "LAHF", 0, 0},
    {0xA4, "MOVSB", 0, 0},
    {0xA5, "MOVSW", 0, 0},
    {0xA6, "CMPSB", 0, 0},
    {0xA7, "CMPSW", 0, 0},
    {0xAA, "STOSB", 0, 0},
    {0xAB, "STOSW", 0, 0},
    {0xAC, "LODSB", 0, 0},
    {0xAD, "LODSW", 0, 0},
    {0xAE, "SCASB", 0, 0},
    {0xAF, "SCASW", 0, 0},
    {0xC3, "RET", 0, 0},
    {0xCB, "RETF", 0, 0},
    {0xCC, "INT3", 0, 0},
    {0xCD, "INT", 1, 0},
    {0xCF, "IRET", 0, 0},
    {0xE4, "IN AL,", 1, 0},
    {0xE6, "OUT", 1, 0},
    {0xEC, "IN AL, DX", 0, 0},
    {0xEE, "OUT DX, AL", 0, 0},
    {0xF4, "HLT", 0, 0},
    {0xFA, "CLI", 0, 0},
    {0xFB, "STI", 0, 0},
    {0xFC, "CLD", 0, 0},
    {0xFD, "STD", 0, 0},
    {0xFE, "INC/DEC/CALL/JMP/PUSH", 0, 1},
    {0xFF, "INC/DEC/CALL/JMP/PUSH/IMUL/BOUND/ARPL", 0, 1},
    {0x00, "ADD", 0, 1},
    
    {0, nullptr, 0, 0} // 结束标记
};

LabelManager labelManager;
Disassemblermode disassemblerModeInstance;  // 实际实例
Disassemblermode* DisassemblermodePtr = &disassemblerModeInstance;  // 全局指针指向实例

const InstructionInfo* get_instruction_info(uint8_t opcode) {
    for (int i = 0; instructions[i].mnemonic != nullptr; i++) {
        if (instructions[i].opcode == opcode) {
            return &instructions[i];
        }
    }
    return nullptr;
}

int Disassemblermode::WriteDisasmmode() {
    FILE* file = fopen("disasmmode.ini", "w");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    const char* content;
    if (g_disasm_mode == 16) {
        content = "16mode";
    } else {
        content = "32mode";
    }
    fprintf(file, "%s", content);

    fclose(file);
    return 0;
}

int Disassemblermode::ReadDisasmmode() {
    FILE* file = fopen("disasmmode.ini", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "16mode") == 0) {
            printf("Found '16mode'\n");
            g_disasm_mode = 16;
        } else if (strcmp(line, "32mode") == 0) {
            printf("Found '32mode'\n");
            g_disasm_mode = 32;
        }
    }

    fclose(file);
    return 0;
}

std::string to_hex_string(uint64_t value) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%08llX", static_cast<unsigned long long>(value));
    return std::string(buffer);
}

void LabelManager::add_label(uint64_t address, const std::string& label) {
    // 如果地址已有标签，不覆盖（或者根据需要决定是否覆盖）
    if (labels_.find(address) == labels_.end()) {
        labels_[address] = label;
    }
}

std::string LabelManager::get_label(uint64_t address) const {
    auto it = labels_.find(address);
    if (it != labels_.end()) {
        return it->second;
    }

    // 如果没有标签，自动生成一个
    std::ostringstream oss;
    oss << "loc_" << std::hex << address;
    return oss.str();
}

void LabelManager::add_reference(uint64_t from, uint64_t to) {
    references_.insert({to, from});
}

std::set<uint64_t> LabelManager::get_references_to(uint64_t address) const {
    std::set<uint64_t> refs;
    auto range = references_.equal_range(address);
    for (auto it = range.first; it != range.second; ++it) {
        refs.insert(it->second);
    }
    return refs;
}

bool LabelManager::has_label(uint64_t address) const {
    return labels_.find(address) != labels_.end();
}

void LabelManager::clear() {
    labels_.clear();
    references_.clear();
}

const char* disassemble_instruction(uint64_t base_addr, const uint8_t *code, size_t *size) {

    static char final_buffer[300]; // 用于带前缀的最终输出
    char prefix_buf[20] = {0};
    
    // 定义opcode变量
    uint8_t opcode = code[0];
    const InstructionInfo *info = get_instruction_info(opcode);

    static char buffer[256];

    if (!info) {
        snprintf(buffer, sizeof(buffer), "DB 0x%02X", code[0]);
        *size = 1;
        return buffer;
    }

    size_t offset = 1;
    const char *mnemonic = info->mnemonic;

    // 处理带立即数的指令
    if (info->operand_size > 0) {
        uint32_t imm = 0;
        // 小端模式读取立即数
        for (int i = 0; i < info->operand_size; i++) {
            imm |= (static_cast<uint32_t>(code[offset + i]) << (i * 8));
        }
        offset += info->operand_size;

        // 跳转指令计算绝对地址
        if (strstr(mnemonic, "J") == mnemonic || strstr(mnemonic, "CALL") == mnemonic) {
            // 计算当前指令结束地址
            uint64_t current_end = base_addr + offset;

            // 计算跳转目标
            uint64_t target = 0;
            if (info->operand_size == 1) {
                int8_t rel8 = static_cast<int8_t>(imm);
                target = current_end + rel8;
            } else if (info->operand_size == 4) {
                int32_t rel32 = static_cast<int32_t>(imm);
                target = current_end + rel32;
            }

            // 创建标签名称
            std::string label = "loc_" + to_hex_string(target);

            // 添加标签
            labelManager.add_label(target, label);

            // 添加引用关系
            labelManager.add_reference(base_addr, target);

            // 在指令中使用标签
            snprintf(buffer, sizeof(buffer), "%s %s", mnemonic, label.c_str());
        } else {
            // 普通立即数
            if (info->operand_size == 1) {
                snprintf(buffer, sizeof(buffer), "%s 0x%02X", mnemonic, imm & 0xFF);
            } else if (info->operand_size == 2) {
                snprintf(buffer, sizeof(buffer), "%s 0x%04X", mnemonic, imm & 0xFFFF);
            } else {
                snprintf(buffer, sizeof(buffer), "%s 0x%08X", mnemonic, imm);
            }
        }
    }
    // 处理带ModR/M的指令
    else if (info->has_modrm && DisassemblermodePtr->g_disasm_mode == 32) {
        uint8_t modrm = code[offset++];
        uint8_t mod = modrm >> 6;
        uint8_t reg = (modrm >> 3) & 7;
        uint8_t rm = modrm & 7;

        const char *reg_names[] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
        const char *reg32_names[] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};

        if (mod == 3) {
            // 寄存器到寄存器
            if (info->operand_size == 1) {
                snprintf(buffer, sizeof(buffer), "%s %s, %s",
                         mnemonic, reg_names[reg], reg_names[rm]);
            } else {
                snprintf(buffer, sizeof(buffer), "%s %s, %s",
                         mnemonic, reg32_names[reg], reg32_names[rm]);
            }
        } else {
            // 内存寻址
            const char *base_reg[] = {"BX+SI", "BX+DI", "BP+SI", "BP+DI", "SI", "DI", "BP", "BX"};

            if (rm == 4) { // SIB 字节
                if (offset < *size) {
                    uint8_t sib = code[offset++];
                    uint8_t scale = (sib >> 6) & 3;
                    uint8_t index = (sib >> 3) & 7;
                    uint8_t base = sib & 7;

                    snprintf(buffer, sizeof(buffer), "%s [%s*%d + %s], %s",
                             mnemonic, reg32_names[index], 1<<scale,
                             reg32_names[base], reg32_names[reg]);
                }
            } else if (mod == 0 && rm == 5) {
                // 直接地址
                uint32_t disp = 0;
                memcpy(&disp, code + offset, 4);
                offset += 4;
                snprintf(buffer, sizeof(buffer), "%s [0x%08X], %s",
                         mnemonic, disp, reg32_names[reg]);
            } else {
                // 带偏移的基址寻址
                int disp_size = (mod == 1) ? 1 : (mod == 2) ? 4 : 0;
                int disp_val = 0;

                if (disp_size > 0) {
                    memcpy(&disp_val, code + offset, disp_size);
                    offset += disp_size;
                }

                if (disp_val == 0) {
                    snprintf(buffer, sizeof(buffer), "%s [%s], %s",
                             mnemonic, base_reg[rm], reg32_names[reg]);
                } else {
                    snprintf(buffer, sizeof(buffer), "%s [%s + %d], %s",
                             mnemonic, base_reg[rm], disp_val, reg32_names[reg]);
                }
            }
        }
    }else if(info->has_modrm && DisassemblermodePtr->g_disasm_mode == 16){
        if (info->has_modrm) {
            uint8_t modrm = code[offset++];
            uint8_t mod = modrm >> 6;
            uint8_t reg = (modrm >> 3) & 7;
            uint8_t rm = modrm & 7;
            
            if (Disassembler::get_mode() == 16) {
                // 16位寄存器名称
                const char *reg16_names[] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
                const char *reg8_names[] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
                const char *sreg_names[] = {"ES", "CS", "SS", "DS", "FS", "GS"};
                
                // 16位寻址模式
                const char *base_reg[] = {"BX+SI", "BX+DI", "BP+SI", "BP+DI", "SI", "DI", "BP", "BX"};
                
                if (mod == 3) {
                    // 寄存器模式
                    if (strstr(mnemonic, "MOV") && opcode == 0x8E) {
                        // 特殊处理段寄存器
                        snprintf(buffer, sizeof(buffer), "MOV %s, %s", 
                                 sreg_names[reg], reg16_names[rm]);
                    } else if (info->operand_size > 0) {
                        // 16位寄存器
                        snprintf(buffer, sizeof(buffer), "%s %s, %s",
                                 mnemonic, reg16_names[reg], reg16_names[rm]);
                    } else {
                        // 8位寄存器
                        snprintf(buffer, sizeof(buffer), "%s %s, %s",
                                 mnemonic, reg8_names[reg], reg8_names[rm]);
                    }
                } else {
                    // 内存寻址模式
                    int disp_size = 0;
                    int16_t disp_val = 0;
                    
                    if (mod == 0) {
                        if (rm == 6) {
                            // 直接地址 [disp16]
                            disp_size = 2;
                            disp_val = *(int16_t*)(code + offset);
                            offset += 2;
                            snprintf(buffer, sizeof(buffer), "%s [0x%04X]", mnemonic, disp_val & 0xFFFF);
                        } else {
                            // 无位移
                            snprintf(buffer, sizeof(buffer), "%s [%s]", mnemonic, base_reg[rm]);
                        }
                    } else if (mod == 1) {
                        // 8位位移
                        disp_size = 1;
                        disp_val = *(int8_t*)(code + offset);
                        offset += 1;
                    } else if (mod == 2) {
                        // 16位位移
                        disp_size = 2;
                        disp_val = *(int16_t*)(code + offset);
                        offset += 2;
                    }
                    
                    if (disp_size > 0) {
                        if (disp_val == 0) {
                            snprintf(buffer, sizeof(buffer), "%s [%s]", 
                                     mnemonic, base_reg[rm]);
                        } else if (disp_val > 0) {
                            snprintf(buffer, sizeof(buffer), "%s [%s + %d]", 
                                     mnemonic, base_reg[rm], disp_val);
                        } else {
                            snprintf(buffer, sizeof(buffer), "%s [%s - %d]", 
                                     mnemonic, base_reg[rm], -disp_val);
                        }
                    }
                }
            }
        }
    }
    else {
        strncpy(buffer, mnemonic, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
    }

    *size = offset;
    return buffer;
}
