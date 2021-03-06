#ifndef ELF32_H
#define ELF32_H

struct elf32_ehdr {
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
};

struct elf32_phdr {
    unsigned long type;
    unsigned long offset;
    unsigned long vaddr;
    unsigned long paddr;
    unsigned long filesize;
    unsigned long memsize;
    unsigned long flags;
    unsigned long align;
};

#endif // ELF32_H