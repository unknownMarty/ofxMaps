#pragma once

#include <set>
#include <map>
#include "ofMain.h"

#include "Coordinate.h"
#include "MapProvider.h"
#include "ofxThreadedImageLoader.h"
#include "ofxOpenCv.h"

// limit simultaneous calls to loadImage
#define MAX_PENDING 4

class TileLoader;
typedef shared_ptr<TileLoader> TileLoaderRef;

class TileLoader
{
	
private:
    
    TileLoader( MapProviderRef _provider ): provider(_provider) {}
    
    void doThreadedPaint( const Coordinate &coord );
    
    void getContours(ofImage &image, ofFbo &fbo);
    
    ofxCvColorImage	colorImg;
    
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
    
    ofxCvContourFinder contourFinder;
    
	ofMutex pendingCompleteMutex;
	
    int threshold;
    
   
	map<Coordinate, ofImage> completed;
    
    MapProviderRef provider;
    
public:
    void setup();
    TileLoader();
    
    set<Coordinate> pending;
    vector<pair< ofImage*, Coordinate> >memoryImages;
    vector<ofxThreadedImageLoader*> threadedLoaders;
    ofxThreadedImageLoader threadedLoader;
    int currentThread;
    float beginTimeForPending;
    static TileLoaderRef create( MapProviderRef provider )
    {
        return TileLoaderRef( new TileLoader( provider ) );
    }
    
	void processQueue( vector<Coordinate> &queue );
	
	void transferTextures( map<Coordinate, ofFbo> &images);

	bool isPending(const Coordinate &coord);
    
    void setMapProvider( MapProviderRef _provider );
    
};
    
