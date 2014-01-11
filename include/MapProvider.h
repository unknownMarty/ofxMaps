//
//  MapProvider.h
//  MultiTouch
//
//  Created by Tom Carden on 7/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

#include "ofMain.h"


#include "Location.h"
#include "Coordinate.h"


    
class MapProvider {
public:
    
    virtual ~MapProvider() { std::cout << "Map Provider destroyed" << std::endl; }
    
    // facts about the tiles that exist:
	virtual ofVec2f getTileSize() = 0;
    virtual int getMaxZoom() = 0;
    virtual int getMinZoom() = 0;

    // how the map turns geography into tiles:
	virtual Coordinate locationCoordinate(const Location &location) = 0;
	virtual Location coordinateLocation(const Coordinate &coordinate) = 0;
    
    virtual std::vector<std::string>  getUrls(const Coordinate &coord) = 0;
    // called from a background thread:
  
};
    
typedef std::shared_ptr<MapProvider> MapProviderRef;


