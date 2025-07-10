#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdint.h>
#include <cstddef>  // 添加 size_t 支持
#include <stdint.h>
#include <cstddef>
#include <map>
#include <set>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_INSTR_LEN 15

typedef struct {
    uint8_t opcode;
    const char *mnemonic;
    int operand_size;
    int has_modrm;
} InstructionInfo;

class LabelManager {
public:
    void add_label(uint64_t address, const std::string& label);
    std::string get_label(uint64_t address) const;
    void add_reference(uint64_t from, uint64_t to);
    std::set<uint64_t> get_references_to(uint64_t address) const;
    bool has_label(uint64_t address) const;
    void clear();

private:
    std::map<uint64_t, std::string> labels_;
    std::multimap<uint64_t, uint64_t> references_;
};

// 声明全局实例
extern LabelManager labelManager;

class Disassemblermode {
public:
    int g_disasm_mode;

    int WriteDisasmmode();
    int ReadDisasmmode();
};

// 修改为全局指针并初始化
extern Disassemblermode* DisassemblermodePtr;

class Disassembler {
public:
    static void set_mode(int mode) { g_disasm_mode = mode; }
    static int get_mode() { return g_disasm_mode; }

private:
    static int g_disasm_mode;
};

inline int Disassembler::g_disasm_mode = 16;  // 默认16位模式

const char* disassemble_instruction(uint64_t base_addr, const uint8_t *code, size_t *size);
const InstructionInfo* get_instruction_info(uint8_t opcode);
int disassemblermain(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif
