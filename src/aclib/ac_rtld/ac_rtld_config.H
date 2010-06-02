/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_rtld_config.H
 * @author    Rafael Auler
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     ArchC ELF runtime loader configuration file parser.
 *            This configuration file is optional, located via 
 *            library paths and contains relocation codes translation
 *            information.
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#ifndef _AC_RTLD_CONFIG_H
#define _AC_RTLD_CONFIG_H


namespace ac_dynlink {

  
  /* Structures definition */

  typedef struct _hash_node {
    unsigned int index;
    unsigned int value;
    struct _hash_node *next;
  } hash_node;

  /* Class ac_rtld_config contains methods and data necessary to locate
     and load relocation code translation information. This "map" file
     is optional and useful to enable the runtime linker to understand
     object files not created with ArchC generated linker.
   */
  class ac_rtld_config {
  private:
    hash_node **hashtable;
    bool config_loaded;
    int hash_get_value (hash_node **hashtable, unsigned int index, unsigned int *value);
    void hash_delete (hash_node ***hashtable);
    int hash_add_value (hash_node **hashtable, unsigned int index, unsigned int value);
    int hash_initialize (hash_node *** hashtable);
    int process_map_file(unsigned int fd, hash_node **hashtable);
    int request_read(unsigned int fd, char *buf, unsigned int quantity);
    int find_config_file (const char *filename);
  public:
    ac_rtld_config();
    
    ~ac_rtld_config();
    int translate(unsigned code, unsigned *result);
    bool is_config_loaded();
  };
  
}

#endif
