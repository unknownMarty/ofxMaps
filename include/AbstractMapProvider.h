#pragma once

#include <cmath>
#include <vector>
#include <string>

#include "ofMain.h"

#include "MapProvider.h"
#include "AbstractProjection.h"
#include "Coordinate.h"
#include "Location.h"


#include "ofxThreadedImageLoader.h"
	
class AbstractMapProvider : public MapProvider {
	
public:
	
	ProjectionRef projection;
    
    ofxThreadedImageLoader threadedLoader;
	
	AbstractMapProvider(ProjectionRef _projection): projection(_projection) {}
	
	virtual ~AbstractMapProvider() { std::cout << "Abstract Map Provider destroyed" << std::endl; }
	
	virtual std::vector<std::string> getTileUrls(const Coordinate &coord)=0;

	ofVec2f getTileSize() {
		return ofVec2f(256,256);
	}
	
    // ...these values describe tiles that exist,
    // constraining the tiles for your app should be a separate
    // setting, TODO :)
	int getMaxZoom() {
		return 18;
	}
	
	int getMinZoom() {
		return 7;
	}    
	
	Coordinate locationCoordinate(const Location &location) {
		return projection->locationCoordinate(location);
	}
	
	Location coordinateLocation(const Coordinate &coordinate) {
		return projection->coordinateLocation(coordinate);
	}
	
	Coordinate sourceCoordinate(const Coordinate &coordinate) {
		const double gridSize = pow(2.0, coordinate.zoom);
		
		double wrappedColumn = coordinate.column;
		if(wrappedColumn >= gridSize) {
			wrappedColumn = fmod(wrappedColumn, gridSize);
		}
		else {
			while (wrappedColumn < 0) {
				wrappedColumn += gridSize;
			}
		}
		
		return Coordinate(coordinate.row, wrappedColumn, coordinate.zoom);
	}
    
     std::vector<std::string>  getUrls( const Coordinate &coord)
    {
        return getTileUrls(coord);
    }
    
    
   
};

