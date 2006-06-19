/**
 * @file      ac_tlm_dev_id.H
 * @author    Thiago Massariolli Sigrist   
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   2.0alpha1
 * @date      Fri, 05 May 2006 18:45:05 -0300
 *
 * @brief     Declares a class that uniquely identifies a device.
 *
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_TLM_DEV_ID_H_
#define _AC_TLM_DEV_ID_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// using statements

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// Class that uniquely identifies a port or another type of ArchC TLM device
/// complying to the ArchC TLM protocol.
class ac_tlm_dev_id {
  private:
    /// Has the ID of the next instance of a device.
    static int counter_;
  public:
    /// ID of the device instance.
    const int dev_id_;

    /// Default constructor.
    ac_tlm_dev_id();
};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_TLM_DEV_ID_H_

