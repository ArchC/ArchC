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

  psc_uint.h -- The psc_uint<W> class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_UINT_H
#define PSC_UINT_H

#include <systemc.h>

//#include "base/psc_base_integer.h"
//#include "utils/psc_tables.h"

#include "psc_base_integer.h"
#include "psc_tables.h"

using namespace sc_dt;
using namespace psc_power_base;
using namespace psc_util;


namespace psc_dt
{

// classes defined within this module
template <int W> class psc_uint;



// ----------------------------------------------------------------------------
//  CLASS TEMPLATE : psc_uint<W>
//
// Power sc_uint
// ----------------------------------------------------------------------------

template <int W>
class psc_uint
   : public sc_uint<W>, public psc_uint_base<W>
{

   // support methods

   unsigned short int on_bits( const sc_uint<W> & v );
   inline sc_uint<W> bit_diff( const sc_uint<W> & v1, const sc_uint<W> & v2 ); // sets on '1' changed bits between v1 and v2
   void update_toggle_count( const sc_uint<W> & cur_val, const sc_uint<W> & new_val );
	uint64 uint64value( const sc_uint<W> & v ) const;

public:

   // constructors
   psc_uint() 
      {}

   psc_uint( uint_type v ) 
      : sc_uint<W>( v )
      {}

   psc_uint( const psc_uint<W>& a ) 
      : sc_uint<W>( a )
      {}

	 psc_uint( const sc_uint_base& a )
		{ 
			sc_uint<W>::operator = ( a ); 
		}

   psc_uint( const sc_uint<W>& a )
      { 
         sc_uint<W>::operator = ( a ); 
      }

   psc_uint( const sc_uint_subref_r& a )
      { 
	 sc_uint<W>::operator = ( a ); 
      }

   template <class T>
   psc_uint( const sc_generic_base<T>& a )
      { 
	 sc_uint<W>::operator = ( a ); 
      }

   psc_uint( const sc_signed& a )
      { 
	 sc_uint<W>::operator = ( a ); 
      }

   psc_uint( const sc_unsigned& a ) 
      { 
	 sc_uint<W>::operator = ( a ); 
      }

#ifdef SC_INCLUDE_FX

   explicit psc_uint( const sc_fxval& a )
      { sc_uint<W>::operator = ( a ); }

   explicit psc_uint( const sc_fxval_fast& a ) 
      { sc_uint<W>::operator = ( a ); }

   explicit psc_uint( const sc_fxnum& a )
      { sc_uint<W>::operator = ( a ); }

   explicit psc_uint( const sc_fxnum_fast& a )
      { sc_uint<W>::operator = ( a ); }

#endif

   psc_uint( const sc_bv_base& a ) 
      { sc_uint<W>::operator = ( a ); }

   psc_uint( const sc_lv_base& a ) 
      { sc_uint<W>::operator = ( a ); }

   psc_uint( const char* a ) 
      { sc_uint<W>::operator = ( a ); }

   psc_uint( unsigned long a ) 
      { sc_uint<W>::operator = ( a ); }

   psc_uint( long a )
      { sc_uint<W>::operator = ( a ); }

   psc_uint( unsigned int a )
      { sc_uint<W>::operator = ( a ); }

   psc_uint( int a )
      { sc_uint<W>::operator = ( a ); }

   psc_uint( int64 a )
      { sc_uint<W>::operator = ( a ); }

   psc_uint( double a )
      { sc_uint<W>::operator = ( a ); }

   
   // destructor
   ~psc_uint()
   {
#ifdef DEBUG_POWER_L3
      cerr << "\t[psc_uint]: Destroying " << PRINT_OBJ_STR << " TC=" << get_toggle_count() << endl;
#endif

      this->finish_pending(); // if some calculation is pending, update the statistics
   }


   // assignment operators

   psc_uint<W>& operator = ( uint_type v );
   psc_uint<W>& operator = ( const sc_uint_base& a );
   psc_uint<W>& operator = ( const sc_uint_subref_r& a );
   psc_uint<W>& operator = ( const sc_uint<W>& a );
   psc_uint<W>& operator = ( const psc_uint<W>& a );
   template <class T>
   psc_uint<W>& operator = ( const sc_generic_base<T>& a );
   psc_uint<W>& operator = ( const sc_signed& a );
   psc_uint<W>& operator = ( const sc_unsigned& a );
   psc_uint<W>& operator = ( const sc_bv_base& a );
   psc_uint<W>& operator = ( const sc_lv_base& a );
   psc_uint<W>& operator = ( const char* a );
   psc_uint<W>& operator = ( unsigned long a );
   psc_uint<W>& operator = ( unsigned int a );
   psc_uint<W>& operator = ( long a );
   psc_uint<W>& operator = ( int a );
   psc_uint<W>& operator = ( int64 a );
   psc_uint<W>& operator = ( double a );

#ifdef SC_INCLUDE_FX
// ATTENTION: these operators are currently ignored by PowerSC.
// If they appear on your design, the toggle count regarding them will not be
// taken into account

   psc_uint<W>& operator = ( const sc_fxval& a )
      { sc_uint<W>::operator = ( a ); return *this; }

   psc_uint<W>& operator = ( const sc_fxval_fast& a )
      { sc_uint<W>::operator = ( a ); return *this; }

   psc_uint<W>& operator = ( const sc_fxnum& a )
      { sc_uint<W>::operator = ( a ); return *this; }

   psc_uint<W>& operator = ( const sc_fxnum_fast& a )
      { sc_uint<W>::operator = ( a ); return *this; }

#endif


   // arithmetic assignment operators

   psc_uint<W>& operator += ( uint_type v );
   psc_uint<W>& operator -= ( uint_type v );
   psc_uint<W>& operator *= ( uint_type v );
   psc_uint<W>& operator /= ( uint_type v );
   psc_uint<W>& operator %= ( uint_type v );


   // bitwise assignment operators

   psc_uint<W>& operator &= ( uint_type v )
      { sc_uint<W>::operator &= ( v ); return *this; }

   psc_uint<W>& operator |= ( uint_type v )
      { sc_uint<W>::operator |= ( v ); return *this; }

   psc_uint<W>& operator ^= ( uint_type v )
      { sc_uint<W>::operator ^= ( v ); return *this; }


   psc_uint<W>& operator <<= ( uint_type v )
      { sc_uint<W>::operator <<= ( v ); return *this; }

   psc_uint<W>& operator >>= ( uint_type v )
      { sc_uint<W>::operator >>= ( v ); return *this; }


   // prefix and postfix increment and decrement operators

   psc_uint<W>& operator ++ () // prefix
      { 
         sc_uint<W> tmp_(this->m_val+1);
         update_toggle_count((*this), tmp_);
         
#ifdef DEBUG_POWER_L1
         cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator ++ (prefix) TC:" 
            << dec << get_toggle_count() << endl;
#endif

         sc_uint<W>::operator ++ (); 
         return *this; 
      }

   const psc_uint<W> operator ++ ( int ) // postfix
      { 
         sc_uint<W> tmp_(this->m_val+1);
         update_toggle_count((*this), tmp_);
         
#ifdef DEBUG_POWER_L1
         cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator ++ (postfix) TC:" 
            << dec << get_toggle_count() << endl;
#endif

         return psc_uint<W>( sc_uint<W>::operator ++ ( 0 ) ); 
      }

   psc_uint<W>& operator -- () // prefix
      { sc_uint<W>::operator -- (); return *this; }

   const psc_uint<W> operator -- ( int ) // postfix
      { return psc_uint<W>( sc_uint<W>::operator -- ( 0 ) ); }


// ATTENTION: the code below was commented out to allow the following
// expression to be accepted by the g++ parser:
// 		sc_uint<5> a, b;
//			sc_uint<4> c;
//			a = b + c;
// But, this minor problem must be solved early

   
   // arithmetic operators
#if 0
   friend const psc_uint<W> operator + ( const int left, const psc_uint<W>& right )
      {
#ifdef DEBUG_POWER_L1
         cerr << "\toperator + (int " << left << ", psc_uint<W> " 
            << right << ")" << endl;
#endif
	 
         return psc_uint<W>(left + right.m_val); 
      }

   friend const psc_uint<W> operator + ( const uint_type left, const psc_uint<W>& right )
      {	      
#ifdef DEBUG_POWER_L1
         cerr << "\toperator + (uint_type " << left << ", psc_uint<W> " 
            << right << ")" << endl;
#endif

         return psc_uint<W>(left + right.m_val); 
      }

   
   const psc_uint<W> operator + ( const psc_uint<W>& right )
      {
#ifdef DEBUG_POWER_L1
         cerr << "\t(" << m_val << ") operator + (psc_uint<W> " << right << ")" << endl;
#endif

         return psc_uint<W>(m_val + right.m_val); 
      }

   const psc_uint<W> operator + ( const int right )
      {	      
#ifdef DEBUG_POWER_L1
         cerr << "\t(" << m_val << ") operator + (int " << right << ")" << endl;
#endif
	 
         return psc_uint<W>(m_val + right); 
      }


   const psc_uint<W> operator + ( const uint_type right )
      {	      
#ifdef DEBUG_POWER_L1
         cerr << "\t(" << m_val << ") operator + (uint_type<W> " << right << ")" << endl;
#endif
         return psc_uint<W>(m_val + right.m_val); 
      }
#endif
};

// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII


// ----------------------------------------------------------------------------
// Specialized versions of the virtual methods (inherited from psc_objinfo):
//  - on_bits
//  - bit_diff
//  - update_toggle_count
// ----------------------------------------------------------------------------
template <int W>
unsigned short int psc_uint<W>::on_bits( const sc_uint<W> & v )
{
#ifdef DEBUG_POWER_L2
   if ( this->m_len == 0 ) cerr << "Warning: m_len value is zero" << endl;
#endif
   
   unsigned char c0, c1, c2, c3, c4, c5, c6, c7;
   unsigned long long int _v = v;
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
   
	if ( psc_uint<W>::must_update() && count > 0 )
		for ( int i = 0 ; i < W ; i++ )
			if ( v[i] != 0 )
				psc_objinfo<W, sc_uint<W> >::inc_bit_toggle_count( i, 1 );
   
   return(count);
}

// sets on '1' changed bits between v1 and v2
template <int W>
inline sc_uint<W> psc_uint<W>::bit_diff(const sc_uint<W> & v1, const sc_uint<W> & v2)
{ 
   sc_uint<W> tmp;
   long long int _v1 = v1, _v2 = v2;
      
   tmp = _v1 ^ _v2;
   return( tmp ); 
}

template <int W>
void psc_uint<W>::update_toggle_count(const sc_uint<W> & cur_val, const sc_uint<W> & new_val)
{
   psc_objinfo<W, sc_uint<W> >::update_toggle_count(cur_val, new_val); 
}

template <int W>
uint64 psc_uint<W>::uint64value( const sc_uint<W> & v ) const
{
	uint64 value = v.to_uint64();
	return( value );
}

// ----------------------------------------------------------------------------
// Assignment Operators
// ----------------------------------------------------------------------------
template <int W>
psc_uint<W>& psc_uint<W>::operator = ( uint_type v )
{ 
   update_toggle_count(sc_uint<W>(*this), v);
         
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (uint_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator = ( v ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const sc_uint_base& a )
{ 
   update_toggle_count(sc_uint<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_uint_base " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const sc_uint_subref_r& a )
{ 
   update_toggle_count(sc_uint<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_uint_subref_r " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const sc_uint<W>& a )
{ 
   update_toggle_count(sc_uint<W>(*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_uint " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator = ( a ); 
   return *this; 
}
   
template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const psc_uint<W>& a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (psc_uint " << a.to_uint() 
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   this->m_val = a.m_val; 
   return *this; 
}

template <int W>
template <class T>
psc_uint<W>& psc_uint<W>::operator = ( const sc_generic_base<T>& a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_generic_base " << a.to_uint() 
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const sc_signed& a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_signed " << a.to_int() 
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const sc_unsigned& a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_unsigned " << a.to_uint() 
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const sc_bv_base& a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_bv_base " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const sc_lv_base& a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (sc_lv_base " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this;
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( const char* a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (char* " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( unsigned long a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (unsigned long " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( unsigned int a )
{
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (unsigned int " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
  
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( long a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (long " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( int a )
{ 
   update_toggle_count(sc_uint<W>(*this), a);
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (int " << a 
      << ") TC:" << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( int64 a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (int64 " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator = ( double a )
{ 
   update_toggle_count((*this), a);

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator = (double " << a
      << ") TC=" << dec << get_toggle_count() << endl;
#endif
   
   sc_uint<W>::operator = ( a ); 
   return *this;
}

// ----------------------------------------------------------------------------
// Arithmetic Assignment Operators
// ----------------------------------------------------------------------------
template <int W>
psc_uint<W>& psc_uint<W>::operator += ( uint_type v )
{ 
   update_toggle_count( sc_uint<W>(*this), ((*this) + v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator += (uint_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator += ( v ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator -= ( uint_type v )
{ 
   update_toggle_count( sc_uint<W>(*this), ((*this) - v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator -= (uint_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator -= ( v ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator *= ( uint_type v )
{ 
   update_toggle_count( sc_uint<W>(*this), ((*this) * v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator *= (uint_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator *= ( v ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator /= ( uint_type v )
{ 
   update_toggle_count( sc_uint<W>(*this), ((*this) / v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator /= (uint_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator /= ( v ); 
   return *this; 
}

template <int W>
psc_uint<W>& psc_uint<W>::operator %= ( uint_type v )
{ 
   update_toggle_count( sc_uint<W>(*this), ((*this) % v) );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << this->m_val << ") operator %= (uint_type " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif

   sc_uint<W>::operator %= ( v ); 
   return *this; 
}

 
} // namespace psc_dt


#endif

