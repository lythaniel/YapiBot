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

#include "YapiBotTypes.h"
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
	void update (uint32_t direction, uint32_t distance);
	void sendMap (void);


private:
	void polar2cartesian (uint32_t direction, uint32_t distance, int32_t &x, int32_t &y);
	void updateMapTile (uint32_t x, uint32_t y, uint8_t val);

	uint8_t * m_Map;

	uint32_t m_PosX;
	uint32_t m_PosY;
};

#endif /* MAPPER_H_ */
