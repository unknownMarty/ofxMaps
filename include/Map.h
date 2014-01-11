#pragma once

#include <set>
#include <map>

#include "MapProvider.h"
#include "Location.h"
#include "MapExtent.h"
#include "Coordinate.h"
#include "QueueSorter.h"
#include "TileLoader.h"

#include "ofMain.h"

	
// TODO: public getter/setting, and/or Settings object for these values?
    
// limit tiles in memory
// 256 would be 64 MB, you may want to lower this quite a bit for your app
// (we'll always keep as many images as needed to fill the screen though)
#define MAX_IMAGES_TO_KEEP 200

// upping this can help appearances when zooming out, but also loads many more tiles
#define GRID_PADDING 0


class Map {
	
private:

	// how big?
	ofVec2f size;
		
	// pan and zoom
	Coordinate centerCoordinate;
	
	// angle in radians
	double rotation;
		
	// about the tiles
	MapProviderRef mapProvider;
    
    // loading tiles
    TileLoaderRef tileLoader;

	// loaded tiles
	map<Coordinate,ofFbo> images;
    
   
	
	// coords waiting to load
	vector<Coordinate> queue;
	
	// a list of the most recent MAX_IMAGES_TO_KEEP images we've seen
	vector<Coordinate> recentImages;
	
	// keep track of what we can see already:
	set<Coordinate> visibleKeys;
    
    
     int numDrawnImages;

	void grabTile(const Coordinate &coord);
	
	void processQueue();
    
    ofPoint beginTouch;

public:	
	
	Map() {}
	
    void setup( MapProviderRef _provider,  double _width, double _height );
    
	void update();
	void draw();
    
    
    void touchDown(ofTouchEventArgs &touch);
    void touchMoved(ofTouchEventArgs &touch);
    
	void panBy(const double &x, const double &y);
	void panBy(const ofVec2f &d);
	void scaleBy(const double &s);
	void scaleBy(const double &s, const double &x, const double &y);
	void scaleBy(const double &s, const ofVec2f &c);
	void rotateBy(const double &r, const double &x, const double &y);
	void rotateBy(const double &r, const ofVec2f &c);
	void zoomBy(const double &distance);

	void setZoom(const double &zoom);
	double getZoom() const; // TODO: maybe int getZoomLevel as well?
	Location getCenter() const;
	Coordinate getCenterCoordinate() const;
	
	void setCenter(const Coordinate &center);
	void setCenter(const Location &location);
	void panTo(const Location &location); // same as setCenter	
	void setCenterZoom(const Location &location, const double &zoom);
	
    void setExtent(const MapExtent &extent, bool forceIntZoom = true);
    MapExtent getExtent() const;
	
	MapProviderRef getMapProvider() const {
	    return mapProvider;
	}
	
    void setMapProvider( MapProviderRef _provider);
	
	ofVec2f coordinatePoint(const Coordinate &coord) const;
	Coordinate pointCoordinate(const ofVec2f &point) const;

	ofVec2f locationPoint(const Location &location) const;
	Location pointLocation(const ofVec2f &point) const;

	void zoomIn();	
	void zoomOut();
		
	void panUp();
	void panDown();
	void panLeft();
	void panRight();	

	void panAndZoomIn(const Location &location);
	
	void setSize(double _width, double _height);
	void setSize(ofVec2f size);
	ofVec2f getSize() const;
};
