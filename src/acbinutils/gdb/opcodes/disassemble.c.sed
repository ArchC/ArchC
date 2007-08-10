/#define ARCH_alpha/ i\
#define ARCH_xxxxx

/#ifdef ARCH_alpha/ i\
#ifdef ARCH_xxxxx \
    case bfd_arch_xxxxx: \
    disassemble = print_insn_xxxxx; \
    break; \
#endif
