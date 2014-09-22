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
#ifndef PSC_SENSITIVE_H
#define PSC_SENSITIVE_H

#include <systemc.h>
#include "psc_logic.h"

using namespace psc_dt;

class psc_sensitive_pos : public sc_sensitive_pos
{
    friend class sc_module;

public:

    // typedefs
    typedef sc_signal_in_if<psc_logic> in_if_pl_type;
    typedef sc_in<psc_logic>           in_port_pl_type;
    typedef sc_inout<psc_logic>        inout_port_pl_type;

/*
private:

    // constructor
    explicit psc_sensitive_pos( sc_module* m_ )
       : sc_sensitive_pos( m_ ) 
    {}

    // destructor
    ~psc_sensitive_pos()
    {}
*/

public:

    psc_sensitive_pos& operator () ( const in_if_pl_type& p_ )
    {
      //return( sc_sensitive_pos::operator () (in_if_l_type(p_)) ); 
      in_if_l_type & pr_ = p_;
      sc_sensitive_pos::operator () ( pr_ );
      return *this;
    }
/*    
    psc_sensitive_pos& operator () ( const in_port_pl_type& p_ )
    {
       return( sc_sensitive_pos::operator () (in_port_l_type(p_)) );
    }
    
    psc_sensitive_pos& operator () ( const inout_port_pl_type& p_ )
    {
       return( sc_sensitive_pos::operator () (inout_port_l_type(p_)) );
    }

    psc_sensitive_pos& operator << ( const in_if_pl_type& p_ )
    {
       return( sc_sensitive_pos::operator << (in_if_l_type(p_)) );
    }
    
    psc_sensitive_pos& operator << ( const in_port_pl_type& p_ )
    {
       return( sc_sensitive_pos::operator << (in_port_l_type(p_)) );
    }
    
    psc_sensitive_pos& operator << ( const inout_port_pl_type& p_ )
    {
       return( sc_sensitive_pos::operator << (in_port_l_type(p_)) );
    }
*/
};

#endif

