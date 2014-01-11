
#include "Map.h"


    
    void Map::setup( MapProviderRef _mapProvider, double _width, double _height )
    {
        tileLoader = TileLoader::create( _mapProvider );
        mapProvider = _mapProvider;
        size = ofVec2f(_width,_height);
        centerCoordinate = Coordinate(0.5,0.5,0);  // half the world width,height at zoom 0
        // fit to screen
        double z = log2(std::min(size.x,size.y) / 256.0); // FIXME: use provider's getTileSize
        centerCoordinate = centerCoordinate.zoomTo(z);
        // start with north up:
        rotation = 0.0;
        tileLoader->setup();
    }
	
void Map::update() {
	// TODO: Move non-drawing logic here
    
    // if we're in between zoom levels, we need to choose the nearest:
	int baseZoom = ofClamp((int)round(centerCoordinate.zoom), mapProvider->getMinZoom(), mapProvider->getMaxZoom());
    
	// these are the top left and bottom right tile coordinates
	// we'll be loading everything in between:
	Coordinate tl = pointCoordinate(ofVec2f::zero()).zoomTo(baseZoom);
	Coordinate tr = pointCoordinate(ofVec2f(size.x,0)).zoomTo(baseZoom);
	Coordinate bl = pointCoordinate(ofVec2f(0,size.y)).zoomTo(baseZoom);
	Coordinate br = pointCoordinate(size).zoomTo(baseZoom);
	
	// find start and end columns
	int minCol = floor(std::min(std::min(tl.column,tr.column),std::min(bl.column,br.column)));
	int maxCol = floor(std::max(std::max(tl.column,tr.column),std::max(bl.column,br.column)));
	int minRow = floor(std::min(std::min(tl.row,tr.row),std::min(bl.row,br.row)));
	int maxRow = floor(std::max(std::max(tl.row,tr.row),std::max(bl.row,br.row)));
	
	// pad a bit, for luck (well, because we might be zooming out between zoom levels)
	minCol -= GRID_PADDING;
	minRow -= GRID_PADDING;
	maxCol += GRID_PADDING;
	maxRow += GRID_PADDING;
	
	visibleKeys.clear();
	
	// grab coords for visible tiles
	for (int col = minCol; col <= maxCol; col++) {
		for (int row = minRow; row <= maxRow; row++) {
			
			Coordinate coord(row,col,baseZoom);
			
			// keep this for later:
			visibleKeys.insert(coord);
			
			if (images.count(coord) == 0) { // || (ofGetElapsedTimeMillis() - images[coord]->loadTime < 255)) {
				
				// fetch it if we don't have it
				grabTile(coord);
				
				// see if we have  a parent coord for this tile?
                bool gotParent = false;
                
                
				for (int i = (int)coord.zoom; i > coord.zoom-1; i--) {
					Coordinate zoomed = coord.zoomTo(i).container();
                    //					if (images.count(zoomed) > 0) {
                    //						visibleKeys.insert(zoomed);
                    //						gotParent = true;
                    //						break;
                    //					}
					// mark all parent tiles valid
					
					gotParent = true;
					if (images.count(zoomed) == 0) {
						// force load of parent tiles we don't already have
						grabTile(zoomed);
					}
				}
                
                
                
                
			}
			
		} // rows
	} // columns
	
	
	
	// stop fetching things we can't see:
	// (visibleKeys also has the parents and children, if needed, but that shouldn't matter)
	//queue.retainAll(visibleKeys);
	std::vector<Coordinate>::iterator iter = queue.begin();
	while (iter != queue.end()) {
		Coordinate key = *iter;
		if (visibleKeys.count(key) == 0){
			iter = queue.erase(iter);
		}
		else {
			++iter;
		}
	}
	
	// TODO sort what's left by distance from center:
	//queueSorter.setCenter(new Coordinate( (minRow + maxRow) / 2.0f, (minCol + maxCol) / 2.0f, zoom));
	//Collections.sort(queue, queueSorter);
	
	// load up to 4 more things:
	processQueue();
	
	// clear some images away if we have too many...
	int numToKeep = std::max(numDrawnImages,MAX_IMAGES_TO_KEEP);
	if (recentImages.size() > numToKeep) {
		// first clear the pointers from recentImages
		recentImages.erase(recentImages.begin(), recentImages.end()-numToKeep);
		//images.values().retainAll(recentImages);
		// TODO: re-think the stl collections used so that a simpler retainAll equivalent is available
		// now look in the images map and if the value is no longer in recent images then get rid of it
		std::map<Coordinate,ofFbo>::iterator iter = images.begin();
		std::map<Coordinate,ofFbo>::iterator endIter = images.end();
		for (; iter != endIter;) {
            Coordinate coord = iter->first;
            //			gl::Texture tile = iter->second;
			std::vector<Coordinate>::iterator result = find(recentImages.begin(), recentImages.end(), coord);
			if (result == recentImages.end()) {
				images.erase(iter++);
			}
			else {
				++iter;
			}
		}
	}

   }


void Map::draw() {
	// TODO: sort by zoom so we draw small zoom levels (big tiles) first:
	// can this be done with a different comparison function on the visibleKeys set?
	//Collections.sort(visibleKeys, zoomComparator);
    
    numDrawnImages  = 0;
    
    glPushMatrix();
    glRotatef(180.0*rotation/M_PI, 0, 0, 1);
    
	int numDrawnImages = 0;
	std::set<Coordinate>::iterator citer;
	for (citer = visibleKeys.begin(); citer != visibleKeys.end(); citer++) {
		Coordinate coord = *citer;
		
		double scale = pow(2.0, centerCoordinate.zoom - coord.zoom);
        ofVec2f tileSize = mapProvider->getTileSize() * scale;
		ofVec2f center = size * 0.5;
		Coordinate theCoord = centerCoordinate.zoomTo(coord.zoom);
		
		double tx = center.x + (coord.column - theCoord.column) * tileSize.x;
		double ty = center.y + (coord.row - theCoord.row) * tileSize.y;
		
		if (images.count(coord) > 0) {
			ofFbo tile = images[coord];
			// we want this image to be at the end of recentImages, if it's already there we'll remove it and then add it again
            //			recentImages.erase(remove(recentImages.begin(), recentImages.end(), tile), recentImages.end());
            std::vector<Coordinate>::iterator result = find(recentImages.begin(), recentImages.end(), coord);
            if (result != recentImages.end()) {
                recentImages.erase(result);
            }
			tile.draw(tx, ty, tileSize.x, tileSize.y );
			numDrawnImages++;
			recentImages.push_back(coord);
		}
	}
    
    glPopMatrix();
		
}
void Map::touchDown(ofTouchEventArgs &touch)
{
    beginTouch = touch;
}

void Map::touchMoved(ofTouchEventArgs &touch)
{
    panBy(touch.x - beginTouch.x, touch.y - beginTouch.y);
}
    
    void Map::panBy(const ofVec2f &delta) { panBy(delta.x, delta.y); }
	
    void Map::panBy(const double &dx, const double &dy) {
        const double sinr = sin(rotation);
        const double cosr = cos(rotation);
        const double dxr = dx*cosr + dy*sinr;
        const double dyr = dy*cosr - dx*sinr;
        const ofVec2f tileSize = mapProvider->getTileSize();
        centerCoordinate.column -= dxr / tileSize.x;
        centerCoordinate.row -= dyr / tileSize.y;
    }
    void Map::scaleBy(const double &s) {
        scaleBy(s, size * 0.5);
    }
    void Map::scaleBy(const double &s, const ofVec2f &c) {
        scaleBy(s, c.x, c.y);
    }
    void Map::scaleBy(const double &s, const double &cx, const double &cy) {
        const double prevRotation = rotation;
        rotateBy(-prevRotation,cx,cy);
        ofVec2f center = size * 0.5;
        panBy(-cx+center.x, -cy+center.y);
        centerCoordinate = centerCoordinate.zoomBy(log2(s));
        panBy(cx-center.x, cy-center.y);
        rotateBy(prevRotation,cx,cy);
    }
    void Map::rotateBy(const double &r, const double &cx, const double &cy) {
        panBy(-cx, -cy);
        rotation += r;
        panBy(cx, cy);
    }
    
    //////////////////
    
    double Map::getZoom() const {
        return centerCoordinate.zoom;
    }
    
    Location Map::getCenter() const {
        return mapProvider->coordinateLocation(centerCoordinate);
    }
    
    Coordinate Map::getCenterCoordinate() const {
        return centerCoordinate;
    }
    
    void Map::setCenter(const Coordinate &center) {
        centerCoordinate = center;
    }
    
    void Map::setCenter(const Location &location) {
        setCenter(mapProvider->locationCoordinate(location).zoomTo(getZoom()));
    }
    
    void Map::setCenterZoom(const Location &location, const double &zoom) {
        setCenter(mapProvider->locationCoordinate(location).zoomTo(zoom));
    }
    
    void Map::setZoom(const double &zoom) {
        centerCoordinate = centerCoordinate.zoomTo(zoom);
    }
    
    void Map::zoomBy(const double &dir) {
        centerCoordinate = centerCoordinate.zoomBy(dir);
    }
    
    void Map::zoomIn()  { zoomBy(1);  }
    void Map::zoomOut() { zoomBy(-1); }
    
    void Map::setExtent( const MapExtent &extent, bool forceIntZoom )
    {
        Coordinate TL = mapProvider->locationCoordinate( extent.getNorthWest() ).zoomTo( getZoom() );
        Coordinate BR = mapProvider->locationCoordinate( extent.getSouthEast() ).zoomTo( getZoom() );
        
        const ofVec2f tileSize = mapProvider->getTileSize();
        
        // multiplication factor between horizontal span and map width
        const double hFactor = (BR.column - TL.column) / (size.x / tileSize.x);
        
        // multiplication factor expressed as base-2 logarithm, for zoom difference
        const double hZoomDiff = log2(hFactor);
        
        // possible horizontal zoom to fit geographical extent in map width
        const double hPossibleZoom = TL.zoom - (forceIntZoom ? ceil(hZoomDiff) : hZoomDiff);
        
        // multiplication factor between vertical span and map height
        const double vFactor = (BR.row - TL.row) / (size.y / tileSize.y);
        
        // multiplication factor expressed as base-2 logarithm, for zoom difference
        const double vZoomDiff = log2(vFactor);
        
        // possible vertical zoom to fit geographical extent in map height
        const double vPossibleZoom = TL.zoom - (forceIntZoom ? ceil(vZoomDiff) : vZoomDiff);
        
        // initial zoom to fit extent vertically and horizontally
        double initZoom = std::min(hPossibleZoom, vPossibleZoom);
        
        // additionally, make sure it's not outside the boundaries set by provider limits
        initZoom = std::min(initZoom, (double)mapProvider->getMaxZoom());
        initZoom = std::max(initZoom, (double)mapProvider->getMinZoom());
        
        // coordinate of extent center
        const double centerRow = (TL.row + BR.row) / 2.0;
        const double centerColumn = (TL.column + BR.column) / 2.0;
        const double centerZoom = (TL.zoom + BR.zoom) / 2.0;
        setCenter( Coordinate(centerRow, centerColumn, centerZoom).zoomTo(initZoom) );
    }
    
    MapExtent Map::getExtent() const
    {
        return MapExtent( pointLocation( ofVec2f::zero() ), pointLocation( size ) );
    }
    
    void Map::setMapProvider( MapProviderRef _mapProvider )
    {
        tileLoader->setMapProvider( _mapProvider );
        images.clear();
        queue.clear();
        recentImages.clear();
        visibleKeys.clear();
        mapProvider = _mapProvider;
    }
    
    ofVec2f Map::coordinatePoint(const Coordinate &target) const
    {
        /* Return an x, y point on the map image for a given coordinate. */
        
        Coordinate coord = target;
        
        if(coord.zoom != centerCoordinate.zoom) {
            coord = coord.zoomTo(centerCoordinate.zoom);
        }
        
        // distance from the center of the map
        const ofVec2f tileSize = mapProvider->getTileSize();
        ofVec2f point = size * 0.5;
        point.x += tileSize.x * (coord.column - centerCoordinate.column);
        point.y += tileSize.y * (coord.row - centerCoordinate.row);
        
        ofVec2f rotated(point);
        rotated.rotate(rotation);
        
        return rotated;
    }
    
    Coordinate Map::pointCoordinate(const ofVec2f &point) const {
        /* Return a coordinate on the map image for a given x, y point. */		
        // new point coordinate reflecting distance from map center, in tile widths
        ofVec2f rotated(point);
        const ofVec2f tileSize = mapProvider->getTileSize();    
        rotated.rotate(-rotation);
        Coordinate coord(centerCoordinate);
        coord.column += (rotated.x - size.x * 0.5) / tileSize.x;
        coord.row += (rotated.y - size.y * 0.5) / tileSize.y;
        return coord;
    }
    
    ofVec2f Map::locationPoint(const Location &location) const {
        return coordinatePoint(mapProvider->locationCoordinate(location));
    }
    
    Location Map::pointLocation(const ofVec2f &point) const {
        return mapProvider->coordinateLocation(pointCoordinate(point));
    }
    
    void Map::panUp()    { panBy(0,size.y/8.0);  }
    void Map::panDown()  { panBy(0,-size.y/8.0); }
    void Map::panLeft()  { panBy(size.x/8.0,0);   }
    void Map::panRight() { panBy(-size.x/8.0,0);  }
    
    void Map::panAndZoomIn(const Location &location) {
        setCenterZoom(location, getZoom() + 1);
    }
    
    void Map::panTo(const Location &location) {
        setCenter(location);
    }
    
    //////////////////////////////////////////////////////////////////////////
    
    void Map::grabTile(const Coordinate &coord) {
        bool isAlreadyLoaded = images.count(coord) > 0;
        if (!isAlreadyLoaded) {
            bool isQueued = find(queue.begin(), queue.end(), coord) != queue.end();
            if (!isQueued) {
                // do this one last because it blocks TileLoader
                bool isPending = tileLoader->isPending(coord);
                if (!isPending) {
                    queue.push_back(coord);
                }
            }
        }
    }
    
    void Map::processQueue() {	
        sort(queue.begin(), queue.end(), QueueSorter(getCenterCoordinate().zoomTo(getZoom())));		
        tileLoader->processQueue(queue);
        tileLoader->transferTextures(images);
    }
    
    void Map::setSize(ofVec2f _size) {
        size = _size;
    }
	
    ofVec2f Map::getSize() const {
        return size;
    }
