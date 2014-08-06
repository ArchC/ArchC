/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* ___arch_name___-specific support for ELF
   Copyright 2005, 2006, 2007, 2008, 2009 --- The ArchC Team

This file is automatically retargeted by ArchC binutils generation tool. 
This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * written by:
 *   Alexandro Baldassin
 *   Daniel Casarotto 
 *   Rafael Auler
 */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include `"elf/'___arch_name___`.h"'
#include "libiberty.h"
#include `"share-'___arch_name___`.h"'

/*
 BFD prior to version 2.16 does not include bfd_get_section_limit
*/
#ifndef bfd_get_section_limit
#define bfd_get_section_limit(bfd, sec) \
  (((sec)->size) \
   / bfd_octets_per_byte (bfd))
#endif

#ifndef ELF_DYNAMIC_INTERPRETER
/* The name of the dynamic interpreter.  This is put in the .interp
   section. 
   This is a sample, set by ArchC in case the model doesn't specify
   it in its modifiers file, so the linker still works. */
#define ELF_DYNAMIC_INTERPRETER     "/usr/lib/ld.so.1"
#endif

#ifndef PLT_HEADER_SIZE
#define PLT_HEADER_SIZE 16
/* The first entry in a procedure linkage table looks like
   this.  It is set up so that any shared library function that is
   called before the relocation has been set up calls the dynamic
   linker first.
   This is a sample, set by ArchC in case the model doesn't specify
   it in its modifiers file, so the linker still works.  */
static const bfd_vma elf_archc_plt0_entry [PLT_HEADER_SIZE / 4] =
  {
    0x00000000, /* These comments should describe the instructions */
    0x00000000, /* coded in this first plt entry.*/
    0x00000000, /* unused    */
    0x00000000, /* unused    */
  };
#endif

#ifndef PLT_ENTRY_SIZE
/* The size in bytes of an entry in the procedure linkage table.  */
#define PLT_ENTRY_SIZE 16
/* Subsequent entries in a procedure linkage table look like
   this.
   This is a sample, set by ArchC in case the model doesn't specify
   it in its modifiers file, so the linker still works. */
static const bfd_vma elf_archc_plt_entry [PLT_ENTRY_SIZE / 4] =
  {
    0x00000000,		/* These comments should describe the instructions	*/
    0x00000000,		/* coded in these PLT entries.	*/
    0x00000000,		/* unused   */
    0x00000000,		/* unused		*/
  };
#endif

/* If the model doesn't define patch methods, define dummy functions */
#ifndef ac_model_can_patch_plt
void elf_archc_patch_plt0_entry(unsigned int got_displacement, char *plt_address) {}
void elf_archc_patch_plt_entry(unsigned int got_displacement, unsigned int plt_offset, char *plt_address){}
#endif

static struct bfd_link_hash_table *elf_archc_link_hash_table_create (bfd *);
static reloc_howto_type* ___arch_name___`_reloc_type_lookup' (bfd *, bfd_reloc_code_real_type);
static void ___arch_name___`_elf_info_to_howto' (bfd *, arelent *, Elf_Internal_Rela *);
static bfd_reloc_status_type elf_archc_final_link_relocate (reloc_howto_type *, bfd *, bfd *, asection *, bfd_byte *, Elf_Internal_Rela *, bfd_vma, struct bfd_link_info *, asection *, const char *, int, struct elf_link_hash_entry *);
static bfd_reloc_status_type elf_archc_apply_modifier(unsigned int operand_id, bfd_byte * data, bfd_vma relocation, bfd_vma address, const char *name);
static bfd_boolean elf_archc_relocate_section (bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *, Elf_Internal_Rela *, Elf_Internal_Sym *, asection **);
static bfd_boolean elf_archc_create_dynamic_sections (bfd *, struct bfd_link_info *);
static bfd_boolean elf_archc_size_dynamic_sections (bfd *, struct bfd_link_info *);
static bfd_boolean allocate_dynrelocs (struct elf_link_hash_entry *, PTR);
static bfd_boolean elf_archc_finish_dynamic_sections (bfd *, struct bfd_link_info *);
static bfd_boolean elf_archc_finish_dynamic_symbol (bfd *, struct bfd_link_info *,
						    struct elf_link_hash_entry *,
						    Elf_Internal_Sym * sym);
static bfd_boolean elf_archc_adjust_dynamic_symbol (struct bfd_link_info *, struct elf_link_hash_entry *);
static asection * elf_archc_gc_mark_hook (asection *, struct bfd_link_info *, Elf_Internal_Rela *, struct elf_link_hash_entry *, Elf_Internal_Sym *);
static bfd_boolean elf_archc_gc_sweep_hook (bfd *, struct bfd_link_info *, asection *, const Elf_Internal_Rela *);
static bfd_boolean elf_archc_check_relocs (bfd *, struct bfd_link_info *, asection *, const Elf_Internal_Rela *);
static bfd_boolean elf_archc_always_size_sections (bfd *, struct bfd_link_info *);
static struct bfd_hash_entry * elf_archc_link_hash_newfunc (struct bfd_hash_entry *, struct bfd_hash_table *, const char *);
static bfd_boolean create_got_section (bfd * dynobj, struct bfd_link_info * info);
static enum elf_reloc_type_class elf_archc_reloc_type_class (const Elf_Internal_Rela *);
int elf_archc_additional_program_headers (bfd *);

/* ArchC generic relocation routine prototype*/
static bfd_reloc_status_type
bfd_elf_archc_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED);

static bfd_reloc_status_type
generic_data_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED);



/* Local label (those who starts with $ are recognized as well) */
static bfd_boolean
___arch_name___`_elf_is_local_label_name' (bfd *abfd, const char *name)
{
  if (name[0] == '$')
    return TRUE;

  return _bfd_elf_is_local_label_name(abfd, name);
}

static reloc_howto_type `elf_'___arch_name___`_howto_table'[] =
{
___reloc_howto___
};

struct ___arch_name___`_reloc_map'
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int ___arch_name___`_reloc_val';
};

static const struct ___arch_name___`_reloc_map' ___arch_name___`_reloc_map'[] =
{
___reloc_map___
};

/* This structure keeps track of the number of relocs we
   have copied for a given symbol.  */
struct elf_archc_dyn_relocs
  {
    /* Next section.  */
    struct elf_archc_dyn_relocs * next;
    /* A section in dynobj.  */
    asection * section;
    /* Number of relocs copied in this section.  */
    bfd_size_type count;
  };

/* ArchC linker hash entry.  */
struct elf_archc_link_hash_entry
  {
    struct elf_link_hash_entry root;

    /* Number of relocs copied for this symbol.  */
    struct elf_archc_dyn_relocs * relocs_copied;
  };

/* Traverse an archc ELF linker hash table.  */
#define elf_archc_link_hash_traverse(table, func, info)			\
  (elf_link_hash_traverse						\
   (&(table)->root,							\
    (bfd_boolean (*) PARAMS ((struct elf_link_hash_entry *, PTR))) (func), \
    (info)))

/* Get the ArchC elf linker hash table from a link_info structure.  */
#define elf_archc_hash_table(info) \
  ((struct elf_archc_link_hash_table *) ((info)->hash))

/* ArchC ELF linker hash table.  */
struct elf_archc_link_hash_table
  {
    /* The main hash table.  */
    struct elf_link_hash_table root;

    /* Short-cuts to get to dynamic linker sections.  */
    asection *sgot;
    asection *sgotplt;
    asection *srelgot;
    asection *splt;
    asection *srelplt;
    asection *sdynbss;
    asection *srelbss;

    /* Small local sym to section mapping cache.  */
    struct sym_cache sym_sec;
  };

/* Create an entry in an ArchC ELF linker hash table.  */

static struct bfd_hash_entry *
elf_archc_link_hash_newfunc (entry, table, string)
     struct bfd_hash_entry * entry;
     struct bfd_hash_table * table;
     const char * string;
{
  struct elf_archc_link_hash_entry * ret =
    (struct elf_archc_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == (struct elf_archc_link_hash_entry *) NULL)
    ret = ((struct elf_archc_link_hash_entry *)
	   bfd_hash_allocate (table,
			      sizeof (struct elf_archc_link_hash_entry)));
  if (ret == (struct elf_archc_link_hash_entry *) NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_archc_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != (struct elf_archc_link_hash_entry *) NULL)
    ret->relocs_copied = NULL;

  return (struct bfd_hash_entry *) ret;
}

static bfd_boolean
create_got_section (dynobj, info)
     bfd *dynobj;
     struct bfd_link_info *info;
{
  struct elf_archc_link_hash_table *htab;

  if (! _bfd_elf_create_got_section (dynobj, info))
    return FALSE;

  htab = elf_archc_hash_table (info);
  htab->sgot = bfd_get_section_by_name (dynobj, ".got");
  htab->sgotplt = bfd_get_section_by_name (dynobj, ".got.plt");
  if (!htab->sgot || !htab->sgotplt)
    abort ();

  htab->srelgot = bfd_make_section (dynobj, ".rela.got");
  if (htab->srelgot == NULL
      || ! bfd_set_section_flags (dynobj, htab->srelgot,
				  (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS
				   | SEC_IN_MEMORY | SEC_LINKER_CREATED
				   | SEC_READONLY))
      || ! bfd_set_section_alignment (dynobj, htab->srelgot, 2))
    return FALSE;
  return TRUE;
}


/* Backend routine responsible for creating dynamic sections. Just
call regular elf routines to create these sections. Also creates
.rel.got section. */
static bfd_boolean
elf_archc_create_dynamic_sections(dynobj, info)
     bfd *dynobj;
     struct bfd_link_info *info;
{
  struct elf_archc_link_hash_table *htab;

  htab = elf_archc_hash_table (info);
  if (!htab->sgot && !create_got_section (dynobj, info))
    return FALSE;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return FALSE;

  htab->splt = bfd_get_section_by_name (dynobj, ".plt");
  htab->srelplt = bfd_get_section_by_name (dynobj, ".rela.plt");
  htab->sdynbss = bfd_get_section_by_name (dynobj, ".dynbss");
  if (!info->shared)
    htab->srelbss = bfd_get_section_by_name (dynobj, ".rela.bss");

  if (!htab->splt || !htab->srelplt || !htab->sdynbss
      || (!info->shared && !htab->srelbss))
    abort ();

  return TRUE;
}


/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
elf_archc_copy_indirect_symbol (struct bfd_link_info *bed,
				struct elf_link_hash_entry *dir,
				struct elf_link_hash_entry *ind)
{
  struct elf_archc_link_hash_entry *edir, *eind;

  edir = (struct elf_archc_link_hash_entry *) dir;
  eind = (struct elf_archc_link_hash_entry *) ind;

  if (eind->relocs_copied != NULL)
    {
      if (edir->relocs_copied != NULL)
	{
	  struct elf_archc_dyn_relocs **pp;
	  struct elf_archc_dyn_relocs *p;

	  if (ind->root.type == bfd_link_hash_indirect)
	    abort ();

	  /* Add reloc counts against the weak sym to the strong sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &eind->relocs_copied; (p = *pp) != NULL; )
	    {
	      struct elf_archc_dyn_relocs *q;

	      for (q = edir->relocs_copied; q != NULL; q = q->next)
		if (q->section == p->section)
		  {
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = edir->relocs_copied;
	}

      edir->relocs_copied = eind->relocs_copied;
      eind->relocs_copied = NULL;
    }

  _bfd_elf_link_hash_copy_indirect (bed, dir, ind);
}


/* Create an ArchC elf linker hash table.  */

static struct bfd_link_hash_table *
elf_archc_link_hash_table_create (abfd)
     bfd *abfd;
{
  struct elf_archc_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct elf_archc_link_hash_table);

  ret = (struct elf_archc_link_hash_table *) bfd_malloc (amt);
  if (ret == (struct elf_archc_link_hash_table *) NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      elf_archc_link_hash_newfunc,
                      sizeof (struct elf_archc_link_hash_entry),
                      GENERIC_ELF_DATA))   // FIXME
    {
      free (ret);
      return NULL;
    }

  ret->sgot = NULL;
  ret->sgotplt = NULL;
  ret->srelgot = NULL;
  ret->splt = NULL;
  ret->srelplt = NULL;
  ret->sdynbss = NULL;
  ret->srelbss = NULL;
  ret->sym_sec.abfd = NULL;

  return &ret->root.root;
}

static reloc_howto_type* ___arch_name___`_reloc_type_lookup' (abfd, code)
  bfd * abfd ATTRIBUTE_UNUSED;
  bfd_reloc_code_real_type code;
{
  unsigned int i;
  for (i = ARRAY_SIZE (___arch_name___`_reloc_map'); --i;)
    if (___arch_name___`_reloc_map'[i].bfd_reloc_val == code)
      return & `elf_'___arch_name___`_howto_table'[___arch_name___`_reloc_map'[i].___arch_name___`_reloc_val'];

  return NULL;
}

static void ___arch_name___`_elf_info_to_howto' (abfd, cache_ptr, dst)
  bfd *abfd ATTRIBUTE_UNUSED;
  arelent *cache_ptr;
  Elf_Internal_Rela *dst;
{
  unsigned int r;
  r = ELF32_R_TYPE (dst->r_info);

  BFD_ASSERT (r < (unsigned int) `R_'___arch_name___`_max');

  cache_ptr->howto = &`elf_'___arch_name___`_howto_table'[r];
}

#if BFD_REQUIRES_RELOC_NAME_LOOKUP
static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name ATTRIBUTE_UNUSED)
{
  return NULL;
}
#endif


static bfd_reloc_status_type
elf_archc_apply_modifier(unsigned int operand_id, bfd_byte * data,
				   bfd_vma relocation, bfd_vma address, const char *name)
{
  mod_parms modifier_parms;

  unsigned int insn_image = (unsigned int) getbits(get_insn_size(operands[operand_id].format_id), (char *) data, ___endian_val___);
  
  modifier_parms.input   = relocation;
  modifier_parms.address = address;
  modifier_parms.section = name;
  modifier_parms.list_results = NULL;

  encode_cons_field(&insn_image, &modifier_parms, operand_id);

  if (modifier_parms.error) {
    _bfd_error_handler (_("Invalid relocation operation"));
  }

  putbits(get_insn_size(operands[operand_id].format_id), (char *) data, insn_image, ___endian_val___); 
  
  return bfd_reloc_ok;
}

/* Perform a relocation as part of a final link. 
   Called by elf_archc_relocate_section() */

static bfd_reloc_status_type
elf_archc_final_link_relocate (howto, input_bfd, output_bfd,
			       input_section, contents, rel, value,
			       info, sym_sec, sym_name, sym_flags, h)
     reloc_howto_type *     howto;          /* relocation howto */
     bfd *                  input_bfd;      /* bfd owning input_section (being relocated)*/
     bfd *                  output_bfd;     /* bfd product of the link process */
     asection *             input_section;  /* input_section being relocated */
     bfd_byte *             contents;       /* contents of input_section */
     Elf_Internal_Rela *    rel;            /* elf internal relocation entry */
     bfd_vma                value;          /* precalculated relocation value */
     struct bfd_link_info * info;           /* link_info (details about the link)*/
     asection *             sym_sec ATTRIBUTE_UNUSED;        /* section referenced by relocation's symbol */
     const char *           sym_name ATTRIBUTE_UNUSED;       /* symbol name */
     int		    sym_flags ATTRIBUTE_UNUSED;      /* symbol flags */
     struct elf_link_hash_entry * h;        /* hash entry */
{

  unsigned long                 r_type = howto->type; /* relocation type*/
  unsigned long                 r_symndx;             /* symbol index */
  bfd_byte *                    hit_data = contents + rel->r_offset;
  bfd *                         dynobj = NULL;
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  bfd_vma *                     local_got_offsets;
  asection *                    sgot = NULL;
  asection *                    splt = NULL;
  asection *                    sreloc = NULL;
  bfd_vma                       addend;
  bfd_signed_vma                signed_addend;
  struct elf_archc_link_hash_table * globals;

  globals = elf_archc_hash_table(info);

  dynobj = elf_hash_table (info)->dynobj;
  if (dynobj)
    {
      sgot = bfd_get_section_by_name (dynobj, ".got");
      splt = bfd_get_section_by_name (dynobj, ".plt");
    }
  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  local_got_offsets = elf_local_got_offsets (input_bfd);
  r_symndx = ELF32_R_SYM (rel->r_info);

  addend = signed_addend = rel->r_addend;


  /* When generating a shared object, these relocations are copied
     into the output file to be resolved at run time. */
  if (info->shared
      && (input_section->flags & SEC_ALLOC)
      && (((r_type == `R_'___arch_name___`_REL32' || r_type == `R_'___arch_name___`_REL16' || r_type == `R_'___arch_name___`_REL8)' && ((h != NULL) && ((h->def_regular) == 0) && !SYMBOL_CALLS_LOCAL(info,h))) || r_type == `R_'___arch_name___`_32' || r_type == `R_'___arch_name___`_16' || r_type == `R_'___arch_name___`_8)'
      && (h == NULL
	  || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	  || h->root.type != bfd_link_hash_undefweak) )
    {
      Elf_Internal_Rela outrel;
      bfd_byte *loc;
      bfd_boolean skip, relocate;

      if (sreloc == NULL)
	{
	  const char * name;
	  
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd,
		   elf_elfheader (input_bfd)->e_shstrndx,
		   elf_section_data (input_section)->rela.hdr->sh_name));
	  if (name == NULL)
	    return bfd_reloc_notsupported;

	  BFD_ASSERT (strncmp (name, ".rela", 5) == 0
		      && strcmp (bfd_get_section_name (input_bfd,
						       input_section),
				 name + 5) == 0);
	  
	  sreloc = bfd_get_section_by_name (dynobj, name);
	  BFD_ASSERT (sreloc != NULL);
	}

      skip = FALSE;
      relocate = FALSE;

      outrel.r_offset =
	_bfd_elf_section_offset (output_bfd, info, input_section,
				     rel->r_offset);
      if (outrel.r_offset == (bfd_vma) -1)
	skip = TRUE;
      else if (outrel.r_offset == (bfd_vma) -2)
	skip = TRUE, relocate = TRUE;
      outrel.r_offset += (input_section->output_section->vma
			  + input_section->output_offset);
      outrel.r_addend = 0;

      if (skip)
	memset (&outrel, 0, sizeof outrel);
      else if (h != NULL
	       && h->dynindx != -1
	       && (!info->shared
		   || !info->symbolic
		   || (h->def_regular) == 0))
	outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
      else
	{
	  /* This symbol is local, or marked to become local.  */
	  relocate = TRUE;
	  outrel.r_info = ELF32_R_INFO (0, `R_'___arch_name___`_RELATIVE);'
	}

      loc = sreloc->contents;
      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

      /* If this reloc is against an external symbol, we do not want to
	 fiddle with the addend.  Otherwise, we need to include the symbol
	 value so that it becomes an addend for the dynamic reloc.  */
      if (! relocate)
	return bfd_reloc_ok;           
    }
   

  switch(r_type) {
  case `R_'___arch_name___`_NONE:'
    return bfd_reloc_ok;

  case `R_'___arch_name___`_32:'
    value += addend;
    bfd_put_32 (input_bfd, value, hit_data);
    return bfd_reloc_ok;

  case `R_'___arch_name___`_REL32:'
    value -= (input_section->output_section->vma
	      + input_section->output_offset + rel->r_offset);
    value += addend;
    bfd_put_32 (input_bfd, value, hit_data);
    return bfd_reloc_ok;

  case `R_'___arch_name___`_REL16:'
    value -= (input_section->output_section->vma
	      + input_section->output_offset + rel->r_offset);
    value += addend;
    if ((long) value > 0x7fff || (long) value < -0x8000)
      return bfd_reloc_overflow;
    bfd_put_16 (input_bfd, value, hit_data);
    return bfd_reloc_ok;

  case `R_'___arch_name___`_REL8' :
    value -= (input_section->output_section->vma
	      + input_section->output_offset + rel->r_offset);
    value += addend;
    if ((long) value > 0x7f || (long) value < -0x80)
      return bfd_reloc_overflow;
    bfd_put_8 (input_bfd, value, hit_data);
    return bfd_reloc_ok;

  case `R_'___arch_name___`_8:'
    value += addend;
    if ((long) value > 0x7f || (long) value < -0x80)
      return bfd_reloc_overflow;
    
    bfd_put_8 (input_bfd, value, hit_data);
    return bfd_reloc_ok;
    
  case `R_'___arch_name___`_16:'
    value += addend;

    if ((long) value > 0x7fff || (long) value < -0x8000)
      return bfd_reloc_overflow;

    bfd_put_16 (input_bfd, value, hit_data);
    return bfd_reloc_ok;

  case `R_'___arch_name___`_GOTOFF:'
    /* Relocation is relative to the start of the
       global offset table.  */

    if (sgot == NULL)
      return bfd_reloc_notsupported;
   
    /* Note that sgot->output_offset is not involved in this
       calculation.  We always want the start of .got.  If we
       define _GLOBAL_OFFSET_TABLE in a different way,
       we might have to change this calculation.  */
    value -= sgot->output_section->vma;
    bfd_put_32 (input_bfd, value, hit_data);
    return bfd_reloc_ok;    

  case `R_'___arch_name___`_GOT:'
    /* Relocation is to the entry for this symbol in the
       global offset table.  */
    if (sgot == NULL)
	return bfd_reloc_notsupported;
    
    if (h != NULL) //global
      {
	bfd_vma off;
	bfd_boolean dyn;
       
	off = h->got.offset;
	BFD_ASSERT (off != (bfd_vma) -1); 
	dyn = globals->root.dynamic_sections_created;	

	if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, info->shared, h)
	    || (info->shared
		&& SYMBOL_REFERENCES_LOCAL (info, h))
	    || (ELF_ST_VISIBILITY (h->other)
		&& h->root.type == bfd_link_hash_undefweak))
	  {
	    /* This is actually a static link, or it is a -Bsymbolic link
	       and the symbol is defined locally.  We must initialize this
	       entry in the global offset table.  Since the offset must
	       always be a multiple of 4, we use the least significant bit
	       to record whether we have initialized it already.
	       
	       When doing a dynamic link, we create a .rel.got relocation
	       entry to initialize the value.  This is done in the
	       finish_dynamic_symbol routine.  */
	    if ((off & 1) != 0)
	      off &= ~1;
	    else
	      {		  
		bfd_put_32 (output_bfd, value, sgot->contents + off);
		h->got.offset |= 1;
	      }
	  }

	value = sgot->output_offset + off;
      }
    else // local
      {
	bfd_vma off;
	
	BFD_ASSERT (local_got_offsets != NULL &&
		    local_got_offsets[r_symndx] != (bfd_vma) -1);

	off = local_got_offsets[r_symndx];

	/* The offset must always be a multiple of 4.  We use the
	   least significant bit to record whether we have already
	   generated the necessary reloc.  */
	if ((off & 1) != 0)
	  off &= ~1;
	else
	  {
	    bfd_put_32 (output_bfd, value, sgot->contents + off);
	    
	    if (info->shared)
	      {
		asection * srelgot;
		Elf_Internal_Rela outrel;
		bfd_byte *loc;

		srelgot = bfd_get_section_by_name (dynobj, ".rela.got");
		BFD_ASSERT (srelgot != NULL);
		
		outrel.r_offset = (sgot->output_section->vma
				   + sgot->output_offset
				   + off);
		outrel.r_info = ELF32_R_INFO (0, `R_'___arch_name___`_RELATIVE);' 
		outrel.r_addend = 0;
		loc = srelgot->contents;
		loc += srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
		bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
	      }

	    local_got_offsets[r_symndx] |= 1;
	  }
	
	value = sgot->output_offset + off;
      }
    bfd_put_32 (input_bfd, value, hit_data);
    return bfd_reloc_ok;     
  }

  // Apply addend
  value += addend;

  /* r_symndx will be zero only for relocs against symbols
     from removed linkonce sections, or sections discarded by
     a linker script.  */
  if (r_symndx == 0)
    return bfd_reloc_ok;

  /* Handle relocations which should use the PLT entry.  ABS32/REL32
     will use the symbol's value, which may point to a PLT entry, but we
     don't need to handle that here.  If we created a PLT entry, all
     branches in this object should go to it.  */
  if (h != NULL
      && splt != NULL
      && h->plt.offset != (bfd_vma) -1)
    {
      /* If we've created a .plt section, and assigned a PLT entry to
	 this function, it should not be known to bind locally.  If
	 it were, we would have cleared the PLT entry.  */
      BFD_ASSERT (!SYMBOL_CALLS_LOCAL (info, h));
      
      value = (splt->output_section->vma
	       + splt->output_offset
	       + h->plt.offset);
    
      return elf_archc_apply_modifier (howto->rightshift, hit_data, value,
					input_section->output_section->vma
					+ input_section->output_offset + rel->r_offset,
					bfd_get_section_name (input_bfd, input_section));
    }
  
  return elf_archc_apply_modifier (howto->rightshift, hit_data, value,
				    input_section->output_section->vma
				    + input_section->output_offset + rel->r_offset,
				    bfd_get_section_name (input_bfd, input_section));
}


/* Relocate an entire section as part of ELF final link phase.
   Called by elf_link_input_bfd() on behalf of bfd_elf_final_link()  */
static bfd_boolean
elf_archc_relocate_section (output_bfd, info, input_bfd, input_section,
			    contents, relocs, local_syms, local_sections)
     bfd *output_bfd;            // the result of the link process
     struct bfd_link_info *info; // link_info structure containing input_bfds feeding
                                 //   the link process, and other info about the output
     bfd *input_bfd;             // bfd owning input_section
     asection *input_section;    // section whose relocations must be resolved
     bfd_byte *contents;         // section contents obtained with bfd_get_section_contents()
     Elf_Internal_Rela *relocs;  // vector with all section relocations
     Elf_Internal_Sym *local_syms; // vector with local symbols
     asection **local_sections;  // vector with sections corresponding to the st_shndx field 
                                 // of a local symbol

{

  Elf_Internal_Shdr *symtab_hdr; // Symbol table elf header
  struct elf_link_hash_entry **sym_hashes; // Symbol hashes
  Elf_Internal_Rela *rel;        // elf relocation
  Elf_Internal_Rela *relend;     // marks the last relocation in the list
  const char *name;              // symbol name

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);

  if (info->relocatable)
    return TRUE;

  /* Iterates through all relocations on this section */
  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int                          r_type;     // ELF relocation type
      reloc_howto_type *           howto;    
      unsigned long                r_symndx;   // relocation symbol index, consult in local
                                               //  symbols table
      Elf_Internal_Sym *           sym;        // relocation symbol
      asection *                   sec;        // bfd section pointer 
      struct elf_link_hash_entry * h;
      bfd_vma                      relocation; // result
      bfd_reloc_status_type        r;          // relocation status
      arelent                      bfd_reloc;  // canonical bfd relocation form

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type   = ELF32_R_TYPE (rel->r_info);

      ___arch_name___`_elf_info_to_howto' (input_bfd, & bfd_reloc, rel);
      howto = bfd_reloc.howto; 

      h = NULL;
      sym = NULL;
      sec = NULL;

      // Relocation against local symbol?
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	 
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else // Global
	{
	  bfd_boolean warned;
	  bfd_boolean unresolved_reloc;
      bfd_boolean ignored;

	  // Macro defined in elf-bfd.h, retrives values for h, sym, sec,
	  // relocation, unresolved_reloc and warned accordingly
	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned,ignored);

	  // Check for special cases
	  if (unresolved_reloc || relocation != 0)
	    {
	      if (howto->type == `R_'___arch_name___`_GOT)'
	        {
	          if ((WILL_CALL_FINISH_DYNAMIC_SYMBOL
		       (elf_hash_table (info)->dynamic_sections_created,
			info->shared, h))
		      && (!info->shared
	                  || (!info->symbolic && h->dynindx != -1)
	                  || (h->def_regular) == 0))
	            relocation = 0;		 
		}
	      else 
		if (unresolved_reloc)
		  _bfd_error_handler
		    (_("%B: %s: warning: unresolvable relocation %d against symbol %s from %s section"),
		     input_bfd,
		     r_type,
		     h->root.root.string,
		     bfd_get_section_name (input_bfd, input_section));		 	    
        }
	}

      /* Retrieving symbol name */
      if (h != NULL)
	name = h->root.root.string;
      else
	{
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd, symtab_hdr->sh_link, sym->st_name));
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (input_bfd, sec);
	}

      r = elf_archc_final_link_relocate (howto, input_bfd, output_bfd,
					 input_section, contents, rel,
					 relocation, info, sec, name,
					 (h ? ELF_ST_TYPE (h->type) :
					  ELF_ST_TYPE (sym->st_info)), h);

      if (r != bfd_reloc_ok)
	{
	  const char * msg = (const char *) 0;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      /* If the overflowing reloc was to an undefined symbol,
		 we have already printed one error message and there
		 is no point complaining again.  */
	      if ((! h ||
		   h->root.type != bfd_link_hash_undefined)
		  && (!((*info->callbacks->reloc_overflow)
			    (info, (h ? &h->root : NULL), 
                name, howto->name, (bfd_vma) 0,
    		    input_bfd, input_section, rel->r_offset))))
		  return FALSE;
	      break;

	    case bfd_reloc_undefined:
	      if (!((*info->callbacks->undefined_symbol)
		    (info, name, input_bfd, input_section,
		     rel->r_offset, TRUE)))
		return FALSE;
	      break;

	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      msg = _("internal error: unsupported relocation error");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous error");
	      goto common_error;

	    default:
	      msg = _("internal error: unknown error");
	      /* fall through */

	    common_error:
	      if (!((*info->callbacks->warning)
		    (info, msg, name, input_bfd, input_section,
		     rel->r_offset)))
		return FALSE;
	      break;
	    }
	}
    }

  return TRUE;
}

/* This function is called during section gc to discover the section a
     particular relocation refers to.  */
static asection *
elf_archc_gc_mark_hook (sec, info, rel, h, sym)
       asection *sec;
       struct bfd_link_info *info ATTRIBUTE_UNUSED;
       Elf_Internal_Rela *rel ATTRIBUTE_UNUSED;
       struct elf_link_hash_entry *h;
       Elf_Internal_Sym *sym;
{
  if (h != NULL)
    {
      switch (h->root.type)
	{
	case bfd_link_hash_defined:
	case bfd_link_hash_defweak:
	  return h->root.u.def.section;

	case bfd_link_hash_common:
	  return h->root.u.c.p->section;

	default:
	  break;
	}
    }
  else
    return bfd_section_from_elf_index (sec->owner, sym->st_shndx);
  
  return NULL;
}

/* Update the got entry reference counts for the section being removed.  */
static bfd_boolean
elf_archc_gc_sweep_hook (abfd, info, sec, relocs)
     bfd *abfd;
     struct bfd_link_info *info ATTRIBUTE_UNUSED;
     asection *sec;
     const Elf_Internal_Rela *relocs;
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_signed_vma *local_got_refcounts;
  const Elf_Internal_Rela *rel, *relend;
  unsigned long r_symndx;
  struct elf_link_hash_entry *h;

  elf_section_data (sec)->local_dynrel = NULL;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case `R_'___arch_name___`_GOT:'
	r_symndx = ELF32_R_SYM (rel->r_info);
	if (r_symndx >= symtab_hdr->sh_info)
	  { /* Global symbol */
	    h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	    if (h->got.refcount > 0)
	      h->got.refcount -= 1;
	  }
	else if (local_got_refcounts != NULL)
	  { /* Symbol is local and we have allocated local refcounts table */
	    if (local_got_refcounts[r_symndx] > 0)
	      local_got_refcounts[r_symndx] -= 1;
	  }
	break;

      default:
	r_symndx = ELF32_R_SYM (rel->r_info);
	if (r_symndx >= symtab_hdr->sh_info)
	  {
	    struct elf_archc_link_hash_entry *eh;
	    struct elf_archc_dyn_relocs **pp;
	    struct elf_archc_dyn_relocs *p;

	    h = sym_hashes[r_symndx - symtab_hdr->sh_info];

	    if (h->plt.refcount > 0)
	      h->plt.refcount -= 1;

	    /* Should we update copy relocs for this symbol? */
	    if (ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_32'
		|| ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_16'
		|| ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_8'
		|| ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_REL32'
		|| ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_REL16'
		|| ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_REL8)'
	      {
		eh = (struct elf_archc_link_hash_entry *) h;

		for (pp = &eh->relocs_copied; (p = *pp) != NULL;
		     pp = &p->next)
		if (p->section == sec)
		  {
		    p->count -= 1;
		    if (p->count == 0)
		      *pp = p->next;
		    break;
		  }
	      }
	  }
	break;
      }

  return TRUE;
}

/* Look through the relocs for a section during the first phase.
   Count got/plt references, mark symbols when PLT/GOT entry
   is needed. */

static bfd_boolean
elf_archc_check_relocs (abfd, info, sec, relocs)
     bfd *abfd;
     struct bfd_link_info *info;
     asection *sec;
     const Elf_Internal_Rela *relocs;
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **sym_hashes_end;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  bfd *dynobj;
  asection *sreloc;
  bfd_vma *local_got_offsets;
  struct elf_archc_link_hash_table *htab;

  if (info->relocatable)
    return TRUE;

  htab = elf_archc_hash_table (info);
  sreloc = NULL;

  dynobj = elf_hash_table (info)->dynobj;
  local_got_offsets = elf_local_got_offsets (abfd);

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sym_hashes_end = sym_hashes
    + symtab_hdr->sh_size / sizeof (Elf32_External_Sym);

  if (!elf_bad_symtab (abfd))
    sym_hashes_end -= symtab_hdr->sh_info;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
        h = NULL;
      else
        h = sym_hashes[r_symndx - symtab_hdr->sh_info];

      switch (ELF32_R_TYPE (rel->r_info))
        {
	  case `R_'___arch_name___`_GOT:'
	    /* This symbol requires a global offset table entry.  */
	    if (h != NULL)
	      {
		h->got.refcount++;
	      }
	    else
	      {
		bfd_signed_vma *local_got_refcounts;

		/* This is a global offset table entry for a local symbol.  */
		local_got_refcounts = elf_local_got_refcounts (abfd);
		if (local_got_refcounts == NULL)
		  {
		    bfd_size_type size;

		    size = symtab_hdr->sh_info;
		    size *= (sizeof (bfd_signed_vma) + sizeof(char));
		    local_got_refcounts = ((bfd_signed_vma *)
					   bfd_zalloc (abfd, size));
		    if (local_got_refcounts == NULL)
		      return FALSE;
		    elf_local_got_refcounts (abfd) = local_got_refcounts;
		  }
		local_got_refcounts[r_symndx] += 1;
	      }
	    break;

	  case `R_'___arch_name___`_GOTOFF:'
	    if (htab->sgot == NULL)
	      {
		if (htab->root.dynobj == NULL)
		  htab->root.dynobj = abfd;
		if (!create_got_section (htab->root.dynobj, info))
		  return FALSE;
	      }
	    break;

          default:
	    if (h != NULL) /* global symbol */
	      {
		/* Relocation against GOT symbol, need GOT table */
		if (strncmp(h->root.root.string, "_GLOBAL_OFFSET_TABLE_", 21)==0)
		  {
		    if (htab->sgot == NULL)
		      {
			if (htab->root.dynobj == NULL)
			  htab->root.dynobj = abfd;
			if (!create_got_section (htab->root.dynobj, info))
			  return FALSE;
		      }
		    break;
		  } 
		/* If this reloc is in a read-only section, we might
		   need a copy reloc.  We can't check reliably at this
		   stage whether the section is read-only, as input
		   sections have not yet been mapped to output sections.
		   Tentatively set the flag for now, and correct in
		   adjust_dynamic_symbol.  */
		if (!info->shared)
            h->non_got_ref = 1;

		/* We may need a .plt entry if the function this reloc
		   refers to is in a different object.  We can't tell for
		   sure yet, because something later might force the
		   symbol local.  */
		if (h->type == STT_FUNC)
            h->needs_plt = 1;

		/* If we create a PLT entry, this relocation will reference
		   it.  */
		h->plt.refcount += 1;
	      }

	    /* If we are creating a shared library, and this is a reloc
               against a global symbol, or a non PC relative reloc
               against a local symbol, then we need to copy the reloc
               into the shared library.  However, if we are linking with
               -Bsymbolic, we do not need to copy a reloc against a
               global symbol which is defined in an object we are
               including in the link (i.e., DEF_REGULAR is set).  At
               this point we have not seen all the input files, so it is
               possible that DEF_REGULAR is not set now but will be set
               later (it is never cleared).  We account for that
               possibility below by storing information in the
               relocs_copied field of the hash table entry.  */
	    /* Usually we would copy any relocation against a global
	       symbol not defined in a regular object, but as ArchC 
	       generates custom relocations for each target, we
	       leave to the dynamic linker only those it can solve, 
	       like absolute 32-bit addresses, etc. This is not a
	       strong restriction, since leaving regular relocations
	       to the dynamic linker is contrary to the whole dynamic
	       linking principle, which we shall not modify readonly
	       text sections (and these regular relocations do) */
	    if (info->shared
		&& (sec->flags & SEC_ALLOC) != 0
		&& (ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_32'
		    || ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_16'
		    || ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_8'
		    || ((ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_REL32'
			 || ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_REL16'
			 || ELF32_R_TYPE (rel->r_info) == `R_'___arch_name___`_REL8)' &&
			(h != NULL
			 && (! info->symbolic
			     || (h->def_regular) == 0)))
		    ))
	      {
		struct elf_archc_dyn_relocs *p, **head;

	        /* When creating a shared object, we must copy these
                   reloc types into the output file.  We create a reloc
                   section in dynobj and make room for this reloc.  */
	        if (sreloc == NULL)
		  {
		    const char * name;

		    name = (bfd_elf_string_from_elf_section
			    (abfd,
			     elf_elfheader (abfd)->e_shstrndx,
                 //MSG: this_hdr ou rela?
			     elf_section_data (sec)->rela.hdr->sh_name));
		    if (name == NULL)
		      return FALSE;

		    BFD_ASSERT (strncmp (name, ".rela", 5) == 0
			        && strcmp (bfd_get_section_name (abfd, sec),
					   name + 5) == 0);

		    sreloc = bfd_get_section_by_name (dynobj, name);
		    if (sreloc == NULL)
		      {
		        flagword flags;

		        sreloc = bfd_make_section (dynobj, name);
		        flags = (SEC_HAS_CONTENTS | SEC_READONLY
			         | SEC_IN_MEMORY | SEC_LINKER_CREATED);
		        if ((sec->flags & SEC_ALLOC) != 0)
			  flags |= SEC_ALLOC | SEC_LOAD;
		        if (sreloc == NULL
			    || ! bfd_set_section_flags (dynobj, sreloc, flags)
			    || ! bfd_set_section_alignment (dynobj, sreloc, 2))
			  return FALSE;
		      }

		    elf_section_data (sec)->sreloc = sreloc;
		  }

		/* If this is a global symbol, we count the number of
		   relocations we need for this symbol.  */
		if (h != NULL)
		  {
		    head = &((struct elf_archc_link_hash_entry *) h)->relocs_copied;
		  }
		else
		  {
		    /* Track dynamic relocs needed for local syms too.
		       We really need local syms available to do this
		       easily.  Oh well.  */
		    

            
            Elf_Internal_Sym *isym;
		    isym = bfd_sym_from_r_symndx (&htab->sym_sec, abfd, r_symndx);
		    if (isym == NULL)
		      return FALSE;
		   
            asection *s;
		    s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		    if (s == NULL)
		        s = sec;

		    head = ((struct elf_archc_dyn_relocs **)
			    &elf_section_data (s)->local_dynrel);
		  }
		
		p = *head;
		if (p == NULL || p->section != sec)
		  {
		    bfd_size_type amt = sizeof *p;
		    p = bfd_alloc (htab->root.dynobj, amt);
		    if (p == NULL)
		      return FALSE;
		    p->next = *head;
		    *head = p;
		    p->section = sec;
		    p->count = 0;
		  }
		
		p->count += 1;
	      }
	    
	    break;

        }
    }

  return TRUE;
}



/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bfd_boolean
elf_archc_adjust_dynamic_symbol (info, h)
     struct bfd_link_info * info;
     struct elf_link_hash_entry * h;
{
  bfd * dynobj;
  asection * s;
  unsigned int power_of_two;

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->u.weakdef != NULL
		  || (h->def_dynamic
		      && h->def_regular
		      && !h->def_regular)));

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC
      || (h->needs_plt) != 0)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table. */
	  h->plt.offset = (bfd_vma) -1;
//	  h->elf_link_hash_flags &= ~ELF_LINK_HASH_NEEDS_PLT;
      h->needs_plt = 0;
	}

      return TRUE;
    }
  else
    /* It's possible that we incorrectly decided a .plt reloc was
       needed for a reloc to a non-function sym in
       check_relocs.  We can't decide accurately between function and
       non-function syms in check-relocs;  Objects loaded later in
       the link may change h->type.  So fix it now.  */
    h->plt.offset = (bfd_vma) -1;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
		  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
      return TRUE;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (info->shared)
    return TRUE;

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */
  s = bfd_get_section_by_name (dynobj, ".dynbss");
  BFD_ASSERT (s != NULL);

  /* We must generate a `R_'___arch_name___`_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rel.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      asection *srel;

      srel = bfd_get_section_by_name (dynobj, ".rela.bss");
      BFD_ASSERT (srel != NULL);
      srel->rawsize += sizeof (Elf32_External_Rela);
//    h->elf_link_hash_flags |= ELF_LINK_HASH_NEEDS_COPY;
      h->needs_copy = 1;
    }

  /* We need to figure out the alignment required for this symbol.  I
     have no idea how ELF linkers handle this.  */
  power_of_two = bfd_log2 (h->size);
  if (power_of_two > 3)
    power_of_two = 3;

  /* Apply the required alignment.  */
  s->rawsize = BFD_ALIGN (s->rawsize,
			    (bfd_size_type) (1 << power_of_two));
  if (power_of_two > bfd_get_section_alignment (dynobj, s))
    {
      if (! bfd_set_section_alignment (dynobj, s, power_of_two))
	return FALSE;
    }

  /* Define the symbol as being at this point in the section.  */
  h->root.u.def.section = s;
  h->root.u.def.value = s->rawsize;

  /* Increment the section size to make room for the symbol.  */
  s->rawsize += h->size;

  return TRUE;
}


/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bfd_boolean
allocate_dynrelocs (h, inf)
     struct elf_link_hash_entry *h;
     PTR inf;
{
  struct bfd_link_info *info;
  struct elf_archc_link_hash_table *htab;
  struct elf_archc_link_hash_entry *eh;
  struct elf_archc_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;
  htab = elf_archc_hash_table(info);

  if (htab->root.dynamic_sections_created
      && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
      && (h->forced_local)==0)	  
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (info->shared
	  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  asection *s = htab->splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->rawsize == 0)
	    s->rawsize += PLT_HEADER_SIZE;

	  h->plt.offset = s->rawsize;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! info->shared
	      && (h->def_regular) == 0)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  s->rawsize += PLT_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->sgotplt->rawsize += 4;

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->srelplt->rawsize += sizeof (Elf32_External_Rela);
	}
      else
	{
	  h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
    }

  if (h->got.refcount > 0)
    {
      asection *s;
      bfd_boolean dyn;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
      && (h->forced_local) == 0 )
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      s = htab->sgot;
      h->got.offset = s->rawsize;
      s->rawsize += 4;
      dyn = htab->root.dynamic_sections_created;
      if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	   || h->root.type != bfd_link_hash_undefweak)
	  && (info->shared
	      || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	htab->srelgot->rawsize += sizeof (Elf32_External_Rela);
    }
  else
    h->got.offset = (bfd_vma) -1;

  eh = (struct elf_archc_link_hash_entry *) h;
  if (eh->relocs_copied == NULL)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (info->shared)
    {
      /* Discard relocs on undefined weak syms with non-default
         visibility.  */
      if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	  && h->root.type == bfd_link_hash_undefweak)
	eh->relocs_copied = NULL;
    }
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if ((h->non_got_ref) == 0
	  && (((h->def_dynamic) != 0
	       && (h->def_regular) == 0)
	      || (htab->root.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined))))
      {
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1
	      && (h->forced_local) == 0)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return FALSE;
	    }

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      eh->relocs_copied = NULL;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (p = eh->relocs_copied; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->section)->sreloc;
      sreloc->rawsize += p->count * sizeof (Elf32_External_Rela);
    }

  return TRUE;
}


/* Set the sizes of the dynamic sections. The ELF linker
   may already set sizes of other sections, here we
   set the sizes of the remaining dynamic sections.  */
static bfd_boolean
elf_archc_size_dynamic_sections (output_bfd, info)
     bfd * output_bfd ATTRIBUTE_UNUSED;
     struct bfd_link_info * info;
{
  bfd * dynobj;
  asection * s;
  bfd_boolean plt;
  bfd_boolean relocs;
  bfd *ibfd;
  struct elf_archc_link_hash_table *htab;

  htab = elf_archc_hash_table (info);
  dynobj = elf_hash_table (info)->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
	{
	  s = bfd_get_section_by_name (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->rawsize = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      char *local_tls_type;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      
      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_archc_dyn_relocs *p;

	  for (p = *((struct elf_archc_dyn_relocs **)
		     &elf_section_data (s)->local_dynrel);
	       p != NULL;
	       p = p->next)
	    {
	      if (!bfd_is_abs_section (p->section)
		  && bfd_is_abs_section (p->section->output_section))
		  {
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */ 
		}
	      else if (p->count != 0)
		{
		  srel = elf_section_data (p->section)->sreloc;
		  srel->rawsize += p->count * sizeof (Elf32_External_Rela);
		  if ((p->section->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}
      
      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      s = htab->sgot;
      srel = htab->srelgot;
      for (; local_got < end_local_got; ++local_got, ++local_tls_type)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->rawsize;
	      s->rawsize += 4;
	      if (info->shared)
		srel->rawsize += sizeof (Elf32_External_Rela);
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, allocate_dynrelocs, (PTR) info);

  /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  plt = FALSE;
  relocs = FALSE;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char * name;
      bfd_boolean strip;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      /* It's OK to base decisions on the section name, because none
	 of the dynobj section names depend upon the input files.  */
      name = bfd_get_section_name (dynobj, s);

      strip = FALSE;

      if (strcmp (name, ".plt") == 0)
	{
	  if (s->rawsize == 0)
	    {
	      /* Strip this section if we don't need it; see the
                 comment below.  */
	      strip = TRUE;
	    }
	  else
	    {
	      /* Remember whether there is a PLT.  */
	      plt = TRUE;
	    }
	}
      else if (strncmp (name, ".rela", 5) == 0)
	{
	  if (s->rawsize == 0)
	    {
	      /* If we don't need this section, strip it from the
		 output file.  This is mostly to handle .rel.bss and
		 .rela.plt.  We must create both sections in
		 create_dynamic_sections, because they must be created
		 before the linker maps input sections to output
		 sections.  The linker does that before
		 adjust_dynamic_symbol is called, and it is that
		 function which decides whether anything needs to go
		 into these sections.  */
	      strip = TRUE;
	    }
	  else
	    {
	      /* Remember whether there are any reloc sections other
                 than .rela.plt.  */
	      if (strcmp (name, ".rela.plt") != 0)
		relocs = TRUE;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0; 
	    }
	}
      else if (strncmp (name, ".got", 4) != 0)
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

    if (strip)
	{
	  //_bfd_strip_section_from_output (info, s); // FIXME
      s->flags |= SEC_EXCLUDE;
	  continue;
	}

      /* Allocate memory for the section contents.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->rawsize);
      if (s->contents == NULL && s->rawsize != 0)
	return FALSE;
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in elf_archc_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (!info->shared)
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return FALSE;
	}

      if (plt)
	{
	  if (   !add_dynamic_entry (DT_PLTGOT, 0)
	      || !add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return FALSE;
	}

      if (relocs)
	{
	  if (   !add_dynamic_entry (DT_RELA, 0)
	      || !add_dynamic_entry (DT_RELASZ, 0)
	      || !add_dynamic_entry (DT_RELENT, sizeof (Elf32_External_Rela)))
	    return FALSE;
	}

      if ((info->flags & DF_TEXTREL) != 0)
	{
	  if (!add_dynamic_entry (DT_TEXTREL, 0))
	    return FALSE;
	  info->flags |= DF_TEXTREL;
	}
    }
#undef add_dynamic_entry

  return TRUE;
}



/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
elf_archc_finish_dynamic_symbol (output_bfd, info, h, sym)
     bfd * output_bfd;
     struct bfd_link_info * info;
     struct elf_link_hash_entry * h;
     Elf_Internal_Sym * sym;
{
  bfd * dynobj;

  dynobj = elf_hash_table (info)->dynobj;

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection * splt;
      asection * sgot;
      asection * srel;
      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rel;
      bfd_byte *loc;
      bfd_vma got_displacement;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */

      BFD_ASSERT (h->dynindx != -1);

      splt = bfd_get_section_by_name (dynobj, ".plt");
      sgot = bfd_get_section_by_name (dynobj, ".got.plt");
      srel = bfd_get_section_by_name (dynobj, ".rela.plt");
      BFD_ASSERT (splt != NULL && sgot != NULL && srel != NULL);

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = (h->plt.offset - PLT_HEADER_SIZE) / PLT_ENTRY_SIZE;

      /* Get the offset into the .got table of the entry that
	 corresponds to this function.  Each .got entry is 4 bytes.
	 The first three are reserved.  */
      got_offset = (plt_index + 3) * 4;

      /* Calculate the displacement between the PLT slot and the
	 entry in the GOT.  */
      got_displacement = (sgot->output_section->vma
			  + sgot->output_offset
			  + got_offset
			  - splt->output_section->vma
			  - splt->output_offset
			  - h->plt.offset);

      BFD_ASSERT ((got_displacement & 0xf0000000) == 0);

      /* Fill in the entry in the procedure linkage table.  */
      bfd_put_32 (output_bfd, elf_archc_plt_entry[0],
		  splt->contents + h->plt.offset + 0);
      bfd_put_32 (output_bfd, elf_archc_plt_entry[1],
		  splt->contents + h->plt.offset + 4);
      bfd_put_32 (output_bfd, elf_archc_plt_entry[2],
		  splt->contents + h->plt.offset + 8);
      bfd_put_32 (output_bfd, elf_archc_plt_entry[3],
		  splt->contents + h->plt.offset + 12);
      elf_archc_patch_plt_entry(got_displacement, h->plt.offset, (char *) splt->contents);


      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
		  (splt->output_section->vma
		   + splt->output_offset),
		  sgot->contents + got_offset);
      
      /* Fill in the entry in the .rela.plt section.  */
      rel.r_offset = (sgot->output_section->vma
		      + sgot->output_offset
		      + got_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, `R_'___arch_name___`_JUMP_SLOT);'
      rel.r_addend = 0;
      loc = srel->contents + plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);

      if ((h->def_regular) == 0)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	  /* If the symbol is weak, we do need to clear the value.
	     Otherwise, the PLT entry would provide a definition for
	     the symbol even if the symbol wasn't defined anywhere,
	     and so the symbol would never be NULL.  */
	  if ((h->ref_regular_nonweak)
	      == 0)
	    sym->st_value = 0;
	}
    }

  if (h->got.offset != (bfd_vma) -1)
    {
      asection * sgot;
      asection * srel;
      Elf_Internal_Rela rel;
      bfd_byte *loc;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */
      sgot = bfd_get_section_by_name (dynobj, ".got");
      srel = bfd_get_section_by_name (dynobj, ".rela.got");
      BFD_ASSERT (sgot != NULL && srel != NULL);

      rel.r_offset = (sgot->output_section->vma
		      + sgot->output_offset
		      + (h->got.offset &~ (bfd_vma) 1));
      rel.r_addend = 0;

      /* If this is a static link, or it is a -Bsymbolic link and the
	 symbol is defined locally or was forced to be local because
	 of a version file, we just want to emit a RELATIVE reloc.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (info->shared
	  && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  BFD_ASSERT((h->got.offset & 1) != 0);
	  rel.r_info = ELF32_R_INFO (0, `R_'___arch_name___`_RELATIVE);' 
	}
      else
	{
	  BFD_ASSERT((h->got.offset & 1) == 0);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + h->got.offset);
	  rel.r_info = ELF32_R_INFO (h->dynindx, `R_'___arch_name___`_GLOB_DAT);'
	}

      loc = srel->contents + srel->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
    }

  if ((h->needs_copy) != 0)
    {
      asection * s;
      Elf_Internal_Rela rel;
      bfd_byte *loc;

      /* This symbol needs a copy reloc.  Set it up. */
      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      s = bfd_get_section_by_name (h->root.u.def.section->owner,
				   ".rela.bss");
      BFD_ASSERT (s != NULL);

      rel.r_offset = (h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, `R_'___arch_name___`_COPY);'
      rel.r_addend = 0;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

/* Finish up the dynamic sections.  */

static bfd_boolean
elf_archc_finish_dynamic_sections (output_bfd, info)
     bfd * output_bfd;
     struct bfd_link_info * info;
{
  bfd * dynobj;
  asection * sgot;
  asection * sdyn;

  dynobj = elf_hash_table (info)->dynobj;

  sgot = bfd_get_section_by_name (dynobj, ".got.plt");
  BFD_ASSERT (sgot != NULL);
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      splt = bfd_get_section_by_name (dynobj, ".plt");
      BFD_ASSERT (splt != NULL && sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->rawsize);

      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  const char * name;
	  asection * s;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      break;

	    case DT_PLTGOT:
	      name = ".got";
	      goto get_vma;
	    case DT_JMPREL:
	      name = ".rela.plt";
	    get_vma:
	      s = bfd_get_section_by_name (output_bfd, name);
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_ptr = s->vma;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = bfd_get_section_by_name (output_bfd, ".rela.plt");
	      BFD_ASSERT (s != NULL);
	      if (s->size != 0)
		dyn.d_un.d_val = s->size;
	      else
		dyn.d_un.d_val = s->rawsize;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_RELASZ:
	      /* My reading of the SVR4 ABI indicates that the
		 procedure linkage table relocs (DT_JMPREL) should be
		 included in the overall relocs (DT_RELA).  This is
		 what Solaris does.  However, UnixWare can not handle
		 that case.  Therefore, we override the DT_RELSZ entry
		 here to make it not include the JMPREL relocs.  Since
		 the linker script arranges for .rel.plt to follow all
		 other relocation sections, we don't have to worry
		 about changing the DT_RELA entry.  */
	      s = bfd_get_section_by_name (output_bfd, ".rela.plt");
	      if (s != NULL)
		{
		  if (s->size != 0)
		    dyn.d_un.d_val -= s->size;
		  else
		    dyn.d_un.d_val -= s->rawsize;
		} 
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon); 
	      break;

	    }
	}

      /* Fill in the first entry in the procedure linkage table.  */
      if (splt->rawsize > 0)
	{
	  bfd_vma got_displacement;

	  /* Calculate the displacement between the PLT slot and &GOT[0].  */
	  got_displacement = (sgot->output_section->vma
			      + sgot->output_offset
			      - splt->output_section->vma
			      - splt->output_offset);

	  bfd_put_32 (output_bfd, elf_archc_plt0_entry[0], splt->contents +  0);
	  bfd_put_32 (output_bfd, elf_archc_plt0_entry[1], splt->contents +  4);
	  bfd_put_32 (output_bfd, elf_archc_plt0_entry[2], splt->contents +  8);
	  bfd_put_32 (output_bfd, elf_archc_plt0_entry[3], splt->contents + 12);

	  elf_archc_patch_plt0_entry(got_displacement, (char*)splt->contents);	  
	}

      /* UnixWare sets the entsize of .plt to 4, although that doesn't
	 really seem like the right value.  */
      elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;
    }

  /* Fill in the first three entries in the global offset table.  */
  if (sgot->rawsize > 0)
    {
      if (sdyn == NULL)
	bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents);
      else
	bfd_put_32 (output_bfd,
		    sdyn->output_section->vma + sdyn->output_offset,
		    sgot->contents);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 4);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + 8);
    }

  elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;

  return TRUE;
}

static enum elf_reloc_type_class
elf_archc_reloc_type_class (rela)
     const Elf_Internal_Rela *rela;
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case `R_'___arch_name___`_RELATIVE:'
      return reloc_class_relative;
    case `R_'___arch_name___`_JUMP_SLOT:'
      return reloc_class_plt;
    case `R_'___arch_name___`_COPY:'
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Should return any additional program headers
   required by the backend.*/
int elf_archc_additional_program_headers (bfd *abfd)
{
  asection *s;
  int ret = 0;

  /* See if we need a .gnu.version segment.  */
  s = bfd_get_section_by_name (abfd, ".gnu.version");
  if (s && (s->flags & SEC_LOAD))
    ret += 2;

  /* Prevents lack of program headers allocation space
     in case extra segments are created because of alignment
     and max-page-size issues.
     FIXME: It must be a better way to do this instead of
     hacking additional_program_headers backend function. */
  ret += 10;

  return ret;
}

/***
  Relocation functions used by assembler when producing a relocatable
  ELF 
 ***/

static bfd_reloc_status_type
generic_data_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd ATTRIBUTE_UNUSED,
             char **error_message ATTRIBUTE_UNUSED)
{
  unsigned int relocation;
  asection *reloc_target_output_section;
  bfd_size_type octets = reloc_entry->address * bfd_octets_per_byte (abfd); 
  reloc_howto_type *howto = reloc_entry->howto;


  reloc_target_output_section = symbol->section->output_section;

  relocation = symbol->value;
  relocation += reloc_target_output_section->vma + symbol->section->output_offset;
  relocation += reloc_entry->addend;

  if (howto->pc_relative)
    relocation -= input_section->output_section->vma + input_section->output_offset;


  putbits(howto->bitsize, (char *) data + octets, relocation, ___endian_val___); 


  return bfd_reloc_ok;
}


static bfd_reloc_status_type
bfd_elf_archc_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED)
{
  unsigned int relocation;
  bfd_size_type octets = reloc_entry->address * bfd_octets_per_byte (abfd); 
  bfd_vma output_base = 0;
  reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;
  mod_parms modifier_parms;
      
   
  /*
   * Taken from bfd_elf_generic_reloc
   * It is basically a short circuit if the output is relocatable
   */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! howto->partial_inplace
      || reloc_entry->addend == 0)) {
    reloc_entry->address += input_section->output_offset;
    return bfd_reloc_ok;
  }
  
  /*
   * This part is mainly copied from bfd_perform_relocation, but
   * here we use our own specific method to apply th relocation
   * Fields of HOWTO structure and their specific mappings
   *   type          --> id
   *   rightshift    --> operand_id
   */

  /* Is the address of the relocation really within the section?  */
  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  /* Work out which section the relocation is targeted at and the
   * initial relocation command value.  */

  /* Get symbol value.  (Common symbols are special.)  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  reloc_target_output_section = symbol->section->output_section;

  /* Convert input-section-relative symbol value to absolute.  */
  if ((output_bfd && ! howto->partial_inplace)
      || reloc_target_output_section == NULL)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

   
  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
   * symbol we are relocating against, plus any addend. 
   * relocation => input variable in our scheme 
   */
  unsigned int address = (unsigned int) (input_section->output_section->vma + input_section->output_offset + reloc_entry->address);


  if (output_bfd != NULL) {
    
  }
  else 
    reloc_entry->addend = 0;
     
 /*
  * Overflow checking is NOT being done!!! 
  */

  unsigned int insn_image = (unsigned int) getbits(get_insn_size(operands[howto->rightshift].format_id), (char *) data + octets, ___endian_val___);


  modifier_parms.input   = relocation;
  modifier_parms.address = address;
  modifier_parms.section = symbol->section->name;
  modifier_parms.list_results = NULL;

  encode_cons_field(&insn_image, &modifier_parms, howto->rightshift);

  if (modifier_parms.error) {
    _bfd_error_handler (_("Invalid relocation operation"));
  }

  putbits(get_insn_size(operands[howto->rightshift].format_id), (char *) data + octets, insn_image, ___endian_val___); 

  
  return bfd_reloc_ok;
}


/* Called by the backend linker after all the linker input files have
   been seen but before the section sizes have been set. This is called
   after adjust_dynamic_symbol, but before size_dynamic_sections */
static bfd_boolean elf_archc_always_size_sections (bfd * output_bfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  /*
  asection *sec;
  
  sec = bfd_make_section (output_bfd, ".note.archc");
  if (sec == NULL
      || ! bfd_set_section_flags (output_bfd, sec, SEC_NEVER_LOAD | 
				  SEC_READONLY | SEC_LINKER_CREATED)
      )
    return FALSE;
  */

  return TRUE;
}


#define `TARGET_'___endian_str___`_SYM'   ___arch_name___`_elf32_be_vec'
#define `TARGET_'___endian_str___`_NAME'  `"elf32-'___arch_name___`"'
#define ELF_ARCH                          `bfd_arch_'___arch_name___
#define ELF_MACHINE_CODE                  EM_NONE
#define ELF_MAXPAGESIZE                   0x1
#define bfd_elf32_bfd_link_hash_table_create    elf_archc_link_hash_table_create
#define bfd_elf32_bfd_reloc_type_lookup   ___arch_name___`_reloc_type_lookup'
#define bfd_elf32_bfd_is_local_label_name ___arch_name___`_elf_is_local_label_name'
#define elf_info_to_howto                 ___arch_name___`_elf_info_to_howto'

#define elf_backend_can_refcount    1
#define elf_backend_can_gc_sections 1
#define elf_backend_plt_readonly    1
#define elf_backend_want_got_plt    1
#define elf_backend_want_plt_sym    0
#define elf_backend_rela_normal     1

#define elf_backend_got_header_size	12

#define elf_backend_gc_mark_hook            elf_archc_gc_mark_hook
#define elf_backend_gc_sweep_hook           elf_archc_gc_sweep_hook
#define elf_backend_relocate_section	    elf_archc_relocate_section
#define elf_backend_create_dynamic_sections elf_archc_create_dynamic_sections
#define elf_backend_finish_dynamic_symbol   elf_archc_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections elf_archc_finish_dynamic_sections
#define elf_backend_size_dynamic_sections   elf_archc_size_dynamic_sections
#define elf_backend_adjust_dynamic_symbol   elf_archc_adjust_dynamic_symbol
#define elf_backend_check_relocs            elf_archc_check_relocs
#define elf_backend_always_size_sections    elf_archc_always_size_sections
#define elf_backend_reloc_type_class	    elf_archc_reloc_type_class
#define elf_backend_copy_indirect_symbol    elf_archc_copy_indirect_symbol
#define elf_backend_additional_program_headers elf_archc_additional_program_headers

#include "elf32-target.h"


