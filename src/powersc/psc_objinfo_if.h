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

  psc_objinfo_if.h -- psc_objinfo_if interface

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#ifndef PSC_OBJINFO_IF_H
#define PSC_OBJINFO_IF_H

#include <string>

using namespace std;

namespace psc_power_base
{

// ----------------------------------------------------------------------------
//  CLASS : psc_objinfo_if
// 
// This class defines the public interface to the psc_objinfo class.
// ----------------------------------------------------------------------------
class psc_objinfo_if
{
public:
   virtual unsigned int get_toggle_count() const = 0;
   virtual double get_toggle_rate_u() const = 0; // u: uniformly distributed among bits
   virtual double get_toggle_rate() const = 0;
   virtual double get_sp0() const = 0;
   virtual double get_sp1() const = 0;
   virtual double get_wire_load() const = 0;
   virtual double get_net_load() const = 0;
   virtual double get_net_delay() const = 0;
	virtual string & get_id() = 0;
	virtual int get_fanout() const = 0;
	virtual void  set_wire_load( const double ) = 0;
	virtual void add_to_net_load( const double ) = 0;
	virtual void set_net_delay( const double ) = 0;
};

} // namespace psc_power_base

#endif
