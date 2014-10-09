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

  psc_lv.h -- The psc_lv<W> class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_LV_H
#define PSC_LV_H

#include <systemc.h>

//#include "base/psc_objinfo.h"
#include "psc_objinfo.h"

using namespace sc_dt;
using namespace psc_power_base;

namespace psc_dt {

// classes defined in this module
template <int W> class psc_lv;


// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : psc_lv<W>
//
//  Custom size logic vector class.
// ----------------------------------------------------------------------------
template <int W>
class psc_lv : public sc_lv<W>, public psc_objinfo<W, sc_lv<W> >
{

   // support methods
   
   unsigned short int on_bits( const sc_lv<W> & v ); 
   inline sc_lv<W> bit_diff(const sc_lv<W> & v1, const sc_lv<W> & v2); 
   void update_toggle_count(const sc_lv<W> & cur_val, const sc_lv<W> & new_val);

public:

   // constructors

   psc_lv()
   : sc_lv<W>(), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   explicit psc_lv( const sc_logic& init_value )
   : sc_lv<W>( init_value ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   explicit psc_lv( bool init_value )
   : sc_lv<W>( sc_logic( init_value ) ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   explicit psc_lv( char init_value )
   : sc_lv<W>( sc_logic( init_value ) ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const char* a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const bool* a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const sc_logic* a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const sc_unsigned& a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const sc_signed& a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const sc_uint_base& a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const sc_int_base& a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( unsigned long a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( long a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( unsigned int a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( int a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( uint64 a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( int64 a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   template <class X>
   psc_lv( const sc_proxy<X>& a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const sc_lv<W>& a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}

   psc_lv( const psc_lv<W>& a )
   : sc_lv<W>( a ), psc_objinfo<W, sc_lv<W> >("psc_lv")
   {}



   // destructor

   ~psc_lv()
   {
#ifdef DEBUG_POWER_L3
      cerr << "\t[psc_lv]: Destroying " << PRINT_OBJ_STR << " TC=" << get_toggle_count() << endl;
#endif

      this->finish_pending(); // if some calculation is pending, update the statistics before destroying the object
   }

   // assignment operators

   template <class X>
   psc_lv<W>& operator = ( const sc_proxy<X>& a );
   psc_lv<W>& operator = ( const sc_lv<W>& a );
   psc_lv<W>& operator = ( const psc_lv<W>& a );
   psc_lv<W>& operator = ( const char* a );
   psc_lv<W>& operator = ( const bool* a );
   psc_lv<W>& operator = ( const sc_logic* a );
   psc_lv<W>& operator = ( const sc_unsigned& a );
   psc_lv<W>& operator = ( const sc_signed& a );
   psc_lv<W>& operator = ( const sc_uint_base& a );
   psc_lv<W>& operator = ( const sc_int_base& a );
   psc_lv<W>& operator = ( unsigned long a );
   psc_lv<W>& operator = ( long a );
   psc_lv<W>& operator = ( unsigned int a );
   psc_lv<W>& operator = ( int a );
   psc_lv<W>& operator = ( uint64 a );
   psc_lv<W>& operator = ( int64 a );

};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII


// ----------------------------------------------------------------------------
// Specialized versions of the virtual methods (inherited from psc_objinfo):
//  - on_bits
//  - bit_diff
//  - update_toggle_count
// ----------------------------------------------------------------------------
template <int W>
unsigned short int psc_lv<W>::on_bits( const sc_lv<W> & v )
{
   unsigned short int count = 0;

   for ( unsigned short int i = 0; i < this->m_len; i++ ) {
      count += v.bit(i).to_bool();
   }

	if ( psc_lv<W>::must_update() && count > 0 )
		for ( int i = 0 ; i < W ; i++ )
			if ( v[i] != 0 )
				psc_objinfo<W, sc_lv<W> >::inc_bit_toggle_count( i, 1 );
   
   return(count);
}

template <int W>
inline sc_lv<W> psc_lv<W>::bit_diff( const sc_lv<W> & v1, const sc_lv<W> & v2 )
{
   sc_lv<W> tmp;
   long long int _v1 = v1.to_long(), _v2 = v2.to_long();

   tmp = _v1 ^ _v2; 
   return( tmp );
}

template <int W>
void psc_lv<W>::update_toggle_count( const sc_lv<W> & cur_val, const sc_lv<W> & new_val )
{
   psc_objinfo<W, sc_lv<W> >::update_toggle_count( cur_val, new_val );
}


// ----------------------------------------------------------------------------
// Assignment Operators
// ----------------------------------------------------------------------------

template <int W>
template <class X>
psc_lv<W>& psc_lv<W>::operator = ( const sc_proxy<X>& a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_proxy " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const sc_lv<W>& a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_lv " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const psc_lv<W>& a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (psc_lv " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const char* a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (char* " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const bool* a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (bool* " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const sc_logic* a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_logic* " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const sc_unsigned& a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_unsigned " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const sc_signed& a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_signed " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const sc_uint_base& a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_uint_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( const sc_int_base& a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_int_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( unsigned long a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (unsigned long " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( long a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (long " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( unsigned int a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (unsigned int " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( int a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (int " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( uint64 a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (uint64 " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_lv<W>& psc_lv<W>::operator = ( int64 a )
{ 
   update_toggle_count( sc_lv<W>(*this), a );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (int64 " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_lv<W>::operator = ( a ); 
   return *this; 
}


} // namespace psc_dt

#endif
