#pragma once
#include <exception>
#include <string>
#include <map>
#include <vector>


class DisassemblerException: public std::exception {
public:
    explicit DisassemblerException(std::string message);
    std::string get_message() const;

private:
    std::string _message;
};


const size_t E_HEADER_SIZE = 52;
const char EI_MAG0 = 0x7f;
const char EI_MAG1 = 0x45;
const char EI_MAG2 = 0x4c;
const char EI_MAG3 = 0x46;
const char ELFCLASS32 = 0x01;
const char ELFDATA2LSB = 0x01;
const char EV_CURRENT = 0x01;
const size_t E_SHOFF_POS = 32;
const size_t E_SINFO_POS = 46;
const size_t E_TI_POS = 50;
const size_t S_OFFSET_INFO = 16;

struct Section_Info {
    uint32_t sh_offset = 0;
    uint32_t sh_size = 0;
};

class ELF_Header {
public:
    explicit ELF_Header(std::ifstream& input);
    void search_sections_info(std::ifstream& input,
                              Section_Info& s_i_text,
                              Section_Info& s_i_symtable,
                              Section_Info& shstrtab) const;

private:
    uint32_t e_shoff = 0;
    uint16_t e_shentsize = 0;
    uint16_t e_shnum = 0;
    uint16_t e_shstrndx = 0;
};


const size_t BIG_INST_SIZE = 4;
const size_t SMALL_INST_SIZE = 2;
const size_t STR_SYMTAB_SIZE = 16;

struct Str_Symtab {
    void write(std::ofstream& output, size_t i) const;
    uint32_t value;
    uint32_t size;
    unsigned char info;
    unsigned char other;
    uint16_t index;
    std::string name;
};

class RWer {
public:
    explicit RWer(Section_Info* s_i_text, Section_Info* s_i_symtable, Section_Info* shstrtab);
    void processing_text(std::ifstream& input, std::ofstream& output);
    void processing_symtable(std::ifstream& input, std::ofstream& output);
    void write_symtab(std::ofstream& output);

private:
    std::vector<Str_Symtab> v_str_symtab;
    std::map<uint32_t, std::string> labels;
    Section_Info* s_i_text;
    Section_Info* s_i_symtable;
    Section_Info* shstrtab;
};





