/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_rtld_config.cpp
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
 *            information. (implementation)
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

/* Note: this file contains a small parser to read the relocation map,
   and mantains the translation table as hashtable. These are implementation
   details hidden from other classes. */

#include "ac_rtld_config.H"
#include "dynamic_info.H"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/* Useful defines */

#define FINE 0
#define ERROR -1

#define HASHTABLE_SIZE 20


namespace ac_dynlink {

  /*** Public methods ***/

  ac_rtld_config::ac_rtld_config() {
    int mapfile_fd;

    config_loaded = false;
    hashtable = NULL;

    /* Our config file exists? */
    mapfile_fd = find_config_file ("ac_rtld.relmap");
    
    if (mapfile_fd >= 0) {      
    
      if (hash_initialize(&hashtable) == FINE) {

	if (process_map_file(mapfile_fd, hashtable) == FINE) {
	  config_loaded = true;
	} else {
	  hash_delete(&hashtable);
	}
      }

      close(mapfile_fd);
    }
  }
  
  ac_rtld_config::~ac_rtld_config() {
    hash_delete (&hashtable);
  }
  bool ac_rtld_config::is_config_loaded() {
    return config_loaded;
  }

  int ac_rtld_config::translate(unsigned code, unsigned *result) {
    if (config_loaded) {
      if(hash_get_value (hashtable, code, result)==FINE)
	return 0;
    }

    return -1;
  }

  /*** Private methods ***/

  /* Find relocation map location and open it. Return a descriptor 
     to the open file.*/
  int ac_rtld_config::find_config_file (const char *filename) 
  {
    int fd;
    unsigned int i, j, k;
    char *envpath, *apath;
    fd = open(filename,0);
    if (fd > 0)
      return fd;
    envpath = getenv(ENV_AC_LIBRARY_PATH);
    if (envpath == NULL)
      return -1;
    /* Process environment variable containing libraries search path,
     also used to search the relocation map. */
    apath = new char[strlen(envpath) + strlen(filename) + 1];
    for (i = 0, j = 0; i < strlen(envpath); i++)
      {
	apath[j++] = envpath[i]; 
	if (envpath[i+1] == ':' || envpath[i+1] == '\0')
	  {
	    i++;
	    apath[j++] = '/';
	    for (k=0; k < strlen(filename); k++)
	      {
		apath[j++] = filename[k];
	      }
	    apath[j] = '\0';
	    fd = open(apath, 0);
	    if (fd > 0)
	      {
		delete [] apath;
		return fd;
	      }
	    j = 0;
	  }           
      }
    delete [] apath;
    return -1;
  }


  int ac_rtld_config::request_read(unsigned int fd, char *buf, unsigned int quantity)
  {
    if(read(fd, buf, quantity)!= quantity)
      {
	buf[0] = '\0';
	return ERROR;
    }
    return FINE;
  }

  int ac_rtld_config::process_map_file(unsigned int fd, hash_node **hashtable)
  {
    unsigned int i, num1, num2;
    char buffer[20];
    unsigned int line = 0;

    buffer[0] = 'a';

    while (buffer[0] != '\0')
      {
	char* endptr;
	line++;

	request_read(fd, buffer, 1);

	/* Ignore line */
	if (buffer[0] == '#')
	  {
	    while (buffer[0] != '\n' && buffer[0] != '\0')
	      request_read(fd, buffer, 1);
	    continue;
	  }

	/* Skips whitespace */
	while (buffer[0] == ' ' || buffer[0] == '\t')
	  request_read(fd, buffer, 1);

	/* Nothing to parse here, go to next line */
	if (buffer[0] == '\n' || buffer[0] == '\0')
	  continue;

	/** Parse num = num statement **/
	/* Not a number */
	if (buffer[0] < '0' || buffer[0] > '9')
	  {
	    return ERROR;
	  }

	i = 0;

	while (buffer[i] >= '0' && buffer[i] <= '9')
	{ 
	  request_read(fd, &buffer[++i], 1);
	  if (i >= 20)
	    {
	      return ERROR;
	    }
	}
	
	/* Unexpected end of line */
	if (buffer[i] == '\n' || buffer[i] == '\0') 
	  {
	    return ERROR;
	  }

	endptr = NULL;

	num1 = strtol(buffer, &endptr, 10);
	if (endptr != &buffer[i])
	  {
	    return ERROR;
	  }
	buffer[0] = buffer[i];

	/* Skips whitespace */
	while (buffer[0] == ' ' || buffer[0] == '\t')
	  request_read(fd, buffer, 1);

	/* Unexpected token */
	if (buffer[0] != '=') 
	{
	  return ERROR;
	}

	request_read(fd, buffer, 1);
	
	/* Skips whitespace */
	while (buffer[0] == ' ' || buffer[0] == '\t')
	  request_read(fd, buffer, 1);

	/* Unexpected end of line */
	if (buffer[0] == '\n' || buffer[0] == '\0') 
	  {
	    return ERROR;
	  }

	/* Parse the second number at num = num statement */
	/* Not a number */
	if (buffer[0] < '0' || buffer[0] > '9')
	  { 
	    return ERROR;
	  }
	
	i = 0;

	while (buffer[i] >= '0' && buffer[i] <= '9')
	  { 
	    request_read(fd, &buffer[++i], 1);
	    if (i >= 20)
	      {
		return ERROR;
	      }
	  }
	
	endptr = NULL;

	num2 = strtol(buffer, &endptr, 10);
	if (endptr != &buffer[i])
	  {
	    return ERROR;
	  }
	buffer[0] = buffer[i];

	if (buffer[0] != '\n' && buffer[0] != '\0')
	  {
	    return ERROR;
	  }
       
	if (hash_add_value(hashtable, num1, num2) == ERROR)
	  {
	    return ERROR;
	  }
      

      }

    return FINE;
  }

  /*** Hash specific functions ***/
  int ac_rtld_config::hash_initialize (hash_node *** hashtable)
  {
    unsigned int ndx = 0;

    *hashtable = new (hash_node*[HASHTABLE_SIZE]);

    if (*hashtable == NULL)
      {
	return ERROR;
      }

    for (ndx = 0; ndx < HASHTABLE_SIZE; ndx++)
      {
	(*hashtable)[ndx] = NULL;
      }

    return FINE;
  }

  int ac_rtld_config::hash_add_value (hash_node **hashtable, unsigned int index, unsigned int value)
  {
    hash_node *newnode;

    newnode = new hash_node;

    if (newnode == NULL)
      {
	return ERROR;
      } 

    newnode->index = index;
    newnode->value = value;
    newnode->next = hashtable[index % HASHTABLE_SIZE];
    hashtable[index % HASHTABLE_SIZE] = newnode;

    return FINE;
  }

  void ac_rtld_config::hash_delete (hash_node ***hashtable)
  {
    unsigned int ndx;
    hash_node * p, * q;

    if (hashtable == NULL || *hashtable == NULL)
      return;
  
    for (ndx = 0; ndx < HASHTABLE_SIZE; ndx++)
      {
	if ((*hashtable)[ndx] != NULL)
	  {
	    p = ((*hashtable)[ndx])->next;
	    while (p != NULL)
	      {
		q = p->next;
		delete p;
		p = q;
	      }
	    delete (*hashtable)[ndx];
	  }
      }
    delete [] *hashtable;
    *hashtable = NULL;
  }

  int ac_rtld_config::hash_get_value (hash_node **hashtable, unsigned int index, unsigned int *value)
  {
    hash_node *p;

    p = hashtable[index % HASHTABLE_SIZE];
    if (p == NULL)
      return ERROR;
    while (p->index != index)
      {
	p = p->next;
	if (p == NULL)
	  return ERROR;
      }
    *value = p->value;
    return FINE;
  }
  
}
