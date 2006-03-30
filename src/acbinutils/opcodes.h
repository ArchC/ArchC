/* ex: set tabstop=2 expandtab: */

/*! \file opcodes.h
 * \brief Opcodes library related code
 *
 * \defgroup opcodes_group Opcodes library
 * \ingroup binutils_group
 *
 * The Opcodes module deals with the aspects involved in retargetting
 * the Opcodes library.
 *
 * @{
 */

#ifndef _OPCODES_H_
#define _OPCODES_H_

extern int CreateOpcodeTable(const char *table_filename);
extern int CreateAsmSymbolTable(const char *symtab_filename);
extern int CreatePseudoOpsTable(const char *optable_filename);

#endif /* _OPCODES_H_ */

/* @} */

