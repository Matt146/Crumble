#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "sha256.h"

#define MAX_FNAME 4096
#define PADDING 8192
#define CRUMBLE_CNF ".crmblecnf"

uint64_t get_file_size(std::string filename) {
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    uint64_t size = (uint64_t)ftell(p_file);
    fclose(p_file);
    return size;
}

std::vector<std::string> read_init_file(const char* fname, size_t parts, size_t padding) {
    FILE* fp = fopen(fname, "rb");
    std::vector<std::string> buff;
    uint64_t buffsize = uint64_t(get_file_size(fname) / (uint64_t)parts);
    if (fp == NULL) {
        std::cout << "[Error] Could not open file!" << std::endl;
        exit(-1);
    }
    else {
        while (!feof(fp)) {
            char* temp_buffer = (char*)malloc((sizeof(char) * buffsize) + (sizeof(char) * padding));
            fgets(temp_buffer, buffsize, fp);
            std::string temp_buffer_container(temp_buffer);
            buff.push_back(temp_buffer_container);
            free(temp_buffer);
        }
    }
    fclose(fp);
    return buff;
}

void write_file(const char* fname, std::string data) {
    FILE* fp = fopen(fname, "w");
    if (fp == NULL) {
        std::cout << "[Error] Could not write to file!" << std::endl;
        exit(-1);
    }
    else {
        fprintf(fp, "%s", data.c_str());
    }
    fclose(fp);
}

std::string create_fname(const char* fname, size_t nonce) {
    std::string str_fname(fname);
    std::string final_fname = sha256(fname + std::to_string(nonce));
    return final_fname + ".crumble";
}

void split(const char* fname, size_t parts) {
    /*Turn back now...*/
    std::string str_fname(fname);
    std::string config_file = str_fname + ".crumblecnf";
    std::vector<std::string> files;
    for (size_t i = 0; i < parts; i++) {
        files.push_back(create_fname(fname, i));
    }
    FILE* fp = fopen(config_file.c_str(), "w");
    std::string fname_c = std::string(fname) + "\n";
    fprintf(fp, "%s", fname_c.c_str());
    for (size_t i = 0; i < parts; i++) {
        std::string fname_to_put = files[i] + '\n';
        fprintf(fp, "%s", fname_to_put.c_str());
    }
    fclose(fp);
    std::vector<std::string> files_data = read_init_file(fname, parts, PADDING);
    for (size_t i = 0; i < parts; i++) {
        write_file(files[i].c_str(), files_data[i]);
    }
}

std::vector<std::string> parse_config(const char* config_fname) {
    std::vector<std::string> buff;
    std::string temp_buffer;
    FILE* fp = fopen(config_fname, "r");
    if (fp == NULL) {
        std::cout << "[Error] Config file not located!" << std::endl;
        std::cout << "\tFile should have the extension: .crumblecnf" << std::endl;
    }
    else {
        while (!feof(fp)) {
            char char_gotten = getc(fp);
            if (char_gotten == '\n') {
                buff.push_back(temp_buffer);
                temp_buffer.clear();
            }
            else {
                temp_buffer.push_back(char_gotten);
            }
        }
    }
    fclose(fp);
    return buff;
}

std::string read_file(const char* fname) {
    std::string buff;
    FILE* fp = fopen(fname, "r");
    char char_gotten;
    if (fp == NULL) {
        std::cout << "[Error] File not located!" << std::endl;
        std::string str_fname(fname);
        std::cout << "\tUnable to find " << str_fname << " to reassemble file." << std::endl;
    }
    else {
        while (!feof(fp)) {
            char_gotten = getc(fp);
            buff.push_back(char_gotten);
        }
    }
    fclose(fp);
    return buff;
}

void reassemble(const char* config_fname) {
    std::vector<std::string> files = parse_config(config_fname);
    std::string main_file_buff = "";
    for (uint64_t i = 1; i < files.size(); i++) {
        main_file_buff += read_file(files[i].c_str());
    }
    std::string resolved_fname = std::string(config_fname).substr(0, std::string(config_fname).size() - (std::string(CRUMBLE_CNF).size()) + 1);
    write_file(resolved_fname.c_str(), main_file_buff);
}

int main(int argc, char** argv) {
    std::cout << "Crumble" << std::endl;
    if (argc < 3) {
        std::cout << "[Error] Incorrect Usage" << std::endl;
        std::cout << "\tCorrect Usage: (<split/reassemble>) (<filename/config file>) (<number of parts>)" << std::endl;
    }
    else {
        if (std::string(argv[1]).compare("split") == 0) {
            size_t parts = 0;
            if (1 == sscanf(argv[3], "%zu", &parts)) {
                split(argv[2], parts);
            }
            else {
                std::cout << "[Error] Unspecified # of parts!" << std::endl;
            }
        }
        else if (std::string(argv[1]).compare("reassemble") == 0) {
            reassemble(argv[2]);
        }
        else {
            std::cout << "[Error] Unrecognized command: " << argv[1] << std::endl;
        }
    }
    return 0;
}
