/*
 * UFC-crypt: ultra fast crypt(3) implementation
 *
 * Copyright (C) 1991, 1992, Free Software Foundation, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * @(#)crypt_util.c	2.40 09/21/92
 *
 * Support routines
 *
 */

#ifndef HTUTILS_H
#include <HTUtils.h>
#endif

#ifdef DEBUG
/*#include <stdio.h>  included by HTUTils.h - FM */
#endif

#ifndef STATIC
#define STATIC static
#endif

#ifndef DOS
#include <patchlevel.h>
#include <ufc-crypt.h>
#else
/*
 * Thanks to greg%wind@plains.NoDak.edu (Greg W. Wettstein)
 * for DOS patches
 */
#include <pl.h>
#include <ufc.h>
#endif

#include <LYLeaks.h>

static char patchlevel_str[] = PATCHLEVEL;

/* 
 * Permutation done once on the 56 bit 
 *  key derived from the original 8 byte ASCII key.
 */
static int pc1[56] = { 
  57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
  10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
  63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
  14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
};

/*
 * How much to rotate each 28 bit half of the pc1 permutated
 *  56 bit key before using pc2 to give the i' key
 */
static int rots[16] = { 
  1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 
};

/* 
 * Permutation giving the key 
 * of the i' DES round 
 */
static int pc2[48] = { 
  14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
  23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
  41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
  44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};

/*
 * The E expansion table which selects
 * bits from the 32 bit intermediate result.
 */
static int esel[48] = { 
  32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
   8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
  16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
  24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
};
static int e_inverse[64];

/* 
 * Permutation done on the 
 * result of sbox lookups 
 */
static int perm32[32] = {
  16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
  2,   8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
};

/* 
 * The sboxes
 */
static int sbox[8][4][16]= {
        { { 14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7 },
          {  0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8 },
          {  4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0 },
          { 15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13 }
        },

        { { 15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10 },
          {  3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5 },
          {  0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15 },
          { 13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9 }
        },

        { { 10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8 },
          { 13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1 },
          { 13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7 },
          {  1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12 }
        },

        { {  7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15 },
          { 13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9 },
          { 10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4 },
          {  3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14 }
        },

        { {  2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9 },
          { 14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6 },
          {  4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14 },
          { 11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 }
        },

        { { 12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11 },
          { 10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8 },
          {  9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6 },
          {  4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 }
        },

        { {  4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1 },
          { 13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6 },
          {  1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2 },
          {  6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12 }
        },

        { { 13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7 },
          {  1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2 },
          {  7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8 },
          {  2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 }
        }
};

/* 
 * This is the initial 
 * permutation matrix
 */
static int initial_perm[64] = { 
  58, 50, 42, 34, 26, 18, 10,  2, 60, 52, 44, 36, 28, 20, 12, 4,
  62, 54, 46, 38, 30, 22, 14,  6, 64, 56, 48, 40, 32, 24, 16, 8,
  57, 49, 41, 33, 25, 17,  9,  1, 59, 51, 43, 35, 27, 19, 11, 3,
  61, 53, 45, 37, 29, 21, 13,  5, 63, 55, 47, 39, 31, 23, 15, 7
};

/* 
 * This is the final 
 * permutation matrix
 */
static int final_perm[64] = {
  40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
  38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
  36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
  34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25
};

/* 
 * The 16 DES keys in BITMASK format 
 */
#ifdef _UFC_32_
long32 _ufc_keytab[16][2];
#endif
#ifdef _UFC_64_
long64 _ufc_keytab[16];
#endif

#define ascii_to_bin(c) ((c)>='a'?(c-59):(c)>='A'?((c)-53):(c)-'.')
#define bin_to_ascii(c) ((c)>=38?((c)-38+'a'):(c)>=12?((c)-12+'A'):(c)+'.')

/* Macro to set a bit (0..23) */
#define BITMASK(i) ( (1L<<(11L-(i)%12L+3L)) << ((i)<12L?16L:0L) )

/*
 * sb arrays:
 *
 * Workhorses of the inner loop of the DES implementation.
 * They do sbox lookup, shifting of this  value, 32 bit
 * permutation and E permutation for the next round.
 *
 * Kept in 'BITMASK' format.
 */

#ifdef _UFC_32_
long32 _ufc_sb0[8192], _ufc_sb1[8192], _ufc_sb2[8192], _ufc_sb3[8192];
static long32 *sb[4] = {_ufc_sb0, _ufc_sb1, _ufc_sb2, _ufc_sb3}; 
#endif

#ifdef _UFC_64_
long64 _ufc_sb0[4096], _ufc_sb1[4096], _ufc_sb2[4096], _ufc_sb3[4096];
static long64 *sb[4] = {_ufc_sb0, _ufc_sb1, _ufc_sb2, _ufc_sb3}; 
#endif

/* 
 * eperm32tab: do 32 bit permutation and E selection
 *
 * The first index is the byte number in the 32 bit value to be permuted
 *  -  second  -   is the value of this byte
 *  -  third   -   selects the two 32 bit values
 *
 * The table is used and generated internally in init_des to speed it up
 */
static ufc_long eperm32tab[4][256][2];

/* 
 * do_pc1: permform pc1 permutation in the key schedule generation.
 *
 * The first   index is the byte number in the 8 byte ASCII key
 *  -  second    -      -    the two 28 bits halfs of the result
 *  -  third     -   selects the 7 bits actually used of each byte
 *
 * The result is kept with 28 bit per 32 bit with the 4 most significant
 * bits zero.
 */
static ufc_long do_pc1[8][2][128];

/*
 * do_pc2: permform pc2 permutation in the key schedule generation.
 *
 * The first   index is the septet number in the two 28 bit intermediate values
 *  -  second    -    -  -  septet values
 *
 * Knowledge of the structure of the pc2 permutation is used.
 *
 * The result is kept with 28 bit per 32 bit with the 4 most significant
 * bits zero.
 */
static ufc_long do_pc2[8][128];

/*
 * efp: undo an extra e selection and do final
 *      permutation giving the DES result.
 * 
 *      Invoked 6 bit a time on two 48 bit values
 *      giving two 32 bit longs.
 */
static ufc_long efp[16][64][2];

/*
 * revfinal: undo final permutation and do E expension.
 *
 *           Invoked 6 bit a time on DES output
 *           giving 4 32 bit longs.
 */
static ufc_long revfinal[11][64][4];


static unsigned char bytemask[8]  = {
  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static ufc_long longmask[32] = {
  0x80000000, 0x40000000, 0x20000000, 0x10000000,
  0x08000000, 0x04000000, 0x02000000, 0x01000000,
  0x00800000, 0x00400000, 0x00200000, 0x00100000,
  0x00080000, 0x00040000, 0x00020000, 0x00010000,
  0x00008000, 0x00004000, 0x00002000, 0x00001000,
  0x00000800, 0x00000400, 0x00000200, 0x00000100,
  0x00000080, 0x00000040, 0x00000020, 0x00000010,
  0x00000008, 0x00000004, 0x00000002, 0x00000001
};

#ifdef DEBUG

pr_bits(a, n)
  ufc_long *a;
  int n;
  { ufc_long i, j, t, tmp;
    n /= 8;
    for(i = 0; i < n; i++) {
      tmp=0;
      for(j = 0; j < 8; j++) {
	t=8*i+j;
	tmp|=(a[t/24] & BITMASK(t % 24))?bytemask[j]:0;
      }
      (void)printf("%02x ",tmp);
    }
    printf(" ");
  }

static set_bits(v, b)
  ufc_long v;
  ufc_long *b;
  { ufc_long i;
    *b = 0;
    for(i = 0; i < 24; i++) {
      if(v & longmask[8 + i])
	*b |= BITMASK(i);
    }
  }

#endif

/*
 * Silly rewrite of 'bzero'. I do so
 * because some machines don't have
 * bzero and some don't have memset.
 */

STATIC void clearmem(start, cnt)
  char *start;
  int cnt;
  { while(cnt--)
      *start++ = '\0';
  }

static int initialized = 0;

/* lookup a 6 bit value in sbox */

#define s_lookup(i,s) sbox[(i)][(((s)>>4) & 0x2)|((s) & 0x1)][((s)>>1) & 0xf];

/*
 * Initialize unit - may be invoked directly
 * by fcrypt users.
 */

void init_des()
  { int comes_from_bit;
    int bit, sg;
    ufc_long j;
    ufc_long mask1, mask2;

    /*
     * Create the do_pc1 table used
     * to affect pc1 permutation
     * when generating keys
     */
    for(bit = 0; bit < 56; bit++) {
      comes_from_bit  = pc1[bit] - 1;
      mask1 = bytemask[comes_from_bit % 8 + 1];
      mask2 = longmask[bit % 28 + 4];
      for(j = 0; j < 128; j++) {
	if(j & mask1) 
	  do_pc1[comes_from_bit / 8][bit / 28][j] |= mask2;
      }
    }

    /*
     * Create the do_pc2 table used
     * to affect pc2 permutation when
     * generating keys
     */
    for(bit = 0; bit < 48; bit++) {
      comes_from_bit  = pc2[bit] - 1;
      mask1 = bytemask[comes_from_bit % 7 + 1];
      mask2 = BITMASK(bit % 24);
      for(j = 0; j < 128; j++) {
	if(j & mask1)
	  do_pc2[comes_from_bit / 7][j] |= mask2;
      }
    }

    /* 
     * Now generate the table used to do combined
     * 32 bit permutation and e expansion
     *
     * We use it because we have to permute 16384 32 bit
     * longs into 48 bit in order to initialize sb.
     *
     * Looping 48 rounds per permutation becomes 
     * just too slow...
     *
     */

    clearmem((char*)eperm32tab, sizeof(eperm32tab));

    for(bit = 0; bit < 48; bit++) {
      ufc_long mask1,comes_from;
	
      comes_from = perm32[esel[bit]-1]-1;
      mask1      = bytemask[comes_from % 8];
	
      for(j = 256; j--;) {
	if(j & mask1)
	  eperm32tab[comes_from / 8][j][bit / 24] |= BITMASK(bit % 24);
      }
    }
    
    /* 
     * Create the sb tables:
     *
     * For each 12 bit segment of an 48 bit intermediate
     * result, the sb table precomputes the two 4 bit
     * values of the sbox lookups done with the two 6
     * bit halves, shifts them to their proper place,
     * sends them through perm32 and finally E expands
     * them so that they are ready for the next
     * DES round.
     *
     */
    for(sg = 0; sg < 4; sg++) {
      int j1, j2;
      int s1, s2;
    
      for(j1 = 0; j1 < 64; j1++) {
	s1 = s_lookup(2 * sg, j1);
	for(j2 = 0; j2 < 64; j2++) {
	  ufc_long to_permute, inx;
    
	  s2         = s_lookup(2 * sg + 1, j2);
	  to_permute = (((ufc_long)s1 << 4)  | 
	               (ufc_long)s2) << (24 - 8 * (ufc_long)sg);

#ifdef _UFC_32_
	  inx = ((j1 << 6)  | j2) << 1;
	  sb[sg][inx  ]  = eperm32tab[0][(to_permute >> 24) & 0xff][0];
	  sb[sg][inx+1]  = eperm32tab[0][(to_permute >> 24) & 0xff][1];
	  sb[sg][inx  ] |= eperm32tab[1][(to_permute >> 16) & 0xff][0];
	  sb[sg][inx+1] |= eperm32tab[1][(to_permute >> 16) & 0xff][1];
  	  sb[sg][inx  ] |= eperm32tab[2][(to_permute >>  8) & 0xff][0];
	  sb[sg][inx+1] |= eperm32tab[2][(to_permute >>  8) & 0xff][1];
	  sb[sg][inx  ] |= eperm32tab[3][(to_permute)       & 0xff][0];
	  sb[sg][inx+1] |= eperm32tab[3][(to_permute)       & 0xff][1];
#endif
#ifdef _UFC_64_
	  inx = ((j1 << 6)  | j2);
	  sb[sg][inx]  = 
	    ((long64)eperm32tab[0][(to_permute >> 24) & 0xff][0] << 32) |
	     (long64)eperm32tab[0][(to_permute >> 24) & 0xff][1];
	  sb[sg][inx] |=
	    ((long64)eperm32tab[1][(to_permute >> 16) & 0xff][0] << 32) |
	     (long64)eperm32tab[1][(to_permute >> 16) & 0xff][1];
  	  sb[sg][inx] |= 
	    ((long64)eperm32tab[2][(to_permute >>  8) & 0xff][0] << 32) |
	     (long64)eperm32tab[2][(to_permute >>  8) & 0xff][1];
	  sb[sg][inx] |=
	    ((long64)eperm32tab[3][(to_permute)       & 0xff][0] << 32) |
	     (long64)eperm32tab[3][(to_permute)       & 0xff][1];
#endif
	}
      }
    }  

    /* 
     * Create an inverse matrix for esel telling
     * where to plug out bits if undoing it
     */
    for(bit=48; bit--;) {
      e_inverse[esel[bit] - 1     ] = bit;
      e_inverse[esel[bit] - 1 + 32] = bit + 48;
    }

    /* 
     * create efp: the matrix used to
     * undo the E expansion and effect final permutation
     */
    clearmem((char*)efp, sizeof efp);
    for(bit = 0; bit < 64; bit++) {
      int o_bit, o_long;
      ufc_long word_value, mask1, mask2;
      int comes_from_f_bit, comes_from_e_bit;
      int comes_from_word, bit_within_word;

      /* See where bit i belongs in the two 32 bit long's */
      o_long = bit / 32; /* 0..1  */
      o_bit  = bit % 32; /* 0..31 */

      /* 
       * And find a bit in the e permutated value setting this bit.
       *
       * Note: the e selection may have selected the same bit several
       * times. By the initialization of e_inverse, we only look
       * for one specific instance.
       */
      comes_from_f_bit = final_perm[bit] - 1;         /* 0..63 */
      comes_from_e_bit = e_inverse[comes_from_f_bit]; /* 0..95 */
      comes_from_word  = comes_from_e_bit / 6;        /* 0..15 */
      bit_within_word  = comes_from_e_bit % 6;        /* 0..5  */

      mask1 = longmask[bit_within_word + 26];
      mask2 = longmask[o_bit];

      for(word_value = 64; word_value--;) {
	if(word_value & mask1)
	  efp[comes_from_word][word_value][o_long] |= mask2;
      }
    }

    
    /*
     * Create revfinal: an array to undo final
     * the effects of efp
     */
    clearmem((char*)revfinal, sizeof(revfinal));
    for(bit = 0; bit < 96; bit++) {
      int ibit = initial_perm[esel[bit % 48] - 1 + ((bit >= 48) ? 32 : 0)] - 1;
      mask1 = bytemask[ibit % 6 +  2];
      mask2 = BITMASK(bit % 24);
      for(j = 64; j--;) {
        if(j & mask1) {
          revfinal[ibit / 6][j][bit / 24] |= mask2;
        }
      }
    }

    initialized++;
  }

/* 
 * Process the elements of the sb table permuting the
 * bits swapped in the expansion by the current salt.
 */

#ifdef _UFC_32_
STATIC void shuffle_sb(k, saltbits)
  long32 *k;
  ufc_long saltbits;
  { ufc_long j;
    long32 x;
    for(j=4096; j--;) {
      x = (k[0] ^ k[1]) & (long32)saltbits;
      *k++ ^= x;
      *k++ ^= x;
    }
  }
#endif

#ifdef _UFC_64_
STATIC void shuffle_sb(k, saltbits)
  long64 *k;
  ufc_long saltbits;
  { ufc_long j;
    long64 x;
    for(j=4096; j--;) {
      x = ((*k >> 32) ^ *k) & (long64)saltbits;
      *k++ ^= (x << 32) | x;
    }
  }
#endif

/* 
 * Setup the unit for a new salt
 * Hopefully we'll not see a new salt in each crypt call.
 */

static unsigned char current_salt[3] = "&&"; /* invalid value */
static ufc_long current_saltbits = 0;
static int direction = 0;

STATIC void setup_salt(s)
  char *s;
  { ufc_long i, j, saltbits;

    if(!initialized)
      init_des();

    if(s[0] == current_salt[0] && s[1] == current_salt[1])
      return;
    current_salt[0] = s[0]; current_salt[1] = s[1];
    
    /* 
     * This is the only crypt change to DES:
     * entries are swapped in the expansion table
     * according to the bits set in the salt.
     */
    saltbits = 0;
    for(i = 0; i < 2; i++) {
      long c=ascii_to_bin(s[i]);
#ifdef notdef
      /* 
       * Some applications do rely on illegal
       * salts. It seems that UFC-crypt behaves
       * identically to standard crypt 
       * implementations on illegal salts -- glad
       */
      if(c < 0 || c > 63)
	c = 0;
#endif
      for(j = 0; j < 6; j++) {
	if((c >> j) & 0x1)
	  saltbits |= BITMASK(6 * i + j);
      }
    }

    /*
     * Permute the sb table values
     * to reflect the changed e
     * selection table
     */
    shuffle_sb(_ufc_sb0, current_saltbits ^ saltbits); 
    shuffle_sb(_ufc_sb1, current_saltbits ^ saltbits);
    shuffle_sb(_ufc_sb2, current_saltbits ^ saltbits);
    shuffle_sb(_ufc_sb3, current_saltbits ^ saltbits);

    current_saltbits = saltbits;
  }

STATIC void ufc_mk_keytab(key)
  char *key;
  { ufc_long v1, v2, *k1;
    int i;
#ifdef _UFC_32_
    long32 v, *k2 = &_ufc_keytab[0][0];
#endif
#ifdef _UFC_64_
    long64 v, *k2 = &_ufc_keytab[0];
#endif

    v1 = v2 = 0; k1 = &do_pc1[0][0][0];
    for(i = 8; i--;) {
      v1 |= k1[*key   & 0x7f]; k1 += 128;
      v2 |= k1[*key++ & 0x7f]; k1 += 128;
    }

    for(i = 0; i < 16; i++) {
      k1 = &do_pc2[0][0];

      v1 = (v1 << rots[i]) | (v1 >> (28 - rots[i]));
      v  = k1[(v1 >> 21) & 0x7f]; k1 += 128;
      v |= k1[(v1 >> 14) & 0x7f]; k1 += 128;
      v |= k1[(v1 >>  7) & 0x7f]; k1 += 128;
      v |= k1[(v1      ) & 0x7f]; k1 += 128;

#ifdef _UFC_32_
      *k2++ = v;
      v = 0;
#endif
#ifdef _UFC_64_
      v <<= 32;
#endif

      v2 = (v2 << rots[i]) | (v2 >> (28 - rots[i]));
      v |= k1[(v2 >> 21) & 0x7f]; k1 += 128;
      v |= k1[(v2 >> 14) & 0x7f]; k1 += 128;
      v |= k1[(v2 >>  7) & 0x7f]; k1 += 128;
      v |= k1[(v2      ) & 0x7f];

      *k2++ = v;
    }

    direction = 0;
  }

/* 
 * Undo an extra E selection and do final permutations
 */

ufc_long *_ufc_dofinalperm(l1, l2, r1, r2)
  ufc_long l1,l2,r1,r2;
  { ufc_long v1, v2, x;
    static ufc_long ary[2];

    x = (l1 ^ l2) & current_saltbits; l1 ^= x; l2 ^= x;
    x = (r1 ^ r2) & current_saltbits; r1 ^= x; r2 ^= x;

    v1=v2=0; l1 >>= 3; l2 >>= 3; r1 >>= 3; r2 >>= 3;

    v1 |= efp[15][ r2         & 0x3f][0]; v2 |= efp[15][ r2 & 0x3f][1];
    v1 |= efp[14][(r2 >>= 6)  & 0x3f][0]; v2 |= efp[14][ r2 & 0x3f][1];
    v1 |= efp[13][(r2 >>= 10) & 0x3f][0]; v2 |= efp[13][ r2 & 0x3f][1];
    v1 |= efp[12][(r2 >>= 6)  & 0x3f][0]; v2 |= efp[12][ r2 & 0x3f][1];

    v1 |= efp[11][ r1         & 0x3f][0]; v2 |= efp[11][ r1 & 0x3f][1];
    v1 |= efp[10][(r1 >>= 6)  & 0x3f][0]; v2 |= efp[10][ r1 & 0x3f][1];
    v1 |= efp[ 9][(r1 >>= 10) & 0x3f][0]; v2 |= efp[ 9][ r1 & 0x3f][1];
    v1 |= efp[ 8][(r1 >>= 6)  & 0x3f][0]; v2 |= efp[ 8][ r1 & 0x3f][1];

    v1 |= efp[ 7][ l2         & 0x3f][0]; v2 |= efp[ 7][ l2 & 0x3f][1];
    v1 |= efp[ 6][(l2 >>= 6)  & 0x3f][0]; v2 |= efp[ 6][ l2 & 0x3f][1];
    v1 |= efp[ 5][(l2 >>= 10) & 0x3f][0]; v2 |= efp[ 5][ l2 & 0x3f][1];
    v1 |= efp[ 4][(l2 >>= 6)  & 0x3f][0]; v2 |= efp[ 4][ l2 & 0x3f][1];

    v1 |= efp[ 3][ l1         & 0x3f][0]; v2 |= efp[ 3][ l1 & 0x3f][1];
    v1 |= efp[ 2][(l1 >>= 6)  & 0x3f][0]; v2 |= efp[ 2][ l1 & 0x3f][1];
    v1 |= efp[ 1][(l1 >>= 10) & 0x3f][0]; v2 |= efp[ 1][ l1 & 0x3f][1];
    v1 |= efp[ 0][(l1 >>= 6)  & 0x3f][0]; v2 |= efp[ 0][ l1 & 0x3f][1];

    ary[0] = v1; ary[1] = v2;
    return ary;
  }

/* 
 * crypt only: convert from 64 bit to 11 bit ASCII 
 * prefixing with the salt
 */

STATIC char *output_conversion(v1, v2, salt)
  ufc_long v1, v2;
  char *salt;
  { static char outbuf[14];
    int i, s, shf;

    outbuf[0] = salt[0];
    outbuf[1] = salt[1] ? salt[1] : salt[0];

    for(i = 0; i < 5; i++) {
      shf = (26 - 6 * i); /* to cope with MSC compiler bug */
      outbuf[i + 2] = bin_to_ascii((v1 >> shf) & 0x3f);
    }

    s  = (v2 & 0xf) << 2;
    v2 = (v2 >> 2) | ((v1 & 0x3) << 30);

    for(i = 5; i < 10; i++) {
      shf = (56 - 6 * i);
      outbuf[i + 2] = bin_to_ascii((v2 >> shf) & 0x3f);
    }

    outbuf[12] = bin_to_ascii(s);
    outbuf[13] = 0;

    return outbuf;
  }

ufc_long *_ufc_doit();

/* 
 * UNIX crypt function
 */
   
char *crypt(key, salt)
  char *key, *salt;
  { ufc_long *s;
    char ktab[9];

    /*
     * Hack DES tables according to salt
     */
    setup_salt(salt);

    /*
     * Setup key schedule
     */
    clearmem(ktab, sizeof ktab);
    (void)strncpy(ktab, key, 8);
    ufc_mk_keytab(ktab);

    /*
     * Go for the 25 DES encryptions
     */
    s = _ufc_doit((ufc_long)0, (ufc_long)0, 
		  (ufc_long)0, (ufc_long)0, (ufc_long)25);
    /*
     * Do final permutations
     */
    s = _ufc_dofinalperm(s[0], s[1], s[2], s[3]);

    /*
     * And convert back to 6 bit ASCII
     */
    return output_conversion(s[0], s[1], salt);
  }

/* 
 * To make fcrypt users happy.
 * They don't need to call init_des.
 */

char *fcrypt(key, salt)
  char *key;
  char *salt;
  { return crypt(key, salt);
  }

/* 
 * UNIX encrypt function. Takes a bitvector
 * represented by one byte per bit and
 * encrypt/decrypt according to edflag
 */

void encrypt(block, edflag)
  char *block;
  int edflag;
  { ufc_long l1, l2, r1, r2, *s;
    int i;

    /*
     * Undo any salt changes to E expansion
     */
    setup_salt("..");

    /*
     * Reverse key table if
     * changing operation (encrypt/decrypt)
     */
    if((edflag == 0) != (direction == 0)) {
      for(i = 0; i < 8; i++) {
#ifdef _UFC_32_
	long32 x;
	x = _ufc_keytab[15-i][0]; 
        _ufc_keytab[15-i][0] = _ufc_keytab[i][0]; 
        _ufc_keytab[i][0] = x;

	x = _ufc_keytab[15-i][1]; 
        _ufc_keytab[15-i][1] = _ufc_keytab[i][1]; 
        _ufc_keytab[i][1] = x;
#endif
#ifdef _UFC_64_
	long64 x;
	x = _ufc_keytab[15-i];
	_ufc_keytab[15-i] = _ufc_keytab[i];
	_ufc_keytab[i] = x;
#endif
      }
      direction = edflag;
    }

    /*
     * Do initial permutation + E expansion
     */
    i = 0;
    for(l1 = 0; i < 24; i++) {
      if(block[initial_perm[esel[i]-1]-1])
	l1 |= BITMASK(i);
    }
    for(l2 = 0; i < 48; i++) {
      if(block[initial_perm[esel[i]-1]-1])
	l2 |= BITMASK(i-24);
    }

    i = 0;
    for(r1 = 0; i < 24; i++) {
      if(block[initial_perm[esel[i]-1+32]-1])
	r1 |= BITMASK(i);
    }
    for(r2 = 0; i < 48; i++) {
      if(block[initial_perm[esel[i]-1+32]-1])
	r2 |= BITMASK(i-24);
    }

    /*
     * Do DES inner loops + final conversion
     */
    s = _ufc_doit(l1, l2, r1, r2, (ufc_long)1);
    /*
     * Do final permutations
     */
    s = _ufc_dofinalperm(s[0], s[1], s[2], s[3]);

    /*
     * And convert to bit array
     */
    l1 = s[0]; r1 = s[1];
    for(i = 0; i < 32; i++) {
      *block++ = (l1 & longmask[i]) != 0;
    }
    for(i = 0; i < 32; i++) {
      *block++ = (r1 & longmask[i]) != 0;
    }
    
  }

/* 
 * UNIX setkey function. Take a 64 bit DES
 * key and setup the machinery.
 */

void setkey(key)
  char *key;
  { int i,j;
    unsigned char c;
    unsigned char ktab[8];

    setup_salt(".."); /* be sure we're initialized */

    for(i = 0; i < 8; i++) {
      for(j = 0, c = 0; j < 8; j++)
	c = c << 1 | *key++;
      ktab[i] = c >> 1;
    }
    
    ufc_mk_keytab(ktab);
  }

/* 
 * Ultrix crypt16 function, thanks to pcl@convex.oxford.ac.uk (Paul Leyland)
 */
   
char *crypt16(key, salt)
  char *key, *salt;
  { ufc_long *s, *t;
    char ktab[9], ttab[9];
    static char q[14], res[25];
    /*
     * Hack DES tables according to salt
     */
    setup_salt(salt);
    
    /*
     * Setup key schedule
     */
    clearmem(ktab, sizeof ktab);
    (void)strncpy(ktab, key, 8);
    ufc_mk_keytab(ktab);
    
    /*
     * Go for first 20 DES encryptions
     */
    s = _ufc_doit((ufc_long)0, (ufc_long)0, 
		  (ufc_long)0, (ufc_long)0, (ufc_long)20);
    
    /*
     * And convert back to 6 bit ASCII
     */
    strcpy (res, output_conversion(s[0], s[1], salt));
    
    clearmem(ttab, sizeof ttab);
    if (strlen (key) > 8) (void)strncpy(ttab, key+8, 8);
    ufc_mk_keytab(ttab);
    
    /*
     * Go for second 5 DES encryptions
     */
    t = _ufc_doit((ufc_long)0, (ufc_long)0, 
		  (ufc_long)0, (ufc_long)0, (ufc_long)5);
    /*
     * And convert back to 6 bit ASCII
     */
    strcpy (q, output_conversion(t[0], t[1], salt));
    strcpy (res+13, q+2);
    
    clearmem(ktab, sizeof ktab);
    (void)strncpy(ktab, key, 8);
    ufc_mk_keytab(ktab);
    
    return res;
  }

/*
 * Experimental -- not supported -- may choke your dog
 */

void ufc_setup_password(cookie, s)
  long *cookie;
  char *s;
  { char c;
    int i;
    ufc_long x;
    ufc_long dl1, dl2, dr1, dr2;

    setup_salt(s);
    dl1 = dl2 = dr1 = dr2 = 0;
    for(i = 0, s += 2; c = *s++; i++) {
      int x = ascii_to_bin(c);
      dl1 |= revfinal[i][x][0];
      dl2 |= revfinal[i][x][1];
      dr1 |= revfinal[i][x][2];
      dr2 |= revfinal[i][x][3];
    }
    x = (dl1 ^ dl2) & current_saltbits;
    x = (dr1 ^ dr2) & current_saltbits;
    cookie[0] = dl1 ^ x; cookie[1] = dl2 ^ x;
    cookie[2] = dr1 ^ x; cookie[3] = dr2 ^ x;
  }

void ufc_do_pw(cookie, guess)
  long *cookie;  
  char *guess;
  { char ktab[9];
    ufc_long *s;
    clearmem(ktab, sizeof ktab);
    (void)strncpy(ktab, guess, 8);
    ufc_mk_keytab(ktab);
    s = _ufc_doit((ufc_long)0, (ufc_long)0, 
		  (ufc_long)0, (ufc_long)0, (ufc_long)25);
    cookie[0] = s[0];    cookie[1] = s[1];
    cookie[2] = s[2];    cookie[3] = s[3];
  }
