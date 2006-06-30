/**
 * @file      117.if.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main function that uses signed long long int ifs
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  signed long long int a,b;
  unsigned char c_active_else,c_inactive_else;
  unsigned char c_active_then,c_inactive_then;

  c_active_then=0;
  c_active_else=0;
  c_inactive_then=0;
  c_inactive_else=0;

  a=0x0000000000000000LL;
  b=0xFFFFFFFFFFFFFFFFLL;
  if(a > b) {
    c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */
  }
  if(a < b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a == b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a >= b) {
    c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */
  }
  if(a <= b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  /* Before, c_active_then must be 2 */ c_active_then=0;
  /* Before, c_active_else must be 3 */ c_active_else=0;
  /* Before, c_inactive_then must be 0 */ c_inactive_then=0;
  /* Before, c_inactive_else must be 0 */ c_inactive_else=0;


  a=0xFFFFFFFFFFFFFFFFLL;
  b=0xFFFFFFFFFFFFFFFELL;
  if(a > b) {
    c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */    
  }
  if(a < b) {
    c_inactive_then++; /* $=FAILED! */    
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a == b) {
    c_inactive_then++; /* $=FAILED! */    
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a >= b) {
   c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */    
  }
  if(a <= b) {
    c_inactive_then++; /* $=FAILED! */    
  }
  else {
    c_active_else++; /* $=OK! */
  }
  /* Before, c_active_then must be 2 */ c_active_then=0;
  /* Before, c_active_else must be 3 */ c_active_else=0;
  /* Before, c_inactive_then must be 0 */ c_inactive_then=0;
  /* Before, c_inactive_else must be 0 */ c_inactive_else=0;


  a=0x0000000000000001LL;
  b=0x0000000000000002LL;
  if(a > b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a < b) {
    c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */
  }
  if(a == b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a >= b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a <= b) {
    c_active_then++; /* $=OK! */
  }
  else {
   c_inactive_else++; /* $=FAILED! */
  }
  /* Before, c_active_then must be 2 */ c_active_then=0;
  /* Before, c_active_else must be 3 */ c_active_else=0;
  /* Before, c_inactive_then must be 0 */ c_inactive_then=0;
  /* Before, c_inactive_else must be 0 */ c_inactive_else=0;

  a=0x000000000000000FLL;
  b=0x000000000000000FLL;
  if(a > b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a < b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a == b) {
    c_active_then++; /* $=OK! */
  }
  else {
   c_inactive_else++; /* $=FAILED! */
  }
  if(a >= b) {
    c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */   
  }
  if(a <= b) {
    c_active_then++; /* $=OK! */
  }
  else {
   c_inactive_else++; /* $=FAILED! */
  }
  /* Before, c_active_then must be 3 */ c_active_then=0;
  /* Before, c_active_else must be 2 */ c_active_else=0;
  /* Before, c_inactive_then must be 0 */ c_inactive_then=0;
  /* Before, c_inactive_else must be 0 */ c_inactive_else=0;


  a=0x0000000000000001LL;
  b=0xFFFFFFFFFFFFFFFFLL;
  if(a > b) {
    c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */
  }
  if(a < b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a == b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  if(a >= b) {
    c_active_then++; /* $=OK! */
  }
  else {
    c_inactive_else++; /* $=FAILED! */
  }
  if(a <= b) {
    c_inactive_then++; /* $=FAILED! */
  }
  else {
    c_active_else++; /* $=OK! */
  }
  /* Before, c_active_then must be 2 */ c_active_then=0;
  /* Before, c_active_else must be 3 */ c_active_else=0;
  /* Before, c_inactive_then must be 0 */ c_inactive_then=0;
  /* Before, c_inactive_else must be 0 */ c_inactive_else=0;


  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
