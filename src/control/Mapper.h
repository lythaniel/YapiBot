/*
 * Mapper.h
 *
 * Copyright (C) 2016 Cyrille Potereau
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef MAPPER_H_
#define MAPPER_H_

#include "Singleton.h"

#define MAP_SIZE 200
#define MAP_TILE_SIZE 5

#define MAP_AREA_UNKNOWN 0x7F
#define MAP_AREA_OBSTACLE 0xFF
#define MAP_AREA_FREE 0x00

#define MAP_AREA_MAXDISTOBS 100 //Distance above which we decide there is no obstacle


/*Map cordinate is as follow:
 *     0,0 ----------------> MAP_SIZE,0 (X axis)
 *     |
 *     |
 *     |
 *     |
 *     |
 *     0,MAP_SIZE (Y axis)
 *
 *     Each map tile is a MAP_TILE_SIZE x MAP_TILE_SIZE square
 */

class CMapper : public CSingleton<CMapper> {
public:
	CMapper();
	~CMapper();

	void reset (void);
	void update (unsigned int direction, unsigned int distance);
	void sendMap (void);


private:
	void polar2cartesian (unsigned int direction, unsigned int distance, int &x, int &y);
	void updateMapTile (unsigned int x, unsigned int y, unsigned char val);

	unsigned char * m_Map;

	unsigned int m_PosX;
	unsigned int m_PosY;
};

#endif /* MAPPER_H_ */
