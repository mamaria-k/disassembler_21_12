#include <iostream>
#include <fstream>
#include <string>
#include "elf_parser.h"

using std::cin, std::cout, std::cerr, std::endl;


int main(int argc, char** argv) {
    try {
        if (argc != 3) throw DisassemblerException("Wrong number of arguments!");
        std::string input_filename = std::string (argv[1]);
        std::string output_filename = std::string (argv[2]);
        std::ifstream input(input_filename, std::ios::binary);
        if (!input) {
            throw DisassemblerException("Unable to open elf file!");
        }
        std::ofstream output(output_filename);
        if (!output) {
            throw DisassemblerException("Unable to open file for saving result!");
        }

        ELF_Header elf_header(input);
        Section_Info s_i_text, s_i_symtable, shstrtab;
        elf_header.search_sections_info(input, s_i_text, s_i_symtable, shstrtab);

        RWer rw(&s_i_text, &s_i_symtable, &shstrtab);
        rw.processing_symtable(input, output);
        rw.processing_text(input, output);
        rw.write_symtab(output);

    }
    catch (DisassemblerException& e) {
        cout << e.get_message() << endl;
    }
    catch (std::bad_alloc& e) {
        cout << "Unable to allocate memory." << endl;
    }
    catch(std::exception& e) {
        cout << e.what() << endl;
    }
    return 0;
}


