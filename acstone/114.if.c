/**
 * @file      114.if.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses unsigned short int ifs
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  unsigned short int a,b;
  unsigned char c_active_else,c_inactive_else;
  unsigned char c_active_then,c_inactive_then;

  c_active_then=0;
  c_active_else=0;
  c_inactive_then=0;
  c_inactive_else=0;

  a=0x0000;
  b=0xFFFF;
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


  a=0xFFFF;
  b=0xFFFE;
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


  a=0x0001;
  b=0x0002;
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

  a=0x000F;
  b=0x000F;
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


  a=0x0001;
  b=0xFFFF;
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


  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
