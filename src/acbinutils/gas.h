/* ex: set tabstop=2 expandtab: */

/*! \file gas.h
 * \brief GNU assembler related code
 *
 * \defgroup gas_group GNU Assembler
 * \ingroup binutils_group
 *
 * GAS module generates target-specific code to retarget the 
 * GNU assembler.
 *  
 * @{
 */

#ifndef _GAS_H_
#define _GAS_H_

extern int CreateEncodingFunc(const char *encfunc_filename);
extern int CreateGetFieldSizeFunc(const char *getfsz_filename);
extern int CreateGetInsnSizeFunc(const char *insnsz_filename);


#endif /* _GAS_H_ */

/* @} */
