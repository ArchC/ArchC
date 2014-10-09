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

  psc_bit.h -- The psc_bit class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_BIT_H
#define PSC_BIT_H

#include <systemc>

//#include "base/psc_base_integer.h"
#include "psc_base_integer.h"

using sc_dt::sc_bit;
using namespace psc_power_base;

namespace psc_dt {

// classes defined in this module
class psc_bit;

// ----------------------------------------------------------------------------
//  CLASS : psc_bit
//
//  The psc_bit class.
// ----------------------------------------------------------------------------
class psc_bit : public sc_bit, public psc_objinfo<1, sc_bit>
{
	
	// support methods

   unsigned short int on_bits( const sc_bit & v );
   inline sc_bit bit_diff(const sc_bit & v1, const sc_bit & v2); // sets on '1' changed bits between v1 and v2
   void update_toggle_count(const sc_bit & cur_val, const sc_bit & new_val);
	uint64 uint64value( const sc_bit & v ) const;
	
public:

	// constructors
	psc_bit()
		: psc_objinfo<1, sc_bit>("psc_bit")
	{
	}

	explicit psc_bit( bool a )
		: sc_bit( a ), psc_objinfo<1, sc_bit>("psc_bit")
		{}

   explicit psc_bit( int a )
		: sc_bit( a ), psc_objinfo<1, sc_bit>("psc_bit")
		{}

	explicit psc_bit( char a )
		: sc_bit( a ), psc_objinfo<1, sc_bit>("psc_bit")
	{}

	psc_bit( const sc_bit& a )
		: sc_bit( a ), psc_objinfo<1, sc_bit>("psc_bit")
	{}

    // copy constructor
	psc_bit( const psc_bit& a )
		: sc_bit( a ), psc_objinfo<1, sc_bit>("psc_bit")
	{}

	// destructor
	~psc_bit()
	{
#ifdef DEBUG_POWER_L3
		cerr << "\t[psc_bit]: Destroying " << PRINT_OBJ_STR << " TC=" << get_toggle_count() << endl;
#endif

		finish_pending(); // if some calculation is pending, update the statistics
	}

    // assignment operators
	
	psc_bit& operator = ( const psc_bit& b )
	{ 
      sc_bit tmp = b;
		update_toggle_count(sc_bit(*this), tmp);

#ifdef DEBUG_POWER_L1
		cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (psc_bit " << bool(b) 
			<< ") TC: " << dec << get_toggle_count() << endl;
#endif

		sc_bit::operator = ( tmp ); 
		return *this;
	}
	
	psc_bit& operator = ( const sc_bit& b )
	{ 
		update_toggle_count(sc_bit(*this), b);

#ifdef DEBUG_POWER_L1
		cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_bit " << bool(b)
			<< ") TC: " << dec << get_toggle_count() << endl;
#endif

		sc_bit::operator = ( b ); 
		return *this;
	}

   psc_bit& operator = ( int b )
	{ 
		update_toggle_count(sc_bit(*this), sc_bit(b));

#ifdef DEBUG_POWER_L1
		cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (int " << b 
			<< ") TC: " << dec << get_toggle_count() << endl;
#endif

		sc_bit::operator = ( b ); 
		return *this;
	}
   
   psc_bit& operator = ( bool b )
	{ 
		update_toggle_count(sc_bit(*this), sc_bit(b));

#ifdef DEBUG_POWER_L1
		cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (bool " << b
			<< ") TC: " << dec << get_toggle_count() << endl;
#endif

		sc_bit::operator = ( b ); 
		return *this;
	}

   psc_bit& operator = ( char b )
	{ 
		update_toggle_count(sc_bit(*this), sc_bit(b));

#ifdef DEBUG_POWER_L1
		cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (char " << b 
			<< ") TC: " << dec << get_toggle_count() << endl;
#endif
		
		return ( *this = sc_bit( b ) ); 
	}

   psc_bit& operator = ( const sc_logic& b );  // non-VSIA

};


} // namespace psc_dt

#endif
