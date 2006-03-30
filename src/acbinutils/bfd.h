/* ex: set tabstop=2 expandtab: */

/*! \file bfd.h
 * \brief BFD library related code
 *
 * \defgroup bfd_group BFD library
 * \ingroup binutils_group
 *
 * The BFD module deals with the aspects involved in retargetting
 * the BFD library.
 *
 * @{
 */


#ifndef _BFD_H_
#define _BFD_H_

extern void create_relocation_list();
extern int CreateRelocIds(const char *relocid_filename); 
extern int CreateRelocHowto(const char *reloc_howto_filename);
extern int CreateRelocMap(const char *relocmap_filename);


#endif /* _BFD_H_ */

/* @} */
