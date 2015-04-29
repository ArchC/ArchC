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

/****************************************************************************

  psc_macromodel.h -- Macromodeling related classes

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2006.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 ***************************************************************************/
#ifndef PSC_MACROMODEL_H
#define PSC_MACROMODEL_H

#include <iostream>
#include <string>
#include <vector>
#include <list>
//#include <ext/hash_map>
#include <unordered_map>

//#include "base/psc_obj_rep.h"
#include "psc_obj_rep.h"

using namespace std;
//using namespace __gnu_cxx;

namespace psc_power_base
{

// classes defined within this file
class psc_macromodel;
class psc_macromodel_parms;

// --------------------------------------------------------------------------
//  CLASS : psc_macromodel_parms
// --------------------------------------------------------------------------
class psc_macromodel_parms {
	public:

		virtual ~psc_macromodel_parms()
		{}
};

// --------------------------------------------------------------------------
//  CLASS : psc_macromodel
//
// Base class for modeling power at different levels of abstraction
//
// Currently supported:
//  - ...
//  - RTL
// --------------------------------------------------------------------------
class psc_macromodel {
	public:

		psc_macromodel()
		{
			init_power_map();
		}

		virtual ~psc_macromodel()
		{}

		virtual void init_power_map();
		virtual double get_power( const psc_macromodel_parms & ) = 0;

	protected:

		// typedef hash_map<string, double, hash<string>, eqstr> PowerMap;
		typedef unordered_map<string, double, hash<string>, eqstr> PowerMap;
		typedef enum { DOWN, UP } direction_t;

		PowerMap rtl_model;
};

} // namespace psc_power_base

#endif
