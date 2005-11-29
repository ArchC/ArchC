/** \file breakpoints.H
 * Breakpoint support
 *
 *    This is just the header file. Look for documentation in breakpoints.cpp 
 * or in the doxygen auto-generated files.
 *
 ******************************************************************************
 *
 * \note When modifing this file respect:
 * \li License
 * \li Previous author names. Add your own after current ones.
 * \li Coding style (basically emacs style)
 * \li Commenting style. This code use doxygen (http://www.doxygen.org)
 *     to be documented.
 *
 *
 * \todo  This class should be replaced for standard STL element.
 *
 ******************************************************************************
 *                                                                             
 * LICENSE:
 *    GNU GPL --- General Public License, version 2.0 or greater.
 *    See archc/COPYING for more information.
 *
 * \author Daniel Cabrini Hauagge    <ra008388@ic.unicamp.br>
 * \author Gustavo Sverzut Barbieri  <ra008849@ic.unicamp.br>
 * \author Joao Victor Andrade Neves <ra008951@ic.unicamp.br>
 * \author Rafael Dantas de Castro   <ra009663@ic.unicamp.br>
 *
 ******************************************************************************
 */
#ifndef _BREAKPOINTS_H_
#define _BREAKPOINTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <iostream>

using namespace std;

/** \class Breakpoints
 * Breakpoint data structure.
 *
 * Keep breakpoints in crescent order, can check if a given breakpoint exists.
 * It's fixed size.
 */
class Breakpoints {
public:
  Breakpoints(int quant);
  ~Breakpoints();
  int add(unsigned int address);
  int exists(unsigned int address);
  int remove(unsigned int address);

protected:
  unsigned int *bp; /**< breakpoint array */
  int quantMax;     /**< Maximum supported breakpoints, that is, the parameter given to constructor */
  int quant;        /**< current count */
};
#endif /* _BREAKPOINTS_H_ */
