#ifndef ELF32_H
#define ELF32_H

typedef struct {
    unsigned char ident[16];
    unsigned short type;
    unsigned short machine;
    unsigned long version;
    unsigned long entry;
    unsigned long phdr_off;
    unsigned long shdr_off;
    unsigned long flags;
    unsigned short ehdr_size;
    unsigned short phdr_size;
    unsigned short phdr_num;
    unsigned short shdr_size;
    unsigned short shdr_num;
    unsigned short stridx;
} elf32_ehdr;

typedef struct {
    unsigned long type;
    unsigned long offset;
    unsigned long vaddr;
    unsigned long paddr;
    unsigned long filesize;
    unsigned long memsize;
    unsigned long flags;
    unsigned long align;
} elf32_phdr;

#endif // ELF32_H