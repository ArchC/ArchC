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

  psc_interp.cpp -- Interpolation functions

  Original Author: Felipe Klein <klein@ic.unicamp.br>, IC/UNICAMP, 2006.
  
  Contributors: Roberto Leao <leao@inf.ufsc.br>, INE/LAPS/UFSC.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#include <iostream>
#include <string.h>
#include "psc_interp.h"

namespace psc_util {

void psc_clear_point( psc_point_t *points, int n )
{
	memset( points, 0, sizeof(psc_point_t) * n );
}

void psc_clear_point( psc_point_4d_t *points, int n )
{
	memset( points, 0, sizeof(psc_point_4d_t) * n );
}

double psc_weighted_average( psc_point_t points[2], double x )
{
	psc_point_t P;

	P.x = x;
	P.y = P.z = P.weight = 0.0;

	points[0].weight = (points[1].x - P.x) / (points[1].x - points[0].x);
	points[1].weight = 1 - points[0].weight;

	P.y = (points[0].weight * points[0].y) + (points[1].weight * points[1].y);

	return( P.y );
}

double psc_geometric_centroid( psc_point_t points[4], double x, double y )
{
	psc_point_t P;

	P.x = x;
	P.y = y;
	P.z = 0.0;
	P.weight = 0.0;

#if 0
	std::cerr << "-- psc_geometric_centroid - begin" << std::endl;
	for ( int i = 0 ; i < 4 ; i++ )
		std::cerr << "points[" << i << "] = {" << points[i].x << ", " 
			<< points[i].y << ", " << points[i].z << "}" << std::endl;
#endif
	
	points[0].weight = 1.0 - ((P.y - points[0].y) / (points[1].y - points[0].y));
	points[0].weight *= 1.0 - ((P.x - points[0].x) / (points[3].x - points[0].x));

	points[1].weight = 1.0 - ((points[1].y - P.y) / (points[1].y - points[0].y));
	points[1].weight *= 1.0 - ((P.x - points[1].x) / (points[2].x - points[1].x));

	points[2].weight = 1.0 - ((points[2].y - P.y) / (points[2].y - points[3].y));
	points[2].weight *= 1.0 - ((points[2].x - P.x) / (points[2].x - points[1].x));

	points[3].weight = 1.0 - ((P.y - points[3].y) / (points[2].y - points[3].y));
	points[3].weight *= 1.0 - ((points[3].x - P.x) / (points[3].x - points[0].x));

#if 0
	for ( int i = 0 ; i < 4 ; i++ )
		std::cerr << "points[" << i << "].weight = " << points[i].weight << std::endl;
#endif

	P.z = (points[0].weight*points[0].z + points[1].weight*points[1].z + points[2].weight*points[2].z + points[3].weight*points[3].z);
	P.z /= (points[0].weight + points[1].weight + points[2].weight + points[3].weight);

#if 0
	std::cerr << "P.x = " << P.x << std::endl;
	std::cerr << "P.y = " << P.y << std::endl;
	std::cerr << "P.z = " << P.z << std::endl;
	std::cerr << "-- psc_geometric_centroid - end" << std::endl;
#endif

	return( P.z );
}

double psc_geometric_centroid( psc_point_4d_t points[8], double x, double y, double z )
{
    double sum_weights = 0.0;
    psc_point_4d_t P;

    P.x = x;
    P.y = y;
    P.z = z;
    P.power = 0.0;
    P.weight = 0.0;

#if 1
    std::cerr << "-- psc_geometric_centroid - begin" << std::endl;
    for ( int i = 0 ; i < 8 ; i++ )
        std::cerr << "points[" << i << "] = {" << points[i].x << ", "
            << points[i].y << ", " << points[i].z
            << ", " << points[i].power << ")" << std::endl;
#endif

    points[0].weight  = 1.0 - ((P.z - points[0].z) / (points[1].z - points[0].z));
    points[0].weight *= 1.0 - ((P.y - points[0].y) / (points[3].y - points[0].y));
    points[0].weight *= 1.0 - ((P.x - points[0].x) / (points[7].x - points[0].x));

    points[1].weight  = 1.0 - ((points[1].z - P.z) / (points[1].z - points[0].z));
    points[1].weight *= 1.0 - ((P.y - points[1].y) / (points[2].y - points[1].y));
    points[1].weight *= 1.0 - ((P.x - points[1].x) / (points[6].x - points[1].x));

    points[2].weight  = 1.0 - ((points[2].z - P.z) / (points[2].z - points[3].z));
    points[2].weight *= 1.0 - ((points[2].y - P.y) / (points[2].y - points[1].y));
    points[2].weight *= 1.0 - ((P.x - points[2].x) / (points[5].x - points[2].x));

    points[3].weight  = 1.0 - ((P.z - points[3].z) / (points[2].z - points[3].z));
    points[3].weight *= 1.0 - ((points[3].y - P.y) / (points[3].y - points[0].y));
    points[3].weight *= 1.0 - ((P.x - points[3].x) / (points[4].x - points[3].x));

    points[4].weight  = 1.0 - ((P.z - points[4].z) / (points[5].z - points[4].z));
    points[4].weight *= 1.0 - ((points[4].y - P.y) / (points[4].y - points[7].y));
    points[4].weight *= 1.0 - ((points[4].x - P.x) / (points[4].x - points[3].x));

    points[5].weight  = 1.0 - ((points[5].z - P.z) / (points[5].z - points[4].z));
    points[5].weight *= 1.0 - ((points[5].y - P.y) / (points[5].y - points[6].y));
    points[5].weight *= 1.0 - ((points[5].x - P.x) / (points[5].x - points[2].x));

    points[6].weight  = 1.0 - ((points[6].z - P.z) / (points[6].z - points[7].z));
    points[6].weight *= 1.0 - ((P.y - points[6].y) / (points[5].y - points[6].y));
    points[6].weight *= 1.0 - ((points[6].x - P.x) / (points[6].x - points[1].x));

    points[7].weight  = 1.0 - ((P.z - points[7].z) / (points[6].z - points[7].z));
    points[7].weight *= 1.0 - ((P.y - points[7].y) / (points[4].y - points[7].y));
    points[7].weight *= 1.0 - ((points[7].x - P.x) / (points[7].x - points[0].x));

#if 1
    for ( int i = 0 ; i < 8 ; i++ )
        std::cerr << "points[" << i << "].weight = " << points[i].weight << std::endl;
#endif
    // interpolate the power value given the weights
    P.power  = (points[0].weight*points[0].power + points[1].weight*points[1].power + points[2].weight*points[2].power + points[3].weight*points[3].power);
    P.power += (points[4].weight*points[4].power + points[5].weight*points[5].power + points[6].weight*points[6].power + points[7].weight*points[7].power);
    sum_weights  = (points[0].weight + points[1].weight + points[2].weight + points[3].weight);
    sum_weights += (points[4].weight + points[5].weight + points[6].weight + points[7].weight);
    P.power /= (sum_weights);

#if 1
    std::cerr << "P.x = " << P.x << std::endl;
    std::cerr << "P.y = " << P.y << std::endl;
    std::cerr << "P.z = " << P.z << std::endl;
    std::cerr << "P.power = " << P.power << std::endl;
    std::cerr << "-- psc_geometric_centroid - end" << std::endl;
#endif

	 return( P.power );
}


} // namespace psc_util
