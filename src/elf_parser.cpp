#include "elf_parser.h"
#include "disassembler.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <string>



DisassemblerException::DisassemblerException(std::string message): _message(std::move(message)) {}

std::string DisassemblerException::get_message() const {
    return _message;
}


ELF_Header::ELF_Header(std::ifstream& input) {
    input.seekg(0, std::ios::end);
    size_t length = input.tellg();
    input.seekg(0, std::ios::beg);
    if (length < E_HEADER_SIZE)
        throw DisassemblerException("Incorrect file format! Size of the ELF Header is too small.");

    char format[E_HEADER_SIZE];
    input.read(format, E_HEADER_SIZE);
    if (format[0] != EI_MAG0 || format[1] != EI_MAG1 || format[2] != EI_MAG2 || format[3] != EI_MAG3 ||
        format[4] != ELFCLASS32 || format[5] != ELFDATA2LSB || format[6] != EV_CURRENT)
        throw DisassemblerException("Incorrect file format! Correct format: ELF file, 32b, LSB");

    input.seekg(E_SHOFF_POS, std::ios::beg);
    input.read((char*) &e_shoff, 4);
    if (e_shoff == 0)
        throw DisassemblerException("Incorrect file format! There is no section header table.");
    input.seekg(E_SINFO_POS, std::ios::beg);
    input.read((char*) &e_shentsize, 2);
    input.read((char*) &e_shnum, 2);
    input.seekg(E_TI_POS, std::ios::beg);
    input.read((char*) &e_shstrndx, 2);
}

void ELF_Header::search_sections_info(std::ifstream& input,
                                      Section_Info& s_i_text,
                                      Section_Info& s_i_symtable,
                                      Section_Info& shstrtab) const {
    input.seekg(e_shoff + e_shstrndx * e_shentsize + S_OFFSET_INFO, std::ios::beg);
    input.read((char*) &shstrtab.sh_offset, 4);
    input.read((char*) &shstrtab.sh_size, 4);

    uint32_t sh_name;
    char name[shstrtab.sh_size];
    for (size_t i = 0; i < e_shnum; i++) {
        input.seekg(e_shoff + i * e_shentsize, std::ios::beg);
        input.read((char*) &sh_name, 4);
        input.seekg(shstrtab.sh_offset + sh_name, std::ios::beg);
        input.getline(name, shstrtab.sh_size, '\0');
        if (strcmp(name, ".text") == 0) {
            input.seekg(e_shoff + i * e_shentsize + S_OFFSET_INFO, std::ios::beg);
            input.read((char*) &s_i_text.sh_offset, 4);
            input.read((char*) &s_i_text.sh_size, 4);
        }
        if (strcmp(name, ".symtab") == 0) {
            input.seekg(e_shoff + i * e_shentsize + S_OFFSET_INFO, std::ios::beg);
            input.read((char*) &s_i_symtable.sh_offset, 4);
            input.read((char*) &s_i_symtable.sh_size, 4);
        }
    }
}


RWer::RWer(Section_Info* s_i_text, Section_Info* s_i_symtable, Section_Info* shstrtab):
        s_i_text(s_i_text),
        s_i_symtable(s_i_symtable),
        shstrtab(shstrtab){}

void RWer::processing_symtable(std::ifstream& input, std::ofstream& output) {
    uint32_t address_name;
    char s_t[s_i_symtable->sh_size];
    input.seekg(s_i_symtable->sh_offset, std::ios::beg);
    input.read(s_t, s_i_symtable->sh_size);
    for (size_t i = 0; i < (s_i_symtable->sh_size / STR_SYMTAB_SIZE); i++) {
        Str_Symtab str_sym;
        char name[shstrtab->sh_size];
        input.seekg(s_i_symtable->sh_offset + i * STR_SYMTAB_SIZE, std::ios::beg);
        input.read((char*) &address_name, 4);
        input.read((char*) &str_sym.value, 4);
        input.read((char*) &str_sym.size, 4);
        input.read((char*) &str_sym.info, 1);
        input.read((char*) &str_sym.other, 1);
        input.read((char*) &str_sym.index, 2);
        input.seekg(shstrtab->sh_offset + address_name, std::ios::beg);
        input.getline(name, shstrtab->sh_size, '\0');
        str_sym.name = name;

        v_str_symtab.push_back(str_sym);
        labels[str_sym.value] = str_sym.name;
    }

}


void Str_Symtab::write(std::ofstream& output, size_t i) const {
    unsigned char type = (info) & 0xf;
    std::string type_s;
    if (type == 0) type_s = "NOTYPE";
    if (type == 1) type_s = "OBJECT";
    if (type == 2) type_s = "FUNC";
    if (type == 3) type_s = "SECTION";
    if (type == 4) type_s = "FILE";
    if (type == 5) type_s = "COMMON";
    if (type == 6) type_s = "TLS";
    if (type == 10) type_s = "LOOS";
    if (type == 12) type_s = "HIOS";
    if (type == 13) type_s = "LOPROC";
    if (type == 15) type_s = "HIPROC";

    unsigned char bind = (info) >> 4;
    std::string bind_s;
    if (bind == 0) bind_s = "LOCAL";
    if (bind == 1) bind_s = "GLOBAL";
    if (bind == 2) bind_s = "WEAK";
    if (bind == 10) bind_s = "LOOS";
    if (bind == 12) bind_s = "HIOS";
    if (bind == 13) bind_s = "LOPROC";
    if (bind == 15) bind_s = "HIPROC";

    unsigned char vis = (other) & 0x3;
    std::string vis_s;
    if (vis == 0) vis_s = "DEFAULT";
    if (vis == 1) vis_s = "INTERNAL";
    if (vis == 2) vis_s = "HIDDEN";
    if (vis == 3) vis_s = "PROTECTED";
    if (vis == 4) vis_s = "EXPORTED";
    if (vis == 5) vis_s = "SINGLETON";
    if (vis == 6) vis_s = "ELIMINATE";

    std::string index_s;
    if (index == 0) index_s = "UNDEF";
    if (index == 0xff00) index_s = "LORESERVE";
    if (index == 0xff01) index_s = "AFTER";
    if (index == 0xff02) index_s = "AMD64_LCOMMON";
    if (index == 0xff1f) index_s = "HIPROC";
    if (index == 0xff20) index_s = "LOOS";
    if (index == 0xff3f) index_s = "LOSUNW";
    if (index == 0xfff1) index_s = "ABS";
    if (index == 0xfff2) index_s = "COMMON";
    if (index_s.empty()) index_s = std::to_string(index);

    char str[200];
    sprintf(str, "[%4i] 0x%-15X %5i %-8s %-8s %-8s %6s %s\n",
            i, value, size, type_s.data(), bind_s.data(), vis_s.data(), index_s.data(), name.data());
    output << str;
}



void RWer::processing_text(std::ifstream& input, std::ofstream& output) {
    output.write(".text\n", 6);

    uint32_t big_inst;
    uint16_t small_inst;
    uint8_t indicator;
    input.seekg(s_i_text->sh_offset, std::ios::beg);
    uint32_t remainder = 0;
    while (remainder != s_i_text->sh_size) {
        input.read((char*) &indicator, 1);
        input.seekg(-1, std::ios::cur);
        if (get_bits(indicator, 0, 2) == 3) {
            remainder += BIG_INST_SIZE;
            input.read((char*) &big_inst, BIG_INST_SIZE);
            write_big_instruction(output, remainder - BIG_INST_SIZE, big_inst, labels[remainder - BIG_INST_SIZE]);
        }
        else {
            remainder += SMALL_INST_SIZE;
            input.read((char*) &small_inst, SMALL_INST_SIZE);
            //// Пока непонятно, откуда брать инфу по сжатым командам.
        }
    }
    output.write("\n", 1);
}

void RWer::write_symtab(std::ofstream& output) {
    output.write(".symtab\n", 8);
    char h_symtab[100];
    sprintf(h_symtab, "%s %-15s %7s %-8s %-8s %-8s %6s %s\n",
            "Symbol", "Value", "Size", "Type", "Bind", "Vis", "Index", "Name");
    output << h_symtab;

    for (size_t i = 0; i < v_str_symtab.size(); i++) {
        v_str_symtab[i].write(output, i);
    }
}






