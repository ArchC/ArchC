/**
 * @file      breakpoints.H
 * @author    Daniel Cabrini Hauagge    <ra008388@ic.unicamp.br>
 *            Gustavo Sverzut Barbieri  <ra008849@ic.unicamp.br>
 *            Joao Victor Andrade Neves <ra008951@ic.unicamp.br>
 *            Rafael Dantas de Castro   <ra009663@ic.unicamp.br>
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     Breakpoint support
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * \note When modifing this file respect:
 * \li License
 * \li Previous author names. Add your own after current ones.
 * \li Coding style (basically emacs style)
 * \li Commenting style. This code use doxygen (http://www.doxygen.org)
 *     to be documented.
 *
 * \todo  This class should be replaced for standard STL element.
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
