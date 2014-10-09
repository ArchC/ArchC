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

  psc_logic.cpp -- The psc_logic class.

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#include "psc_logic.h"

using namespace psc_dt;

// ----------------------------------------------------------------------------
// Specialized versions of the virtual methods (inherited from psc_objinfo):
//  - on_bits
//  - bit_diff
//  - update_toggle_count
// ----------------------------------------------------------------------------
unsigned short int psc_logic::on_bits( const sc_logic & v )
{
//   if ( !v.is_01() ) 
//      cerr << "Warning: value of sc_logic parameter is undefined" << endl;
	if ( psc_logic::must_update() && (v.to_bool() != 0) )
		psc_objinfo<1, sc_logic>::inc_bit_toggle_count( 0, 1 );

   return( v.to_bool() );
}

inline sc_logic psc_logic::bit_diff( const sc_logic & v1, const sc_logic & v2 )
{
   bool v1_, v2_;

   if ( v1.is_01() )
      v1_ = v1.to_bool();
   else {
      v1_ = ( m_bLastUndefToBool == true ) ? false : true;
      m_bLastUndefToBool = !m_bLastUndefToBool;
   }

   if ( v2.is_01() )
      v2_ = v2.to_bool();
   else {
      v2_ = ( m_bLastUndefToBool == true ) ? false : true;
      m_bLastUndefToBool = !m_bLastUndefToBool;
   }
   
   return( sc_logic(v1_ ^ v2_) );
}

void psc_logic::update_toggle_count( const sc_logic & cur_val, const sc_logic & new_val )
{
   psc_objinfo<1, sc_logic>::update_toggle_count( cur_val, new_val );
}

uint64 psc_logic::uint64value( const sc_logic & v ) const
{
	uint64 value = v.to_bool();
	return( value );
}


// ----------------------------------------------------------------------------
// Assignment Operators
// ----------------------------------------------------------------------------
psc_logic& psc_logic::operator = ( const psc_logic& a )
{ 
   update_toggle_count( (*this), a );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (psc_logic " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_logic::operator = ( a.value() );
   return *this;
}

psc_logic& psc_logic::operator = ( const sc_logic& a )
{ 
   update_toggle_count( (*this), a );

#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_logic " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_logic::operator = ( a );
   return *this;
}

psc_logic& psc_logic::operator = ( sc_logic_value_t v )
{ 
   update_toggle_count( (*this), v );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_logic_value_t " << v 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_logic::operator = ( v );
   return *this; 
}

psc_logic& psc_logic::operator = ( bool a )
{ 
   update_toggle_count( (*this), sc_logic( a ) );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (bool " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_logic::operator = ( a );
   return *this; 
}

psc_logic& psc_logic::operator = ( char a )
{ 
   update_toggle_count( (*this), sc_logic( a ) );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (char " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_logic::operator = ( a );
   return *this; 
}

psc_logic& psc_logic::operator = ( int a )
{ 
   update_toggle_count( (*this), sc_logic( a ) );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (int " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_logic::operator = ( a );
   return *this; 
}

psc_logic& psc_logic::operator = ( const sc_bit& a )
{ 
   update_toggle_count( (*this), sc_logic( a ) );
   
#ifdef DEBUG_POWER_L1
   cerr << hex << "\t" << PRINT_OBJ_STR << ": (" << (*this) << ") operator = (sc_bit " << a 
      << ") TC: " << dec << get_toggle_count() << endl;
#endif
   
   sc_logic::operator = ( a );
   return *this; 
}

