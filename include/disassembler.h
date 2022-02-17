#pragma once
#include <iostream>
#include <string>


uint32_t get_bits(uint32_t n, size_t pos, size_t len);

void write_big_instruction(std::ofstream& output, uint32_t num, uint32_t big_inst, std::string label);

class R_type {
public:
    explicit R_type(uint32_t s_command);
    std::string command_to_string() const;
    std::string to_string() const;


private:
    uint8_t opcode;
    uint8_t fun3;
    uint8_t fun7;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t rd;
};

class I_type {
public:
    explicit I_type(uint32_t s_command);
    std::string command_to_string() const;
    std::string imm_to_str() const;
    std::string to_string() const;

private:
    uint8_t opcode;
    uint8_t fun3;
    uint8_t rs1;
    uint8_t rd;
    uint32_t imm;
};

class S_type {
public:
    explicit S_type(uint32_t s_command);
    std::string command_to_string() const;
    std::string imm_to_str() const;
    std::string to_string() const;

private:
    uint8_t opcode;
    uint8_t fun3;
    uint8_t rs1;
    uint8_t rs2;
    uint32_t imm;
};

class B_type {
public:
    explicit B_type(uint32_t s_command);
    std::string command_to_string() const;
    std::string imm_to_str() const;
    std::string to_string() const;

private:
    uint8_t opcode;
    uint8_t fun3;
    uint8_t rs1;
    uint8_t rs2;
    uint32_t imm;
};

class U_type {
public:
    explicit U_type(uint32_t s_command);
    std::string command_to_string() const;
    std::string imm_to_str() const;
    std::string to_string() const;

private:
    uint8_t opcode;
    uint8_t rd;
    uint32_t imm;
};

class J_type {
public:
    explicit J_type(uint32_t s_command);
    std::string command_to_string() const;
    std::string imm_to_str() const;
    std::string to_string() const;

private:
    uint8_t opcode;
    uint8_t rd;
    uint32_t imm;
};





