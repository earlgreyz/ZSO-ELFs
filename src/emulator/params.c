#define _GNU_SOURCE

#include "params.h"
#include "alienos.h"

#include <stdio.h>
#include <elf.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define MUST(x) if ((x) == -1) return -1;
#define IS_ELF64_HDR(x) \
    ((x).e_ident[EI_MAG0] == ELFMAG0) \
    && ((x).e_ident[EI_MAG1] == ELFMAG1) \
    && ((x).e_ident[EI_MAG2] == ELFMAG2) \
    && ((x).e_ident[EI_MAG3] == ELFMAG3) \
    && ((x).e_ident[EI_CLASS] == ELFCLASS64)


int get_params_address(const char *filename, off_t *section_start, size_t *section_size) {
    Elf64_Ehdr elf_header;
    Elf64_Phdr *p_headers;
    int fd;

    MUST(fd = open(filename, O_RDONLY))

    if (pread(fd, &elf_header, sizeof(elf_header), 0) != sizeof(elf_header)) {
        close(fd);
    }

    if (!IS_ELF64_HDR(elf_header)) {
        return ERR_PARAMS;
    }

    size_t p_headers_size = elf_header.e_phentsize * elf_header.e_phnum;

    p_headers = (Elf64_Phdr *) malloc(p_headers_size);
    if (p_headers == NULL) {
        close(fd);
        return ERR_PARAMS;
    }

    if (pread(fd, p_headers, p_headers_size, elf_header.e_phoff) != p_headers_size) {
        close(fd);
        free(p_headers);
        return ERR_PARAMS;
    }
    close(fd);

    for (Elf64_Phdr *p_header = p_headers; p_header < p_headers + elf_header.e_phnum; ++p_header) {
        if (p_header->p_type == PT_PARAMS) {
            *section_start = p_header->p_vaddr;
            *section_size = p_header->p_filesz;
            free(p_headers);
            return HAS_PARAMS;
        }
    }

    free(p_headers);
    return NO_PARAMS;
}