/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/

#include "bj_hash.h"

#define hashsize(n) ((uint32_t) 1 << (n))
#define hashmask(n) (hashsize(n) - 1)
#define rot(x, k) (((x) << (k)) ^ ((x) >> (32 - (k))))

/*
Documentation by the original author.
-------------------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.

This is reversible, so any information in (a,b,c) before mix() is
still in (a,b,c) after mix().

If four pairs of (a,b,c) inputs are run through mix(), or through
mix() in reverse, there are at least 32 bits of the output that
are sometimes the same for one pair and different for another pair.
This was tested for:
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or
  all zero plus a counter that starts at zero.

Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
satisfy this are
    4  6  8 16 19  4
    9 15  3 18 27 15
   14  9  3  7 17  3
Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
for "differ" defined as + with a one-bit base and a two-bit delta.  I
used http://burtleburtle.net/bob/hash/avalanche.html to choose
the operations, constants, and arrangements of the variables.

This does not achieve avalanche.  There are input bits of (a,b,c)
that fail to affect some output bits of (a,b,c), especially of a.  The
most thoroughly mixed value is c, but it doesn't really even achieve
avalanche in c.

This allows some parallelism.  Read-after-writes are good at doubling
the number of bits affected, so the goal of mixing pulls in the opposite
direction as the goal of parallelism.  I did what I could.  Rotates
seem to cost as much as shifts on every machine I could lay my hands
on, and rotates are much kinder to the top and bottom bits, so I used
rotates.
-------------------------------------------------------------------------------
*/

#define mix(a, b, c) \
{ \
 a -= c; a ^= rot(c, 4); c += b; \
 b -= a; b ^= rot(a, 6); a += c; \
 c -= b; c ^= rot(b, 8); b += a; \
 a -= c; a ^= rot(c, 16); c += b; \
 b -= a; b ^= rot(a, 19); a += c; \
 c -= b; c ^= rot(b, 4); b += a; \
}

/*
Documentation by the original author.
-------------------------------------------------------------------------------
final -- final mixing of 3 32-bit values (a,b,c) into c

Pairs of (a,b,c) values differing in only a few bits will usually
produce values of c that look totally different.  This was tested for
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or
  all zero plus a counter that starts at zero.

These constants passed:
 14 11 25 16 4 14 24
 12 14 25 16 4 14 24
and these came close:
  4  8 15 26 3 22 24
 10  8 15 26 3 22 24
 11  8 15 26 3 22 24
-------------------------------------------------------------------------------
*/

#define final(a, b, c) \
{ \
 c ^= b; c -= rot(b, 14); \
 a ^= c; a -= rot(c, 11); \
 b ^= a; b -= rot(a, 25); \
 c ^= b; c -= rot(b, 16); \
 a ^= c; a -= rot(c, 4); \
 b ^= a; b -= rot(a, 14); \
 c ^= b; c -= rot(b, 24); \
}

/** Union for runtime endianness checking. */
typedef union
{
 int i;
 char c[sizeof(int) / sizeof(char)];
} endian;

/*
Documentation by the original author.
--------------------------------------------------------------------
 This works on all machines.  To be useful, it requires
 -- that the key be an array of uint32_t's, and
 -- that all your machines have the same endianness, and
 -- that the length be the number of uint32_t's in the key

 The function hashword() is identical to hashlittle() on little-endian
 machines, and identical to hashbig() on big-endian machines,
 except that the length has to be measured in uint32_ts rather than in
 bytes.  hashlittle() is more complicated than hashword() only because
 hashlittle() has to dance around fitting the key bytes into registers.
--------------------------------------------------------------------
*/
/**
 \param k the key, an array of uint32_t values
 \param length the length of the key, in uint32_ts
 \param initval the previous hash, or an arbitrary value
*/
uint32_t hashword(uint32_t* k, size_t length, uint32_t initval)
{
 uint32_t a, b, c;

 /* Set up the internal state */
 a = b = c = 0xdeadbeef + (((uint32_t) length) << 2) + initval;
 /*------------------------------------------------- handle most of the key */
 while (length > 3)
 {
  a += k[0];
  b += k[1];
  c += k[2];
  mix(a, b, c);
  length -= 3;
  k += 3;
 }
 /*------------------------------------------- handle the last 3 uint32_t's */
 switch (length) /* all the case statements fall through */
 {
  case 3:
   c += k[2];
  case 2:
   b += k[1];
  case 1:
   a += k[0];
   final(a, b, c);
  case 0: /* case 0: nothing left to add */
   break;
 }
 /*------------------------------------------------------ report the result */
 return c;
}

/*
Documentation by the original author.
-------------------------------------------------------------------------------
hashlittle() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  length  : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Two keys differing by one or two bits will have
totally different hash values.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (uint8_t **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hashlittle( k[i], len[i], h);

By Bob Jenkins, 2006.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
-------------------------------------------------------------------------------
*/
uint32_t hashlittle(void* key, size_t length, uint32_t initval)
{
 uint32_t a, b, c;
 endian testchar, testint;
 unsigned hash_little_endian;

 testint.i = 255;
 testchar.i = 0;
 testchar.c[(sizeof(int) / sizeof(char)) - 1] = 255;
 if (testchar.i == testint.i)
  hash_little_endian = 0; /* Host machine is big endian. */
 else
  hash_little_endian = 1; /* Host machine is little endian. */
 /* Set up the internal state */
 a = b = c = 0xdeadbeef + ((uint32_t) length) + initval;
 if (hash_little_endian && !((((uint8_t*) key) - (uint8_t*) 0) & 0x3))
 {
  uint32_t* k = key; /* read 32-bit chunks */

  /*------ all but last block: aligned reads and affect 32 bits of (a, b, c) */
  while (length > 12)
  {
   a += k[0];
   b += k[1];
   c += k[2];
   mix(a, b, c);
   length -= 12;
   k += 3;
  }
  /*----------------------------- handle the last (probably partial) block */
  switch (length)
  {
   case 12:
    c += k[2];
    b += k[1];
    a += k[0];
    break;
   case 11:
    c += k[2] & 0xffffff;
    b += k[1];
    a += k[0];
    break;
   case 10:
    c += k[2] & 0xffff;
    b += k[1];
    a += k[0];
    break;
   case 9:
    c += k[2] & 0xff;
    b += k[1];
    a += k[0];
    break;
   case 8:
    b += k[1];
    a += k[0];
    break;
   case 7:
    b += k[1] & 0xffffff;
    a += k[0];
    break;
   case 6:
    b += k[1] & 0xffff;
    a += k[0];
    break;
   case 5:
    b += k[1] & 0xff;
    a += k[0];
    break;
   case 4:
    a += k[0];
    break;
   case 3:
    a += k[0] & 0xffffff;
    break;
   case 2:
    a += k[0] & 0xffff;
    break;
   case 1:
    a += k[0] & 0xff;
    break;
   case 0:
    return c; /* zero length strings require no mixing */
  }
 }
 else if (hash_little_endian && !((((uint8_t* ) key) - (uint8_t*) 0) & 0x1))
 {
  uint16_t* k = key; /* read 16-bit chunks */

  /*--------------- all but last block: aligned reads and different mixing */
  while (length > 12)
  {
   a += k[0] + (((uint32_t) k[1]) << 16);
   b += k[2] + (((uint32_t) k[3]) << 16);
   c += k[4] + (((uint32_t) k[5]) << 16);
   mix(a, b, c);
   length -= 12;
   k += 6;
  }
  /*----------------------------- handle the last (probably partial) block */
  switch(length)
  {
   case 12:
    c += k[4] + (((uint32_t) k[5]) << 16);
    b += k[2] + (((uint32_t) k[3]) << 16);
    a += k[0] + (((uint32_t) k[1]) << 16);
    break;
   case 11:
    c += ((uint32_t) (k[5] & 0xff)) << 16; /* fall through */
   case 10:
    c += k[4];
    b += k[2] + (((uint32_t) k[3]) << 16);
    a += k[0] + (((uint32_t) k[1]) << 16);
    break;
   case 9:
    c += k[4] & 0xff; /* fall through */
   case 8:
    b += k[2] + (((uint32_t) k[3]) << 16);
    a += k[0] + (((uint32_t) k[1]) << 16);
    break;
   case 7:
    b += ((uint32_t) (k[3] & 0xff)) << 16; /* fall through */
   case 6:
    b += k[2];
    a += k[0] + (((uint32_t) k[1]) << 16);
    break;
   case 5:
    b += k[2] & 0xff; /* fall through */
   case 4:
    a += k[0] + (((uint32_t) k[1]) << 16);
    break;
   case 3:
    a += ((uint32_t) (k[1] & 0xff)) << 16; /* fall through */
   case 2:
    a += k[0];
    break;
   case 1:
    a += k[0] & 0xff;
    break;
   case 0:
    return c; /* zero length requires no mixing */
  }
 }
 else
 { /* need to read the key one byte at a time */
  uint8_t *k = key;

  /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
  while (length > 12)
  {
   a += k[0];
   a += ((uint32_t) k[1]) << 8;
   a += ((uint32_t) k[2]) << 16;
   a += ((uint32_t) k[3]) << 24;
   b += k[4];
   b += ((uint32_t) k[5]) << 8;
   b += ((uint32_t) k[6]) << 16;
   b += ((uint32_t) k[7]) << 24;
   c += k[8];
   c += ((uint32_t) k[9]) << 8;
   c += ((uint32_t) k[10]) << 16;
   c += ((uint32_t) k[11]) << 24;
   mix(a, b, c);
   length -= 12;
   k += 12;
  }
  /*-------------------------------- last block: affect all 32 bits of (c) */
  switch (length) /* all the case statements fall through */
  {
   case 12:
    c += ((uint32_t) k[11]) << 24;
   case 11:
    c += ((uint32_t) k[10]) << 16;
   case 10:
    c += ((uint32_t) k[9]) << 8;
   case 9:
    c += k[8];
   case 8:
    b += ((uint32_t) k[7]) << 24;
   case 7:
    b += ((uint32_t) k[6]) << 16;
   case 6:
    b += ((uint32_t) k[5]) << 8;
   case 5:
    b += k[4];
   case 4:
    a += ((uint32_t) k[3]) << 24;
   case 3:
    a += ((uint32_t) k[2]) << 16;
   case 2:
    a += ((uint32_t) k[1]) << 8;
   case 1:
    a += k[0];
    break;
   case 0:
    return c;
  }
 }
 final(a, b, c);
 return c;
}

/*
 * Documentation by the original author.
 * --------------------------------------------------------------------
 * hashbig():
 * This is the same as hashword() on big-endian machines.  It is different
 * from hashlittle() on all machines.  hashbig() takes advantage of
 * big-endian byte ordering.
 */
uint32_t hashbig(void* key, size_t length, uint32_t initval)
{
 uint32_t a, b, c;
 endian testchar, testint;
 unsigned hash_big_endian;

 testint.i = 255;
 testchar.i = 0;
 testchar.c[(sizeof(int) / sizeof(char)) - 1] = 255;
 if (testchar.i == testint.i)
  hash_big_endian = 1; /* Host machine is big endian. */
 else
  hash_big_endian = 0; /* Host machine is little endian. */
 /* Set up the internal state */
 a = b = c = 0xdeadbeef + ((uint32_t) length) + initval;
 if (hash_big_endian && !((((uint8_t*) key) - (uint8_t*) 0) & 0x3))
 {
  uint32_t* k = key; /* read 32-bit chunks */

  /*------ all but last block: aligned reads and affect 32 bits of (a, b, c) */
  while (length > 12)
  {
   a += k[0];
   b += k[1];
   c += k[2];
   mix(a, b, c);
   length -= 12;
   k += 3;
  }
  /*----------------------------- handle the last (probably partial) block */
  switch (length)
  {
   case 12:
    c += k[2];
    b += k[1];
    a += k[0];
    break;
   case 11:
    c += k[2] << 8;
    b += k[1];
    a += k[0];
    break;
   case 10:
    c += k[2] << 16;
    b += k[1];
    a += k[0];
    break;
   case 9:
    c += k[2] << 24;
    b += k[1];
    a += k[0];
    break;
   case 8:
    b += k[1];
    a += k[0];
    break;
   case 7:
    b += k[1] << 8;
    a += k[0];
    break;
   case 6:
    b += k[1] << 16;
    a += k[0];
    break;
   case 5:
    b += k[1] << 24;
    a += k[0];
    break;
   case 4:
    a += k[0];
    break;
   case 3:
    a += k[0] << 8;
    break;
   case 2:
    a += k[0] << 16;
    break;
   case 1:
    a += k[0] << 24;
    break;
   case 0:
    return c; /* zero length strings require no mixing */
  }
 }
 else
 {  /* need to read the key one byte at a time */
  uint8_t* k = key;

  /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
  while (length > 12)
  {
   a += ((uint32_t) k[0]) << 24;
   a += ((uint32_t) k[1]) << 16;
   a += ((uint32_t) k[2]) << 8;
   a += ((uint32_t) k[3]);
   b += ((uint32_t) k[4]) << 24;
   b += ((uint32_t) k[5]) << 16;
   b += ((uint32_t) k[6]) << 8;
   b += ((uint32_t) k[7]);
   c += ((uint32_t) k[8]) << 24;
   c += ((uint32_t) k[9]) << 16;
   c += ((uint32_t) k[10]) << 8;
   c += ((uint32_t) k[11]);
   mix(a, b, c);
   length -= 12;
   k += 12;
  }
  /*-------------------------------- last block: affect all 32 bits of (c) */
  switch (length) /* all the case statements fall through */
  {
   case 12:
    c += ((uint32_t) k[11]) << 24;
   case 11:
    c += ((uint32_t) k[10]) << 16;
   case 10:
    c += ((uint32_t) k[9]) << 8;
   case 9:
    c += k[8];
   case 8:
    b += ((uint32_t) k[7]) << 24;
   case 7:
    b += ((uint32_t) k[6]) << 16;
   case 6:
    b += ((uint32_t) k[5]) << 8;
   case 5:
    b += k[4];
   case 4:
    a += ((uint32_t) k[3]) << 24;
   case 3:
    a += ((uint32_t) k[2]) << 16;
   case 2:
    a += ((uint32_t) k[1]) << 8;
   case 1:
    a+=k[0];
    break;
   case 0:
    return c;
  }
 }
 final(a, b, c);
 return c;
}

uint32_t bj_hash(void* key, size_t length, uint32_t initval)
{
 endian testchar, testint;

 testint.i = 255;
 testchar.i = 0;
 testchar.c[(sizeof(int) / sizeof(char)) - 1] = 255;
 if (testchar.i == testint.i)
  return hashbig(key, length, initval); /* Host machine is big endian. */
 else
  return hashlittle(key, length, initval); /* Host machine is little endian. */
}

#ifdef BJ_SELF_TEST

/* used for timings */
void driver1(void)
{
 uint8_t buf[256];
 uint32_t i;
 uint32_t h = 0;
 time_t a, z;

 time(&a);
 for (i = 0; i < 256; ++i)
  buf[i] = 'x';
 for (i = 0; i < 1; ++i)
 {
  h = hashlittle(&buf[0], 1, h);
 }
 time(&z);
 if ((z - a) > 0)
  fprintf(stderr, "time %ld %.8x\n", (z - a), h);
 return;
}

/* check that every input bit changes every output bit half the time */
#define HASHSTATE 1
#define HASHLEN 1
#define MAXPAIR 60
#define MAXLEN 70

void driver2(void)
{
 uint8_t qa[MAXLEN + 1], qb[MAXLEN + 2];
 uint8_t* a = &qa[0];
 uint8_t* b = &qb[1];
 uint32_t c[HASHSTATE], d[HASHSTATE], i = 0, j = 0, k, l, m = 0, z;
 uint32_t e[HASHSTATE], f[HASHSTATE], g[HASHSTATE], h[HASHSTATE];
 uint32_t x[HASHSTATE], y[HASHSTATE];
 uint32_t hlen;

 fprintf(stderr, "No more than %d trials should ever be needed \n", (MAXPAIR / 2));
 for (hlen = 0; hlen < MAXLEN; ++hlen)
 {
  z = 0;
  for (i = 0; i < hlen; ++i) /*----------------------- for each input byte, */
  {
   for (j = 0; j < 8; ++j) /*-------------------------- for each input bit, */
   {
    for (m = 1; m < 8; ++m) /*------------- for serveral possible initvals, */
    {
     for (l = 0; l < HASHSTATE; ++l)
      e[l] = f[l] = g[l] = h[l] = x[l] = y[l] = ~((uint32_t) 0);
     /*---------- check that every output bit is affected by that input bit */
     for (k = 0; k < MAXPAIR; k += 2)
     {
      uint32_t finished = 1;
      /* keys have one bit different */
      for (l = 0; l < hlen + 1; ++l)
      {
       a[l] = b[l] = (uint8_t) 0;
      }
      /* have a and b be two keys differing in only one bit */
      a[i] ^= (k << j);
      a[i] ^= (k >> (8 - j));
      c[0] = hashlittle(a, hlen, m);
      b[i] ^= ((k + 1) << j);
      b[i] ^= ((k + 1) >> (8 - j));
      d[0] = hashlittle(b, hlen, m);
      /* check every bit is 1, 0, set, and not set at least once */
      for (l = 0; l < HASHSTATE; ++l)
      {
       e[l] &= (c[l] ^ d[l]);
       f[l] &= ~(c[l] ^ d[l]);
       g[l] &= c[l];
       h[l] &= ~c[l];
       x[l] &= d[l];
       y[l] &= ~d[l];
       if (e[l] | f[l] | g[l] | h[l] | x[l] | y[l])
        finished = 0;
      }
      if (finished)
       break;
     }
     if (k > z)
      z = k;
     if (k == MAXPAIR)
     {
      fprintf(stderr, "Some bit didn't change: ");
      fprintf(stderr, "%.8x %.8x %.8x %.8x %.8x %.8x  ",
              e[0], f[0], g[0], h[0], x[0], y[0]);
      fprintf(stderr, "i %ld j %ld m %ld len %ld\n", i, j, m, hlen);
     }
     if (z == MAXPAIR)
      goto done;
    }
   }
  }
done:
  if (z < MAXPAIR)
  {
   fprintf(stderr, "Mix success  %2ld bytes  %2ld initvals  ", i, m);
   fprintf(stderr, "required  %ld  trials\n", (z / 2));
  }
 }
 fprintf("\n");
 return;
}

/* Check for reading beyond the end of the buffer and alignment problems */
void driver3(void)
{
 uint8_t buf[MAXLEN + 20];
 uint8_t* b;
 uint32_t len;
 uint8_t q[] = "This is the time for all good men to come to the aid of their country...";
 uint32_t dummy1;
 uint8_t qq[] = "xThis is the time for all good men to come to the aid of their country...";
 uint32_t dummy2;
 uint8_t qqq[] = "xxThis is the time for all good men to come to the aid of their country...";
 uint32_t dummy3;
 uint8_t qqqq[] = "xxxThis is the time for all good men to come to the aid of their country...";
 uint32_t h, i, j, ref, x, y;
 uint8_t* p;

 fprintf(stderr, "Endianness.  These lines should all be the same (for values filled in):\n");
 fprintf(stderr, "%.8x                            %.8x                            %.8x\n",
         hashword(q, ((sizeof(q) - 1) / 4), 13), hashword(q, ((sizeof(q) - 5) / 4), 13),
         hashword(q, ((sizeof(q) - 9) / 4), 13));
 p = q;
 fprintf(stderr, "%.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x\n",
        hashlittle(p, sizeof(q) - 1, 13), hashlittle(p, sizeof(q) - 2, 13),
        hashlittle(p, sizeof(q) - 3, 13), hashlittle(p, sizeof(q) - 4, 13),
        hashlittle(p, sizeof(q) - 5, 13), hashlittle(p, sizeof(q) - 6, 13),
        hashlittle(p, sizeof(q) - 7, 13), hashlittle(p, sizeof(q) - 8, 13),
        hashlittle(p, sizeof(q) - 9, 13), hashlittle(p, sizeof(q) - 10, 13),
        hashlittle(p, sizeof(q) - 11, 13), hashlittle(p, sizeof(q) - 12, 13));
 p = &qq[1];
 fprintf(stderr, "%.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x\n",
        hashlittle(p, sizeof(q) - 1, 13), hashlittle(p, sizeof(q) - 2, 13),
        hashlittle(p, sizeof(q) - 3, 13), hashlittle(p, sizeof(q) - 4, 13),
        hashlittle(p, sizeof(q) - 5, 13), hashlittle(p, sizeof(q) - 6, 13),
        hashlittle(p, sizeof(q) - 7, 13), hashlittle(p, sizeof(q) - 8, 13),
        hashlittle(p, sizeof(q) - 9, 13), hashlittle(p, sizeof(q) - 10, 13),
        hashlittle(p, sizeof(q) - 11, 13), hashlittle(p, sizeof(q) - 12, 13));
 p = &qqq[2];
 fprintf(stderr, "%.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x\n",
         hashlittle(p, sizeof(q) - 1, 13), hashlittle(p, sizeof(q) - 2, 13),
         hashlittle(p, sizeof(q) - 3, 13), hashlittle(p, sizeof(q) - 4, 13),
         hashlittle(p, sizeof(q) - 5, 13), hashlittle(p, sizeof(q) - 6, 13),
         hashlittle(p, sizeof(q) - 7, 13), hashlittle(p, sizeof(q) - 8, 13),
         hashlittle(p, sizeof(q) - 9, 13), hashlittle(p, sizeof(q) - 10, 13),
         hashlittle(p, sizeof(q) - 11, 13), hashlittle(p, sizeof(q) - 12, 13));
 p = &qqqq[3];
 fprintf(stderr, "%.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x\n",
         hashlittle(p, sizeof(q) - 1, 13), hashlittle(p, sizeof(q) - 2, 13),
         hashlittle(p, sizeof(q) - 3, 13), hashlittle(p, sizeof(q) - 4, 13),
         hashlittle(p, sizeof(q) - 5, 13), hashlittle(p, sizeof(q) - 6, 13),
         hashlittle(p, sizeof(q) - 7, 13), hashlittle(p, sizeof(q) - 8, 13),
         hashlittle(p, sizeof(q) - 9, 13), hashlittle(p, sizeof(q) - 10, 13),
         hashlittle(p, sizeof(q) - 11, 13), hashlittle(p, sizeof(q) - 12, 13));
 fprintf(stderr, "\n");
 for (h = 0, b = buf + 1; h < 8; ++h, ++b)
 {
  for (i = 0; i < MAXLEN; ++i)
  {
   len = i;
   for (j = 0; j < i; ++j)
    *(b + j) = 0;
   /* these should all be equal */
   ref = hashlittle(b, len, (uint32_t) 1);
   *(b + i) = (uint8_t) ~ 0;
   *(b - 1) = (uint8_t) ~ 0;
   x = hashlittle(b, len, (uint32_t) 1);
   y = hashlittle(b, len, (uint32_t) 1);
   if ((ref != x) || (ref != y))
   {
    fprintf(stderr, "alignment error: %.8x %.8x %.8x %ld %ld\n", ref, x, y, h, i);
   }
  }
 }
 return;
}

/* check for problems with nulls */
 void driver4(void)
{
 uint8_t buf[1];
 uint32_t h, i, state[HASHSTATE];

 buf[0] = ~0;
 for (i = 0; i < HASHSTATE; ++i)
  state[i] = 1;
 fprintf(stderr, "These should all be different\n");
 for (i = 0, h = 0; i < 8; ++i)
 {
  h = hashlittle(buf, 0, h);
  fprintf(stderr, "%2ld 0-byte strings, hash is %.8x\n", i, h);
 }
 return;
}

void bj_hash_test(void)
{
 driver1(); /* test that the key is hashed: used for timings */
 driver2(); /* test that whole key is hashed thoroughly */
 driver3(); /* test that nothing but the key is hashed */
 driver4(); /* test hashing multiple buffers (all buffers are null) */
 return;
}

#endif /* BJ_SELF_TEST */
