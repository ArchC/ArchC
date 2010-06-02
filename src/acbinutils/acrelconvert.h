/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      acrelconvert.h
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
 * @date      Thu, 01 Jun 2006 14:28:06 -0300
 * 
 * @brief     ArchC binary utilities relocation converter (header file). Converts
 *            relocation codes in ELF files so an incompatible object
 *            file can be converted and linked against other object files created
 *            by an ArchC generated linker, and vice-versa.
 * 
 * 
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */



/* Function return values */

#define ACRELCONVERT_FUNC_ERROR -1
#define ACRELCONVERT_FUNC_OK 1

#define TRUE 1
#define FALSE 0

#define ACRELCONVERT_HASHTABLE_SIZE 20

/* Structures definition */

typedef struct _hash_node {
  unsigned int index;
  unsigned int value;
  struct _hash_node *next;
} hash_node;

/* Function prototypes */

int hash_initialize (hash_node *** hashtable);
int hash_add_value (hash_node **hashtable, unsigned int index, unsigned int value);
void hash_delete (hash_node ***hashtable);
int hash_get_value (hash_node **hashtable, unsigned int index, unsigned int *value);
int process_parameters(int argc, char **argv, char *reverse_mode);
int request_read(unsigned int fd, char *buf, unsigned int quantity, int is_fatal);
int process_map_file(unsigned int fd, hash_node **hashtable, char reverse_mode);
void print_usage(FILE *fp, char *appname) ;
unsigned int convert_endian(unsigned int size, unsigned int num, unsigned int match_endian);
int patch_relocation_section(unsigned int fd, unsigned int size, int rel_type, hash_node **hashtable,
			     unsigned int match_endian);
int process_elf(char* filename, hash_node **hashtable);


