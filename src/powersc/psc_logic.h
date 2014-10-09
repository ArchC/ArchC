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

  psc_logic.h -- The psc_logic class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_LOGIC_H
#define PSC_LOGIC_H

#include <systemc.h>

//#include "base/psc_objinfo.h"
#include "psc_objinfo.h"

using sc_dt::sc_logic;
using sc_dt::sc_logic_value_t;
using namespace psc_power_base;

namespace psc_dt {


// classes defined in this module
class psc_logic;



// ----------------------------------------------------------------------------
//  CLASS : psc_logic
//
//  The psc_logic class.
// ----------------------------------------------------------------------------
class psc_logic : public sc_logic, public psc_objinfo<1, sc_logic>
{
	// support methods

   unsigned short int on_bits( const sc_logic & v );
   inline sc_logic bit_diff( const sc_logic & v1, const sc_logic & v2 ); // sets on '1' changed bits between v1 and v2
   void update_toggle_count( const sc_logic & cur_val, const sc_logic & new_val );
	uint64 uint64value( const sc_logic & v ) const;
   
public:


   // constructors

   psc_logic()
   : psc_objinfo<1, sc_logic>("psc_logic"),
      m_bLastUndefToBool(true)
   {}

   psc_logic( const sc_logic& a )
   : sc_logic( a ), psc_objinfo<1, sc_logic>("psc_logic"),
      m_bLastUndefToBool(true)
   {}

   psc_logic( sc_logic_value_t v )
   : sc_logic( v ), psc_objinfo<1, sc_logic>("psc_logic"),
      m_bLastUndefToBool(true)
   {}

   explicit psc_logic( bool a )
   : sc_logic( a ), psc_objinfo<1, sc_logic>("psc_logic"),
      m_bLastUndefToBool(true)
   {}

   explicit psc_logic( char a )
   : sc_logic( a ), psc_objinfo<1, sc_logic>("psc_logic"),
      m_bLastUndefToBool(true)
   {}

   explicit psc_logic( int a )
   : sc_logic( a ), psc_objinfo<1, sc_logic>("psc_logic"),
      m_bLastUndefToBool(true)
   {}

   explicit psc_logic( const sc_bit& a )
   : sc_logic( a ), psc_objinfo<1, sc_logic>("psc_logic"),
      m_bLastUndefToBool(true)
   {}


   // destructor

   ~psc_logic()
   {
#ifdef DEBUG_POWER_L3
		cerr << "\t[psc_logic]: Destroying " << PRINT_OBJ_STR << " TC=" << get_toggle_count() << endl;
#endif

		finish_pending(); // if some calculation is pending, update the statistics
   }

   // assignment operators

   psc_logic& operator = ( const psc_logic& a );
   psc_logic& operator = ( const sc_logic& a );
   psc_logic& operator = ( sc_logic_value_t v );
   psc_logic& operator = ( bool a );
   psc_logic& operator = ( char a );
   psc_logic& operator = ( int a );
   psc_logic& operator = ( const sc_bit& a );

   // implicit conversion operators
#if 0
   operator my_logic () const
   { return sc_logic( *this ); }
#endif

protected:

   bool m_bLastUndefToBool;
};

} // namespace psc_dt

#endif
