#define _GNU_SOURCE

#include "params.h"
#include "alienos.h"

#include <stdio.h>
#include <elf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define IS_ELF64_HDR(x) \
    ((x).e_ident[EI_MAG0] == ELFMAG0) \
    && ((x).e_ident[EI_MAG1] == ELFMAG1) \
    && ((x).e_ident[EI_MAG2] == ELFMAG2) \
    && ((x).e_ident[EI_MAG3] == ELFMAG3) \
    && ((x).e_ident[EI_CLASS] == ELFCLASS64)

static int search_headers(int fd, Elf64_Ehdr *elf_header, off_t *section_start, size_t *section_size) {
    int result = NO_PARAMS;

    Elf64_Phdr *p_headers;
    size_t p_headers_size = elf_header->e_phentsize * elf_header->e_phnum;
    p_headers = (Elf64_Phdr *) malloc(p_headers_size);
    if (p_headers == NULL) {
        return ERR_PARAMS;
    }

    if (pread(fd, p_headers, p_headers_size, elf_header->e_phoff) != p_headers_size) {
        free(p_headers);
        return ERR_PARAMS;
    }

    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        if (p_headers[i].p_type == PT_PARAMS) {
            *section_start = p_headers[i].p_vaddr;
            *section_size = p_headers[i].p_filesz;
            result = HAS_PARAMS;
            break;
        }
    }

    free(p_headers);
    return result;
}


int get_params_address(const char *filename, off_t *section_start, size_t *section_size) {
    int fd;
    Elf64_Ehdr elf_header;

    if ((fd = open(filename, O_RDONLY)) == -1) {
        return ERR_PARAMS;
    }

    if (pread(fd, &elf_header, sizeof(elf_header), 0) != sizeof(elf_header)) {
        close(fd);
        return ERR_PARAMS;
    }

    if (!IS_ELF64_HDR(elf_header)) {
        close(fd);
        return ERR_PARAMS;
    }

    int result = search_headers(fd, &elf_header, section_start, section_size);
    close(fd);
    return result;
}