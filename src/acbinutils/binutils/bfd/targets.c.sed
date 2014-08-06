#/extern const bfd_target aout_mips_big_vec;/ i\
#/extern const bfd_target .*;/ a\
#extern const bfd_target xxxxx_elf32_be_vec;

/^extern const bfd_target .*$/{s/.*/&\nextern const bfd_target xxxxx_elf32_be_vec;/;:a;n;ba}


