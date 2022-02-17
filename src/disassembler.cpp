#include <valarray>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "disassembler.h"


uint32_t get_bits(uint32_t n, size_t pos, size_t len) {
    return ((n >> pos) & ((1 << len) - 1));
}

static std::string decode_reg(uint32_t reg) {
    if (reg == 0) return "zero";
    if (reg == 1) return "ra";
    if (reg == 2) return "sp";
    if (reg == 3) return "gp";
    if (reg == 4) return "tp";
    if (reg >= 5 && reg <= 7) return ("t" + std::to_string(reg - 5));
    if (reg >= 8 && reg <= 9) return ("s" + std::to_string(reg - 8));
    if (reg >= 10 && reg <= 17) return ("a" + std::to_string(reg - 10));
    if (reg >= 18 && reg <= 27) return ("s" + std::to_string(reg - 16));
    if (reg >= 28 && reg <= 31) return ("t" + std::to_string(reg - 25));
    return "-";
}


void write_big_instruction(std::ofstream& output, uint32_t num, uint32_t big_inst, std::string label) {
    std::string rec = "unknown_command";
    uint8_t opcode = get_bits(big_inst, 0, 7);
    if (opcode == 0b0110011 || opcode == 0b0111011) {
        R_type r_inst = R_type(big_inst);
        rec = r_inst.to_string();
    }
    if (opcode == 0b0000011 || opcode == 0b0001111 || opcode == 0b0010011 ||
        opcode == 0b0011011 || opcode == 0b1100111 || opcode == 0b1110011) {
        I_type i_inst = I_type(big_inst);
        rec = i_inst.to_string();
    }
    if (opcode == 0b0100011) {
        S_type s_inst = S_type(big_inst);
        rec = s_inst.to_string();
    }
    if (opcode == 0b1100011 ) {
        B_type b_inst = B_type(big_inst);
        rec = b_inst.to_string();
    }
    if (opcode == 0b0010111 || opcode == 0b0110111) {
        U_type u_inst = U_type(big_inst);
        rec = u_inst.to_string();
    }
    if (opcode == 0b1101111) {
        J_type j_inst = J_type(big_inst);
        rec = j_inst.to_string();
    }

    char str[100];
    sprintf(str, "%08x %10s: %s\n", num, label.data(), rec.data());
    output << str;
}


R_type::R_type(uint32_t s_command) {
    opcode = get_bits(s_command, 0, 7);
    fun3 = get_bits(s_command, 12, 3);
    fun7 = get_bits(s_command, 25, 7);
    rs1 = get_bits(s_command, 15, 5);
    rs2 = get_bits(s_command, 20, 5);
    rd = get_bits(s_command, 7, 5);
}

std::string R_type::command_to_string() const {
    if (opcode == 0b0110011) {
        if (fun7 == 0b0000001) {
            if (fun3 == 0b000) return "mul";
            if (fun3 == 0b001) return "mulh";
            if (fun3 == 0b010) return "mulhsu";
            if (fun3 == 0b011) return "mulhu";
            if (fun3 == 0b100) return "div";
            if (fun3 == 0b101) return "divu";
            if (fun3 == 0b110) return "rem";
            if (fun3 == 0b111) return "remu";
        }

        if (fun3 == 0b000) {
            if (fun7 == 0b0000000) return "add";
            if (fun7 == 0b0100000) return "sub";
        }
        if (fun3 == 0b001) return "sll";
        if (fun3 == 0b010) return "slt";
        if (fun3 == 0b011) return "sltu";
        if (fun3 == 0b100) return "xor";
        if (fun3 == 0b101) {
            if (fun7 == 0b0000000) return "srl";
            if (fun7 == 0b0100000) return "sra";
        }
        if (fun3 == 0b110) return "or";
        if (fun3 == 0b111) return "and";
    }
    if (opcode == 0b0111011) {
        if (fun3 == 0b000) {
            if (fun7 == 0b0000000) return "addw";
            if (fun7 == 0b0100000) return "subw";
        }
        if (fun3 == 0b001) return "sllw";
        if (fun3 == 0b101) {
            if (fun7 == 0b0000000) return "srlw";
            if (fun7 == 0b0100000) return "sraw";
        }
    }
    return "unknown_command";
}

std::string R_type::to_string() const {
    char inst[100];
    sprintf(inst, "%s %s, %s, %s",
            command_to_string().data(), decode_reg(rd).data(), decode_reg(rs1).data(), decode_reg(rs2).data());
    return inst;
}


I_type::I_type(uint32_t s_command) {
    opcode = get_bits(s_command, 0, 7);
    fun3 = get_bits(s_command, 12, 3);
    rs1 = get_bits(s_command, 15, 5);
    rd = get_bits(s_command, 7, 5);
    imm = get_bits(s_command, 20, 12);
}

std::string I_type::command_to_string() const {
    if (opcode == 0b0000011) {
        if (fun3 == 0b000) return "lb";
        if (fun3 == 0b001) return "lh";
        if (fun3 == 0b010) return "lw";
        if (fun3 == 0b011) return "ld";
        if (fun3 == 0b100) return "lbu";
        if (fun3 == 0b101) return "lhu";
        if (fun3 == 0b110) return "lwu";
    }
    if (opcode == 0b0001111) {
        if (fun3 == 0b000) return "fence";
        if (fun3 == 0b001) return "fence.i";
    }
    if (opcode == 0b0010011) {
        if (fun3 == 0b000) return "addi";
        if (fun3 == 0b001) return "slli";
        if (fun3 == 0b010) return "slti";
        if (fun3 == 0b011) return "sltiu";
        if (fun3 == 0b100) return "xori";
        if (fun3 == 0b101) {
            if (get_bits(imm, 5, 7) == 0b0000000) return "srli";
            if (get_bits(imm, 5, 7) == 0b0100000) return "srai";
        }
        if (fun3 == 0b110) return "ori";
        if (fun3 == 0b111) return "andi";
    }
    if (opcode == 0b0011011) {
        if (fun3 == 0b000) return "addiw";
        if (fun3 == 0b001) return "slliw";
        if (fun3 == 0b101) {
            if (get_bits(imm, 5, 7) == 0b0000000) return "srliw";
            if (get_bits(imm, 5, 7) == 0b0100000) return "sraiw";
        }
    }
    if (opcode == 0b1100111) return "jalr";
    if (opcode == 0b1110011) {
        if (fun3 == 0b000) {
            if (get_bits(imm, 5, 7) == 0) return "ecall";
            if (get_bits(imm, 5, 7) == 1) return "ebreak";
        }
    }
    return "unknown_command";
}

std::string I_type::to_string() const {
    char inst[100];
    if (opcode == 0b0000011)
        sprintf(inst, "%s %s, %s(%s)",
                command_to_string().data(), decode_reg(rd).data(), imm_to_str().data(), decode_reg(rs1).data());
    else
        sprintf(inst, "%s %s, %s, %s",
                command_to_string().data(), decode_reg(rd).data(), decode_reg(rs1).data(), imm_to_str().data());

    return inst;
}

std::string I_type::imm_to_str() const {
    if (get_bits(imm, 11, 1) == 1) return (std::to_string((int)imm - (1 << 12)));
    return std::to_string((int) imm);
}


S_type::S_type(uint32_t s_command) {
    opcode = get_bits(s_command, 0, 7);
    fun3 = get_bits(s_command, 12, 3);
    rs1 = get_bits(s_command, 15, 5);
    rs2 = get_bits(s_command, 20, 5);
    imm = get_bits(s_command, 7, 5) +
            (get_bits(s_command, 25, 7) << 5);
}

std::string S_type::command_to_string() const {
    if (opcode == 0b0100011) {
        if (fun3 == 0b000) return "sb";
        if (fun3 == 0b001) return "sh";
        if (fun3 == 0b010) return "sw";
        if (fun3 == 0b011) return "sd";
    }
    return "unknown_command";
}

std::string S_type::to_string() const {
    char inst[100];
    sprintf(inst, "%s %s, %s(%s)",
            command_to_string().data(), decode_reg(rs1).data(), imm_to_str().data(), decode_reg(rs2).data());
    return inst;
}

std::string S_type::imm_to_str() const {
    if (get_bits(imm, 11, 1) == 1) return (std::to_string((int)imm - (1 << 12)));
    return std::to_string((int) imm);
}


B_type::B_type(uint32_t s_command) {
    opcode = get_bits(s_command, 0, 7);
    fun3 = get_bits(s_command, 12, 3);
    rs1 = get_bits(s_command, 15, 5);
    rs2 = get_bits(s_command, 20, 5);
    imm = (get_bits(s_command, 7, 1) << 11) +
            (get_bits(s_command, 8, 4) << 1) +
            (get_bits(s_command, 25, 6) << 5) +
            (get_bits(s_command, 31, 1) << 12);
}

std::string B_type::command_to_string() const {
    if (opcode == 0b1100011) {
        if (fun3 == 0b000) return "beq";
        if (fun3 == 0b001) return "bne";
        if (fun3 == 0b100) return "blt";
        if (fun3 == 0b101) return "bge";
        if (fun3 == 0b110) return "bltu";
        if (fun3 == 0b111) return "bgeu";
    }
    return "unknown_command";
}

std::string B_type::to_string() const {
    char inst[100];
    sprintf(inst, "%s %s, %s, %s",
            command_to_string().data(), decode_reg(rs1).data(), decode_reg(rs2).data(), imm_to_str().data());
    return inst;
}

std::string B_type::imm_to_str() const {
    if (get_bits(imm, 12, 1) == 1) return (std::to_string((int)imm - (1 << 13)));
    return std::to_string((int) imm);
}


U_type::U_type(uint32_t s_command) {
    opcode = get_bits(s_command, 0, 7);
    rd = get_bits(s_command, 7, 5);
    imm = get_bits(s_command, 12, 20) << 12;
}

std::string U_type::command_to_string() const {
    if (opcode == 0b0010111) return "auipc";
    if (opcode == 0b0110111) return "lui";
    return "unknown_command";
}

std::string U_type::to_string() const {
    char inst[100];
    sprintf(inst, "%s %s, %s",
            command_to_string().data(), decode_reg(rd).data(), imm_to_str().data());
    return inst;
}

std::string U_type::imm_to_str() const {
    if (get_bits(imm, 31, 1) == 1) return (std::to_string((int)imm - (1 << 32)));
    return std::to_string((int) imm);
}


J_type::J_type(uint32_t s_command) {
    opcode = get_bits(s_command, 0, 7);
    rd = get_bits(s_command, 7, 5);
    imm = (get_bits(s_command, 12, 7) << 12) +
            (get_bits(s_command, 20, 1) << 11) +
            (get_bits(s_command, 21, 10) << 1) +
            (get_bits(s_command, 31, 1) << 20);
}

std::string J_type::command_to_string() const {
    if (opcode == 0b1101111) return "jal";
    return "unknown_command";
}

std::string J_type::to_string() const {
    char inst[100];
    sprintf(inst, "%s %s, %s",
            command_to_string().data(), decode_reg(rd).data(), imm_to_str().data());
    return inst;
}

std::string J_type::imm_to_str() const {
    if (get_bits(imm, 20, 1) == 1) return (std::to_string((int)imm - (1 << 21)));
    return std::to_string((int) imm);
}



