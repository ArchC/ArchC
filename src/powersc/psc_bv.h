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

  psc_bv.h -- The psc_bv<W> class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_BV_H
#define PSC_BV_H

#include <systemc.h>

//#include "base/psc_objinfo.h"
#include "psc_objinfo.h"

using namespace sc_dt;
using namespace psc_power_base;

namespace psc_dt {

// classes defined in this module
template <int W> class psc_bv;

// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : psc_bv<W>
//
//  Custom size bit vector class.
// ----------------------------------------------------------------------------
template <int W>
class psc_bv : public sc_bv<W>, public psc_objinfo<W, sc_bv<W> >
{

   // support methods
   
   unsigned short int on_bits( const sc_bv<W> & v ); 
   inline sc_bv<W> bit_diff(const sc_bv<W> & v1, const sc_bv<W> & v2); 
   void update_toggle_count(const sc_bv<W> & cur_val, const sc_bv<W> & new_val);
	uint64 uint64value( const sc_bv<W> & v ) const;

public:

   // constructors

   psc_bv() 
      : sc_bv<W>(), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   explicit psc_bv( bool init_value )
   : sc_bv<W>( init_value ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   explicit psc_bv( char init_value )
   : sc_bv<W>( init_value ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const char* a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const bool* a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const sc_logic* a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const sc_unsigned& a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const sc_signed& a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const sc_uint_base& a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const sc_int_base& a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( unsigned long a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( long a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( unsigned int a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( int a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( uint64 a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( int64 a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const sc_bv<W>& a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   psc_bv( const psc_bv<W>& a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   template <class X>
   psc_bv( const sc_proxy<X>& a )
   : sc_bv<W>( a ), psc_objinfo<W, sc_bv<W> >("psc_bv")
   {}

   // destructor
   ~psc_bv()
   {
#ifdef DEBUG_POWER_L3
      cerr << "\t[psc_bv]: Destroying " << PRINT_OBJ_STR << " TC=" << get_toggle_count() << endl;
#endif

      this->finish_pending(); // if some calculation is pending, update the statistics before destroying the object
   }


   // assignment operators
   
   template <class X>
   psc_bv<W>& operator = ( const sc_proxy<X>& a );
   psc_bv<W>& operator = ( const sc_bv<W>& a );
   psc_bv<W>& operator = ( const psc_bv<W>& a );
   psc_bv<W>& operator = ( const char* a );
   psc_bv<W>& operator = ( const bool* a );
   psc_bv<W>& operator = ( const sc_logic* a );
   psc_bv<W>& operator = ( const sc_unsigned& a );
   psc_bv<W>& operator = ( const sc_signed& a );
   psc_bv<W>& operator = ( const sc_uint_base& a );
   psc_bv<W>& operator = ( const sc_int_base& a );
   psc_bv<W>& operator = ( unsigned long a );
   psc_bv<W>& operator = ( long a );
   psc_bv<W>& operator = ( unsigned int a );
   psc_bv<W>& operator = ( int a );
   psc_bv<W>& operator = ( uint64 a );
   psc_bv<W>& operator = ( int64 a );

};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII


// ----------------------------------------------------------------------------
// Specialized versions of the virtual methods (inherited from psc_objinfo):
//  - on_bits
//  - bit_diff
//  - update_toggle_count
// ----------------------------------------------------------------------------
template <int W>
unsigned short int psc_bv<W>::on_bits( const sc_bv<W> & v )
{
#ifdef DEBUG_POWER_L2
   if ( this->m_len == 0 ) cerr << "Warning: m_len value is zero" << endl;
#endif
   
   unsigned char c0, c1, c2, c3, c4, c5, c6, c7;
   unsigned long long int _v = v.to_ulong();
   unsigned short int count;
   
   c0 = psc_util_on_bits_8( 0xFF & _v );
   c1 = psc_util_on_bits_8( 0xFF & (_v >> 8) );
   c2 = psc_util_on_bits_8( 0xFF & (_v >> 16) );
   c3 = psc_util_on_bits_8( 0xFF & (_v >> 24) );

   count = c0 + c1 + c2 + c3;
   
   if ( this->m_len > 32 ) {
      c4 = psc_util_on_bits_8( 0xFF & (_v >> 32) );
      c5 = psc_util_on_bits_8( 0xFF & (_v >> 40) );
      c6 = psc_util_on_bits_8( 0xFF & (_v >> 48) );
      c7 = psc_util_on_bits_8( 0xFF & (_v >> 56) );
      count += (c4 + c5 + c6 + c7);
   }

	if ( psc_bv<W>::must_update() && count > 0 )
		for ( int i = 0 ; i < W ; i++ )
			if ( v[i] != 0 )
				psc_objinfo<W, sc_bv<W> >::inc_bit_toggle_count( i, 1 );
   
   return(count);
}

template <int W>
sc_bv<W> psc_bv<W>::bit_diff(const sc_bv<W> & v1, const sc_bv<W> & v2)
{
   sc_bv<W> tmp;
   long long int _v1 = v1.to_long(), _v2 = v2.to_long();

   tmp = ( _v1 ^ _v2 );
   return( tmp );
}

template <int W>
void psc_bv<W>::update_toggle_count(const sc_bv<W> & cur_val, const sc_bv<W> & new_val)
{
   psc_objinfo<W, sc_bv<W> >::update_toggle_count(cur_val, new_val);
}

template <int W>
uint64 psc_bv<W>::uint64value( const sc_bv<W> & v ) const
{
	uint64 value = v.to_uint64();
	return( value );
}

// ----------------------------------------------------------------------------
// Assignment Operators
// ----------------------------------------------------------------------------

template <int W>
template <class X>
psc_bv<W>& psc_bv<W>::operator = ( const sc_proxy<X>& a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_proxy " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const sc_bv<W>& a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_bv " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const psc_bv<W>& a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (psc_bv " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}


template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const char* a )
{ 
   sc_bv<W> tmp( a );
   update_toggle_count(sc_bv<W>(*this), tmp);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (char* " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( tmp ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const bool* a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   sc_bv<W> tmp_( a );
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (bool* " << tmp_ 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const sc_logic* a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   sc_bv<W> tmp_( a );
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_logic* " << tmp_ 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const sc_unsigned& a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_unsigned " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const sc_signed& a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_signed " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const sc_uint_base& a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_uint_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( const sc_int_base& a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_int_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( unsigned long a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (unsigned long " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( long a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (long " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( unsigned int a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (unsigned int " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( int a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (int " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; }

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( uint64 a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (uint64 " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_bv<W>& psc_bv<W>::operator = ( int64 a )
{ 
   update_toggle_count(sc_bv<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (int64 " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_bv<W>::operator = ( a ); 
   return *this; 
}
  
} // namespace psc_dt

#endif
