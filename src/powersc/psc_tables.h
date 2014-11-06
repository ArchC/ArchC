/*****************************************************************************
                             PowerSC Library

                           Copyright 2004-2006
                       Computer Systems Laboratory
                          All Rights Reserved

 PERMISSION IS GRANTED TO USE, COPY AND REDISTRIBUTE THIS SOFTWARE FOR
 NONCOMMERCIAL EDUCATION AND RESEARCH PURPOSES, SO LONG AS NO FEE IS CHARGED,
 AND SO LONG AS THE COPYRIGHT NOTICE ABOVE, THIS GRANT OF PERMISSION, AND THE
 DISCLAIMER BELOW APPEAR IN ALL COPIES MADE; AND SO LONG AS THE NAME OF THE
 COMPUTER SYSTEMS LABORATORY IS NOT USED IN ANY ADVERTISING OR PUBLICITY
 PERTAINING TO THE USE OR DISTRIBUTION OF THIS SOFTWARE WITHOUT SPECIFIC,
 WRITTEN PRIOR AUTHORIZATION. PERMISSION TO MODIFY OR OTHERWISE CREATE
 DERIVATIVE WORKS OF THIS SOFTWARE IS NOT GRANTED.

 THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION AS TO ITS FITNESS
 FOR ANY PURPOSE, AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE STATE UNIVERSITY
 OF CAMPINAS, THE INSTITUTE OF COMPUTING AND THE COMPUTER SYSTEMS LABORATORY
 SHALL NOT BE LIABLE FOR ANY DAMAGES, INCLUDING SPECIAL, INDIRECT, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM ARISING OUT OF OR IN
 CONNECTION WITH THE USE OF THE SOFTWARE, EVEN IF IT HAS BEEN OR IS HEREAFTER
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *****************************************************************************/

/*****************************************************************************

  psc_tables.h -- Tables and associated functions used to reduce calculation time 
                  for toggle count

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_TABLES_H
#define PSC_TABLES_H

namespace psc_util
{

static int ON_BITS[ 256 ] = {
	0, 	// 0 (0x00)
	1, 	// 1 (0x01)
	1, 	// 2 (0x02)
	2, 	// 3 (0x03)
	1, 	// 4 (0x04)
	2, 	// 5 (0x05)
	2, 	// 6 (0x06)
	3, 	// 7 (0x07)
	1, 	// 8 (0x08)
	2, 	// 9 (0x09)
	2, 	// 10 (0x0A)
	3, 	// 11 (0x0B)
	2, 	// 12 (0x0C)
	3, 	// 13 (0x0D)
	3, 	// 14 (0x0E)
	4, 	// 15 (0x0F)
	1, 	// 16 (0x10)
	2, 	// 17 (0x11)
	2, 	// 18 (0x12)
	3, 	// 19 (0x13)
	2, 	// 20 (0x14)
	3, 	// 21 (0x15)
	3, 	// 22 (0x16)
	4, 	// 23 (0x17)
	2, 	// 24 (0x18)
	3, 	// 25 (0x19)
	3, 	// 26 (0x1A)
	4, 	// 27 (0x1B)
	3, 	// 28 (0x1C)
	4, 	// 29 (0x1D)
	4, 	// 30 (0x1E)
	5, 	// 31 (0x1F)
	1, 	// 32 (0x20)
	2, 	// 33 (0x21)
	2, 	// 34 (0x22)
	3, 	// 35 (0x23)
	2, 	// 36 (0x24)
	3, 	// 37 (0x25)
	3, 	// 38 (0x26)
	4, 	// 39 (0x27)
	2, 	// 40 (0x28)
	3, 	// 41 (0x29)
	3, 	// 42 (0x2A)
	4, 	// 43 (0x2B)
	3, 	// 44 (0x2C)
	4, 	// 45 (0x2D)
	4, 	// 46 (0x2E)
	5, 	// 47 (0x2F)
	2, 	// 48 (0x30)
	3, 	// 49 (0x31)
	3, 	// 50 (0x32)
	4, 	// 51 (0x33)
	3, 	// 52 (0x34)
	4, 	// 53 (0x35)
	4, 	// 54 (0x36)
	5, 	// 55 (0x37)
	3, 	// 56 (0x38)
	4, 	// 57 (0x39)
	4, 	// 58 (0x3A)
	5, 	// 59 (0x3B)
	4, 	// 60 (0x3C)
	5, 	// 61 (0x3D)
	5, 	// 62 (0x3E)
	6, 	// 63 (0x3F)
	1, 	// 64 (0x40)
	2, 	// 65 (0x41)
	2, 	// 66 (0x42)
	3, 	// 67 (0x43)
	2, 	// 68 (0x44)
	3, 	// 69 (0x45)
	3, 	// 70 (0x46)
	4, 	// 71 (0x47)
	2, 	// 72 (0x48)
	3, 	// 73 (0x49)
	3, 	// 74 (0x4A)
	4, 	// 75 (0x4B)
	3, 	// 76 (0x4C)
	4, 	// 77 (0x4D)
	4, 	// 78 (0x4E)
	5, 	// 79 (0x4F)
	2, 	// 80 (0x50)
	3, 	// 81 (0x51)
	3, 	// 82 (0x52)
	4, 	// 83 (0x53)
	3, 	// 84 (0x54)
	4, 	// 85 (0x55)
	4, 	// 86 (0x56)
	5, 	// 87 (0x57)
	3, 	// 88 (0x58)
	4, 	// 89 (0x59)
	4, 	// 90 (0x5A)
	5, 	// 91 (0x5B)
	4, 	// 92 (0x5C)
	5, 	// 93 (0x5D)
	5, 	// 94 (0x5E)
	6, 	// 95 (0x5F)
	2, 	// 96 (0x60)
	3, 	// 97 (0x61)
	3, 	// 98 (0x62)
	4, 	// 99 (0x63)
	3, 	// 100 (0x64)
	4, 	// 101 (0x65)
	4, 	// 102 (0x66)
	5, 	// 103 (0x67)
	3, 	// 104 (0x68)
	4, 	// 105 (0x69)
	4, 	// 106 (0x6A)
	5, 	// 107 (0x6B)
	4, 	// 108 (0x6C)
	5, 	// 109 (0x6D)
	5, 	// 110 (0x6E)
	6, 	// 111 (0x6F)
	3, 	// 112 (0x70)
	4, 	// 113 (0x71)
	4, 	// 114 (0x72)
	5, 	// 115 (0x73)
	4, 	// 116 (0x74)
	5, 	// 117 (0x75)
	5, 	// 118 (0x76)
	6, 	// 119 (0x77)
	4, 	// 120 (0x78)
	5, 	// 121 (0x79)
	5, 	// 122 (0x7A)
	6, 	// 123 (0x7B)
	5, 	// 124 (0x7C)
	6, 	// 125 (0x7D)
	6, 	// 126 (0x7E)
	7, 	// 127 (0x7F)
	1, 	// 128 (0x80)
	2, 	// 129 (0x81)
	2, 	// 130 (0x82)
	3, 	// 131 (0x83)
	2, 	// 132 (0x84)
	3, 	// 133 (0x85)
	3, 	// 134 (0x86)
	4, 	// 135 (0x87)
	2, 	// 136 (0x88)
	3, 	// 137 (0x89)
	3, 	// 138 (0x8A)
	4, 	// 139 (0x8B)
	3, 	// 140 (0x8C)
	4, 	// 141 (0x8D)
	4, 	// 142 (0x8E)
	5, 	// 143 (0x8F)
	2, 	// 144 (0x90)
	3, 	// 145 (0x91)
	3, 	// 146 (0x92)
	4, 	// 147 (0x93)
	3, 	// 148 (0x94)
	4, 	// 149 (0x95)
	4, 	// 150 (0x96)
	5, 	// 151 (0x97)
	3, 	// 152 (0x98)
	4, 	// 153 (0x99)
	4, 	// 154 (0x9A)
	5, 	// 155 (0x9B)
	4, 	// 156 (0x9C)
	5, 	// 157 (0x9D)
	5, 	// 158 (0x9E)
	6, 	// 159 (0x9F)
	2, 	// 160 (0xA0)
	3, 	// 161 (0xA1)
	3, 	// 162 (0xA2)
	4, 	// 163 (0xA3)
	3, 	// 164 (0xA4)
	4, 	// 165 (0xA5)
	4, 	// 166 (0xA6)
	5, 	// 167 (0xA7)
	3, 	// 168 (0xA8)
	4, 	// 169 (0xA9)
	4, 	// 170 (0xAA)
	5, 	// 171 (0xAB)
	4, 	// 172 (0xAC)
	5, 	// 173 (0xAD)
	5, 	// 174 (0xAE)
	6, 	// 175 (0xAF)
	3, 	// 176 (0xB0)
	4, 	// 177 (0xB1)
	4, 	// 178 (0xB2)
	5, 	// 179 (0xB3)
	4, 	// 180 (0xB4)
	5, 	// 181 (0xB5)
	5, 	// 182 (0xB6)
	6, 	// 183 (0xB7)
	4, 	// 184 (0xB8)
	5, 	// 185 (0xB9)
	5, 	// 186 (0xBA)
	6, 	// 187 (0xBB)
	5, 	// 188 (0xBC)
	6, 	// 189 (0xBD)
	6, 	// 190 (0xBE)
	7, 	// 191 (0xBF)
	2, 	// 192 (0xC0)
	3, 	// 193 (0xC1)
	3, 	// 194 (0xC2)
	4, 	// 195 (0xC3)
	3, 	// 196 (0xC4)
	4, 	// 197 (0xC5)
	4, 	// 198 (0xC6)
	5, 	// 199 (0xC7)
	3, 	// 200 (0xC8)
	4, 	// 201 (0xC9)
	4, 	// 202 (0xCA)
	5, 	// 203 (0xCB)
	4, 	// 204 (0xCC)
	5, 	// 205 (0xCD)
	5, 	// 206 (0xCE)
	6, 	// 207 (0xCF)
	3, 	// 208 (0xD0)
	4, 	// 209 (0xD1)
	4, 	// 210 (0xD2)
	5, 	// 211 (0xD3)
	4, 	// 212 (0xD4)
	5, 	// 213 (0xD5)
	5, 	// 214 (0xD6)
	6, 	// 215 (0xD7)
	4, 	// 216 (0xD8)
	5, 	// 217 (0xD9)
	5, 	// 218 (0xDA)
	6, 	// 219 (0xDB)
	5, 	// 220 (0xDC)
	6, 	// 221 (0xDD)
	6, 	// 222 (0xDE)
	7, 	// 223 (0xDF)
	3, 	// 224 (0xE0)
	4, 	// 225 (0xE1)
	4, 	// 226 (0xE2)
	5, 	// 227 (0xE3)
	4, 	// 228 (0xE4)
	5, 	// 229 (0xE5)
	5, 	// 230 (0xE6)
	6, 	// 231 (0xE7)
	4, 	// 232 (0xE8)
	5, 	// 233 (0xE9)
	5, 	// 234 (0xEA)
	6, 	// 235 (0xEB)
	5, 	// 236 (0xEC)
	6, 	// 237 (0xED)
	6, 	// 238 (0xEE)
	7, 	// 239 (0xEF)
	4, 	// 240 (0xF0)
	5, 	// 241 (0xF1)
	5, 	// 242 (0xF2)
	6, 	// 243 (0xF3)
	5, 	// 244 (0xF4)
	6, 	// 245 (0xF5)
	6, 	// 246 (0xF6)
	7, 	// 247 (0xF7)
	5, 	// 248 (0xF8)
	6, 	// 249 (0xF9)
	6, 	// 250 (0xFA)
	7, 	// 251 (0xFB)
	6, 	// 252 (0xFC)
	7, 	// 253 (0xFD)
	7, 	// 254 (0xFE)
	8 	// 255 (0xFF)
};

static unsigned long long MASK_BITS[ 64+1 ] = {
	0x0,	// for 0
	0x1,	// for 1
	0x3,	// for 2
	0x7,	// for 3
	0xf,	// for 4
	0x1f,	// for 5
	0x3f,	// for 6
	0x7f,	// for 7
	0xff,	// for 8
	0x1ff,	// for 9
	0x3ff,	// for 10
	0x7ff,	// for 11
	0xfff,	// for 12
	0x1fff,	// for 13
	0x3fff,	// for 14
	0x7fff,	// for 15
	0xffff,	// for 16
	0x1ffff,	// for 17
	0x3ffff,	// for 18
	0x7ffff,	// for 19
	0xfffff,	// for 20
	0x1fffff,	// for 21
	0x3fffff,	// for 22
	0x7fffff,	// for 23
	0xffffff,	// for 24
	0x1ffffff,	// for 25
	0x3ffffff,	// for 26
	0x7ffffff,	// for 27
	0xfffffff,	// for 28
	0x1fffffff,	// for 29
	0x3fffffff,	// for 30
	0x7fffffff,	// for 31
	0xffffffff,	// for 32
	0x1ffffffffULL,	// for 33
	0x3ffffffffULL,	// for 34
	0x7ffffffffULL,	// for 35
	0xfffffffffULL,	// for 36
	0x1fffffffffULL,	// for 37
	0x3fffffffffULL,	// for 38
	0x7fffffffffULL,	// for 39
	0xffffffffffULL,	// for 40
	0x1ffffffffffULL,	// for 41
	0x3ffffffffffULL,	// for 42
	0x7ffffffffffULL,	// for 43
	0xfffffffffffULL,	// for 44
	0x1fffffffffffULL,	// for 45
	0x3fffffffffffULL,	// for 46
	0x7fffffffffffULL,	// for 47
	0xffffffffffffULL,	// for 48
	0x1ffffffffffffULL,	// for 49
	0x3ffffffffffffULL,	// for 50
	0x7ffffffffffffULL,	// for 51
	0xfffffffffffffULL,	// for 52
	0x1fffffffffffffULL,	// for 53
	0x3fffffffffffffULL,	// for 54
	0x7fffffffffffffULL,	// for 55
	0xffffffffffffffULL,	// for 56
	0x1ffffffffffffffULL,	// for 57
	0x3ffffffffffffffULL,	// for 58
	0x7ffffffffffffffULL,	// for 59
	0xfffffffffffffffULL,	// for 60
	0x1fffffffffffffffULL,	// for 61
	0x3fffffffffffffffULL,	// for 62
	0x7fffffffffffffffULL,	// for 63
	0xffffffffffffffffULL	// for 64
};

inline
unsigned char psc_util_on_bits_8( unsigned char v )
{
   return( ON_BITS[ v ] );
}

inline
unsigned long long int psc_util_sel_mask( unsigned char num_bits )
{
   return( MASK_BITS[ num_bits ] );
}

} // namespace

#endif
