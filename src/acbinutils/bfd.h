
#ifndef _BFD_H_
#define _BFD_H_

extern void create_relocation_list();
extern int CreateRelocIds(const char *relocid_filename); 
extern int CreateRelocHowto(const char *reloc_howto_filename);
extern int CreateRelocMap(const char *relocmap_filename);



#endif /* _BFD_H_ */

