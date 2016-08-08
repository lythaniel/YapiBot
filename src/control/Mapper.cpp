/*
 * Mapper.cpp
 *
 *  Created on: 6 mars 2016
 *      Author: lythaniel
 */

#include "Mapper.h"
#include "YapiBotCmd.h"
#include <stdlib.h> //for abs
#include <cstring> //for memset
#include <cmath> //for cos/sin
#include "Network.h"
#include "Utils.h"

#define PI 3.14159265


CMapper::CMapper()
{
	m_Map = new unsigned char [MAP_SIZE*MAP_SIZE];
	reset();
}

CMapper::~CMapper()
{
	if (m_Map != NULL)
	{
		delete m_Map;
	}
}

void CMapper::reset (void)
{
	memset (m_Map,MAP_AREA_UNKNOWN,MAP_SIZE*MAP_SIZE);
	m_PosX = MAP_SIZE/2;
	m_PosY = MAP_SIZE/2;
	updateMapTile(0,0,0xFF);
	updateMapTile(MAP_SIZE-1,0,0x00);
	updateMapTile(0,MAP_SIZE-1,0x00);
	updateMapTile(MAP_SIZE-1,MAP_SIZE-1,0xFF);
	/*m_Map[MAP_HEADER_SIZE] = 0xFF;
	m_Map[MAP_SIZE+MAP_HEADER_SIZE-1] = 0x00;
	m_Map[MAP_SIZE*(MAP_SIZE-1)+MAP_HEADER_SIZE] = 0x00;
	m_Map[MAP_SIZE*MAP_SIZE+MAP_HEADER_SIZE-1] = 0xFF;*/
}


void CMapper::update(unsigned int direction, unsigned int distance)
{
	int x,y,lx,ly;
	int delta_x, delta_y;
	float error, delta_error;

	//Transform coordinate from polar to cartesian
	polar2cartesian (direction, distance, x, y);
	fprintf(stdout,"Map update for direction %d distance %d: x= %d y=%d\n", direction, distance,x+m_PosX,y+m_PosY);


	updateMapTile(x+m_PosX,y+m_PosY,MAP_AREA_OBSTACLE);
	//Draw a line of "free area" over the line from our position to the one provided.
	/*error = 0;
	delta_x = m_PosX - x;
	delta_y = m_PosY - y;
	int dir_x = delta_x>=0?1:-1;
	int dir_y = delta_y>=0?1:-1;

	delta_error = abs(delta_y/delta_x);

	ly = y;
	for (lx = x; lx != (m_PosX+dir_x); lx += dir_x)
	{
		updateMapTile(lx,ly,MAP_AREA_FREE);
		error += delta_error;
		while (error >= 0.5)
		{
			updateMapTile(lx,ly,MAP_AREA_FREE);
			ly +=  dir_y;
			error = error -1;
		}
	}

	//If distance is below the obstacle threshold, then it means we detected an obstacle
	if (distance < MAP_AREA_MAXDISTOBS)
	{
		updateMapTile(x,y,MAP_AREA_OBSTACLE);
	}*/
}



void CMapper::updateMapTile (unsigned int x, unsigned int y, unsigned char val)
{
	if ((x < MAP_SIZE)&&(y < MAP_SIZE))
	{
		m_Map[((y * MAP_SIZE) + x)] = val;
		fprintf (stdout,"updating map @ %d\n",((y * MAP_SIZE) + x));
	}
}

void CMapper::polar2cartesian (unsigned int direction, unsigned int distance, int &x, int &y)
{
	//the direction is given according the heading from the compass, so 0° is "north" 90° "east" 180° "south" and 270° "west"
	// so we need to convert to radian.
	float dir_rad = direction * PI / 180;
	//As we are going to turn around the coordinate system we need do not use the direct mapping from cos to X axis and sin to Y axis.
	float fx = sin (dir_rad) * distance;
	float fy = - cos (dir_rad) * distance;

	//Normalise according to the tile size
	x = fx / MAP_TILE_SIZE;
	y = fy / MAP_TILE_SIZE;
}

void CMapper::sendMap(void)
{
	unsigned char payload [YAPIBOT_MAX_PL_SIZE];
	unsigned int mapLen = MAP_SIZE;
	unsigned int lenRemaining = MAP_SIZE*MAP_SIZE;
	unsigned int idx = 0;
	unsigned int size;

	while (lenRemaining > 0)
	{
		Utils::fromInt(mapLen,&payload[0]); //Total map len.
		Utils::fromInt(idx,&payload[4]); //Chunck offset.

		size = (lenRemaining>(YAPIBOT_MAX_PL_SIZE-8))?(YAPIBOT_MAX_PL_SIZE-8):lenRemaining;
		memcpy (&payload[8],&m_Map[idx],size);
		CNetwork::getInstance()->sendCmdPck (CmdInfoMap, payload, size+8);
		lenRemaining -= size;
		idx += size;
	}

}

