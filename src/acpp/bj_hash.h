/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/

#ifndef _BJ_HASH_H_
#define _BJ_HASH_H_

/*
Documentation by the original author.
-------------------------------------------------------------------------------
lookup3.c, by Bob Jenkins, May 2006, Public Domain.

These are functions for producing 32-bit hashes for hash table lookup.
hashword(), hashlittle(), hashbig(), mix(), and final() are externally
useful functions.  Routines to test the hash are included if BJ_SELF_TEST
is defined.  You can use this free for any purpose.  It has no warranty.

You probably want to use hashlittle().  hashlittle() and hashbig()
hash byte arrays.  hashlittle() is is faster than hashbig() on
little-endian machines.  Intel and AMD are little-endian machines.

If you want to find a hash of, say, exactly 7 integers, do
  a = i1;  b = i2;  c = i3;
  mix(a,b,c);
  a += i4; b += i5; c += i6;
  mix(a,b,c);
  a += i7;
  final(a,b,c);
then use c as the hash value.  If you have a variable length array of
4-byte integers to hash, use hashword().  If you have a byte array (like
a character string), use hashlittle().  If you have several byte arrays, or
a mix of things, see the comments above hashlittle().
-------------------------------------------------------------------------------
*/

#ifdef BJ_SELF_TEST
#include <stdio.h>
#endif
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#ifdef BJ_SELF_TEST
#include <time.h>
#endif
#include <sys/types.h>
#include <sys/param.h>

uint32_t hashword(uint32_t* k, size_t length, uint32_t initval);
uint32_t hashlittle(void* key, size_t length, uint32_t initval);
uint32_t hashbig(void* key, size_t length, uint32_t initval);
uint32_t bj_hash(void* key, size_t length, uint32_t initval);

#ifdef BJ_SELF_TEST
void driver1(void);
void driver2(void);
void driver3(void);
void driver4(void);
void bj_hash_test(void);
#endif

#endif
