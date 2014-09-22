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

#ifndef PSC_BASE_INTEGER_H
#define PSC_BASE_INTEGER_H

#include <systemc.h>

//#include "base/psc_objinfo.h"

#include "psc_objinfo.h"

using namespace sc_dt;
using namespace psc_power_base;


namespace psc_dt
{

// classes defined in this module
template <int W> class psc_int_base;
template <int W> class psc_uint_base;


// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : psc_int_base<W>
//
//  Base class for psc_int<W>
// ----------------------------------------------------------------------------

template <int W>
class psc_int_base
    : public psc_objinfo<W, sc_int<W> >
{
public:
   // constructor
   psc_int_base()
      : psc_objinfo<W, sc_int<W> >("psc_int")
      {}
};

// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : psc_uint_base<W>
//
//  Base class for psc_uint<W>
// ----------------------------------------------------------------------------

template <int W>
class psc_uint_base
   : public psc_objinfo<W, sc_uint<W> >
{
public:
   // constructor
   psc_uint_base()
      : psc_objinfo<W, sc_uint<W> >("psc_uint")
      {}
};


} // namespace psc_dt


#endif
