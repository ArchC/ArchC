/#define ARCH_a29k/ i\
#define ARCH_xxxxx

/#ifdef ARCH_a29k/ i\
#ifdef ARCH_xxxxx \
    case bfd_arch_xxxxx: \
    disassemble = print_insn_xxxxx; \
    break; \
#endif
