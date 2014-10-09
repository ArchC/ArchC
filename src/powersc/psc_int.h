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

  psc_int.h -- The psc_int<W> class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_INT_H
#define PSC_INT_H

#include <systemc.h>

//#include "base/psc_base_integer.h"
#include "psc_base_integer.h"

using namespace sc_dt;
using namespace psc_power_base;


namespace psc_dt
{

// classes defined in this module
template <int W> class psc_int;


// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : psc_int<W>
//
//  Power sc_int
// ----------------------------------------------------------------------------

template <int W>
class psc_int
    : public sc_int<W>, public psc_int_base<W>
{

   // support methods
   unsigned short int on_bits( const sc_int<W> & v ); 		// sets on '1' changed bits between v1 and v2
   inline sc_int<W> bit_diff(const sc_int<W> & v1, const sc_int<W> & v2);
   void update_toggle_count(const sc_int<W> & cur_val, const sc_int<W> & new_val);
	uint64 uint64value( const sc_int<W> & v ) const;
   
public:

    // constructors

    psc_int()
	{}

    psc_int( int_type v )
	: sc_int<W>( v )
	{}

    psc_int( const psc_int<W>& a )
	: sc_int<W>( a )
	{}

    psc_int( const sc_int<W>& a )
	{
	   sc_int<W>::operator = ( a );
	}

    psc_int( const sc_int_base& a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( const sc_int_subref_r& a )
	{ sc_int<W>::operator = ( a ); }

    template <class T>
    psc_int( const sc_generic_base<T>& a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( const sc_signed& a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( const sc_unsigned& a )
	{ sc_int<W>::operator = ( a ); }

#ifdef SC_INCLUDE_FX

    explicit psc_int( const sc_fxval& a )
	{ sc_int<W>::operator = ( a ); }

    explicit psc_int( const sc_fxval_fast& a )
	{ sc_int<W>::operator = ( a ); }

    explicit psc_int( const sc_fxnum& a )
	{ sc_int<W>::operator = ( a ); }

    explicit psc_int( const sc_fxnum_fast& a )
	{ sc_int<W>::operator = ( a ); }

#endif

    psc_int( const sc_bv_base& a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( const sc_lv_base& a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( const char* a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( unsigned long a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( long a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( unsigned int a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( int a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( uint64 a )
	{ sc_int<W>::operator = ( a ); }

    psc_int( double a )
	{ sc_int<W>::operator = ( a ); }


    // destructor
   ~psc_int()
   {
#ifdef DEBUG_POWER_L3
   cerr << "\t[psc_int]: Destroying " << PRINT_OBJ_STR << " TC=" << get_toggle_count() << endl;
#endif

      this->finish_pending(); // if some calculation is pending, update the statistics
   }

    // assignment operators

    psc_int<W>& operator = ( int_type v );
    psc_int<W>& operator = ( const sc_int_base& a );
    psc_int<W>& operator = ( const sc_int_subref_r& a );
    psc_int<W>& operator = ( const sc_int<W>& a );
    psc_int<W>& operator = ( const psc_int<W>& a );
    template <class T>
    psc_int<W>& operator = ( const sc_generic_base<T>& a );
    psc_int<W>& operator = ( const sc_signed& a );
    psc_int<W>& operator = ( const sc_unsigned& a );
    psc_int<W>& operator = ( const sc_bv_base& a );
    psc_int<W>& operator = ( const sc_lv_base& a );
    psc_int<W>& operator = ( const char* a );
    psc_int<W>& operator = ( unsigned long a );
    psc_int<W>& operator = ( long a );
    psc_int<W>& operator = ( unsigned int a );
    psc_int<W>& operator = ( int a );
    psc_int<W>& operator = ( uint64 a );
    psc_int<W>& operator = ( double a );

#ifdef SC_INCLUDE_FX
// ATTENTION: these operators are currently ignored by PowerSC.
// If they appear on your design, the toggle count regarding them will not be
// taken into account

    psc_int<W>& operator = ( const sc_fxval& a )
	{ sc_int<W>::operator = ( a ); return *this; }

    psc_int<W>& operator = ( const sc_fxval_fast& a )
	{ sc_int<W>::operator = ( a ); return *this; }

    psc_int<W>& operator = ( const sc_fxnum& a )
	{ sc_int<W>::operator = ( a ); return *this; }

    psc_int<W>& operator = ( const sc_fxnum_fast& a )
	{ sc_int<W>::operator = ( a ); return *this; }

#endif

    // arithmetic assignment operators

   psc_int<W>& operator += ( int_type v );
   psc_int<W>& operator -= ( int_type v );
   psc_int<W>& operator *= ( int_type v );
   psc_int<W>& operator /= ( int_type v );
   psc_int<W>& operator %= ( int_type v );

};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII


// ----------------------------------------------------------------------------
// Specialized versions of the virtual methods (inherited from psc_objinfo):
//  - on_bits
//  - bit_diff
//  - update_toggle_count
// ----------------------------------------------------------------------------
template <int W>
unsigned short int psc_int<W>::on_bits( const sc_int<W> & v )
{
#ifdef DEBUG_POWER_L2
   if ( this->m_len == 0 ) cerr << "Warning: m_len value is zero" << endl;
#endif
   
   unsigned char c0, c1, c2, c3, c4, c5, c6, c7;
   long long int _v = v & psc_util_sel_mask( this->m_len ); // masks only the necessary bits
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

	if ( psc_int<W>::must_update() && count > 0 )
		for ( int i = 0 ; i < W ; i++ )
			if ( v[i] != 0 )
				psc_objinfo<W, sc_int<W> >::inc_bit_toggle_count( i, 1 );

   return(count);
}

// sets on '1' changed bits between v1 and v2
template <int W>
inline sc_int<W> psc_int<W>::bit_diff(const sc_int<W> & v1, const sc_int<W> & v2)
{ 
   sc_int<W> tmp;
   long long int _v1 = v1, _v2 = v2;
 
   tmp = ( _v1 ^ _v2 );
   return( tmp ); 
}

template <int W>
void psc_int<W>::update_toggle_count(const sc_int<W> & cur_val, const sc_int<W> & new_val)
{
   psc_objinfo<W, sc_int<W> >::update_toggle_count(cur_val, new_val);
}

template <int W>
uint64 psc_int<W>::uint64value( const sc_int<W> & v ) const
{
	uint64 value = v.to_uint64();
	return( value );
}

// ----------------------------------------------------------------------------
// Assignment Operators
// ----------------------------------------------------------------------------
template <int W>
psc_int<W>& psc_int<W>::operator = ( int_type v )
{ 
   update_toggle_count(sc_int<W>(*this), v);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (int_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( v ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const sc_int_base& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_int_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const sc_int_subref_r& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_int_subref_r " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const sc_int<W>& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_int " << a.to_int()
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   this->m_val = a.m_val; 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const psc_int<W>& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (psc_int " << a.to_int() 
   << ") TC=" << dec << get_toggle_count() << endl;
#endif

   this->m_val = a.m_val; 
   return *this; 
}

template <int W>
template <class T>
psc_int<W>& psc_int<W>::operator = ( const sc_generic_base<T>& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_generic_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const sc_signed& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_signed " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const sc_unsigned& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_unsigned " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const sc_bv_base& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_bv_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const sc_lv_base& a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_lv_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( const char* a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (char* " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( unsigned long a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (unsigned long " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( long a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (long " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( unsigned int a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (unsigned int " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( int a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (int " << a 
   << ") TC:" << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( uint64 a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (uint64 " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator = ( double a )
{ 
   update_toggle_count(sc_int<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (double " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator = ( a ); 
   return *this; 
}


// ----------------------------------------------------------------------------
// Arithmetic Assignment Operators
// ----------------------------------------------------------------------------
template <int W>
psc_int<W>& psc_int<W>::operator += ( int_type v )
{ 
   update_toggle_count( sc_int<W>(*this), ((*this) + v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator += (int_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator += ( v ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator -= ( int_type v )
{ 
   update_toggle_count( sc_int<W>(*this), ((*this) - v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator -= (int_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator -= ( v ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator *= ( int_type v )
{ 
   update_toggle_count( sc_int<W>(*this), ((*this) * v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator *= (int_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator *= ( v ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator /= ( int_type v )
{ 
   update_toggle_count( sc_int<W>(*this), ((*this) / v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator /= (int_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator /= ( v ); 
   return *this; 
}

template <int W>
psc_int<W>& psc_int<W>::operator %= ( int_type v )
{ 
   update_toggle_count( sc_int<W>(*this), ((*this) % v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator %= (int_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_int<W>::operator %= ( v ); 
   return *this; 
}


} // namespace psc_dt


#endif
