/**
 * @file      breakpoints.cpp
 * @author    Daniel Cabrini Hauagge    <ra008388@ic.unicamp.br>
 *            Gustavo Sverzut Barbieri  <ra008849@ic.unicamp.br>
 *            Joao Victor Andrade Neves <ra008951@ic.unicamp.br>
 *            Rafael Dantas de Castro   <ra009663@ic.unicamp.br>
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     Breakpoint support
 *            This class implements breakpoint support, actually it's 
 *            just a vector that keeps crescent order of elements and 
 *            have exits() method. It should be replaced for STL in future.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 * \note When modifing this file respect:
 * \li License
 * \li Previous author names. Add your own after current ones.
 * \li Coding style (basically emacs style)
 * \li Commenting style. This code use doxygen (http://www.doxygen.org)
 *     to be documented.
 *
 * \todo  This class should be replaced for standard STL element.
 */

#include "breakpoints.H"

/**
 * Constructor
 *
 * \param quant how many breakpoints to support
 */
Breakpoints::Breakpoints(int quant) {
  quantMax = quant;
  if ( ( bp = (unsigned int *) calloc( quantMax,
				       sizeof( unsigned int ) ) ) == NULL )
    {
      perror( "Couldn't allocate breakpoint array." );
      quantMax = 0;
    }
  memset( bp, 255, sizeof(unsigned int)* quantMax );
  this->quant = 0; /* no breakpoints at start up */
}


/**
 * Destructor
 */
Breakpoints::~Breakpoints() {
  if ( bp ) free( bp );
  bp = NULL;
}


/**
 * Add breakpoint at address
 *
 * \param address where to put breakpoint
 *
 * \param 0 on success, -1 otherwise
 */
int Breakpoints::add(unsigned int address) {
  int i, tmp;

  if ( ( ! bp ) || ( quant >= quantMax ) )
    return -1;

  for ( i = 0; i < quant; i++ )
    /* Insert in order */
    if ( address < bp[i] )
      {
	tmp     = address;
	address = bp[i];
	bp[i]   = tmp;
      }

  bp[ quant ++ ] = address;
  return 0;
}


/**
 * Remove breakpoint at address
 *
 * \param address the address to have breakpoint removed
 *
 * \param 0 on success, -1 otherwise
 */
int Breakpoints::remove(unsigned int address) {
  int i;

  if ( ( ! bp ) || ( quant >= quantMax ) )
    return -1;

  for ( i = 0; i < quant; i ++ )
    if ( bp[ i ] == address )
      {
	/* Copy remaining breakpoints: [ a | b | c ] => [ a | c ] */
	for ( ; i < ( quant - 1 ); i ++ )
	  bp[ i ] = bp[ i + 1 ];

	quant --;
	return 0;
      }

  return -1;
}

/**
 * Check if breakpoint exists
 *
 * \param address the address to be checked
 *
 * \return 1 if there is a breakpoint, 0 otherwise
 */
int Breakpoints::exists(unsigned int address) {
  int i;

  if ( ( ! bp ) || ( quant >= quantMax ) )
    return 0;

  for ( i = 0; i < quant; i ++ )
    if ( address == bp[ i ] )
      return 1;
    else
      /* bp is crescent order, so if current element is greater than desired
       * address, there's no address in list
       */
      if (  bp[ i ] > address )
	return 0;

  return 0;
}
