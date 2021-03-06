#pragma once

#include <set>
#include <map>
#include "ofMain.h"

#include "Coordinate.h"
#include "MapProvider.h"
#include "ofxThreadedImageLoader.h"
#include "ofxOpenCv.h"
#include "ofxAssimpModelLoader.h"


// limit simultaneous calls to loadImage
#define MAX_PENDING 8


#define IMAGE_POLYLINES 0
#define IMAGE_IMAGE 1

#if IMAGE_POLYLINES == 1
typedef vector<ofPolyline > mapImage;
#elif IMAGE_IMAGE == 1
typedef ofTexture mapImage;
#else
typedef ofMesh mapImage;
#endif


class TileLoader;
typedef shared_ptr<TileLoader> TileLoaderRef;

class TileLoader
{
	
private:
    
    TileLoader( MapProviderRef _provider ): provider(_provider) {}
    
    void doThreadedPaint( const Coordinate &coord );
    
    void getContours(ofImage &image, ofFbo &fbo);
    void getContours(ofImage &image, vector<ofPolyline> & blob);
    void getContours(ofImage &image, ofVbo &vbo);
    void getContours(ofImage &image, ofxAssimpModelLoader & model);
    void getContours(ofImage &image, ofMesh & model);
    ofxCvColorImage	colorImg;
    
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
    
    ofxCvContourFinder contourFinder;
    
	ofMutex pendingCompleteMutex;
	
    int threshold;
    
   
	map<Coordinate, ofImage> completed;
    map<Coordinate,float> pendingTime;
    
    MapProviderRef provider;
    
public:
    void setup();
    TileLoader();
    
    set<Coordinate> pending;
    vector<pair< ofImage*, Coordinate> >memoryImages;
    vector<pair< Coordinate, string> >coordWithUrl;
    vector<ofxThreadedImageLoader*> threadedLoaders;
    ofxThreadedImageLoader threadedLoader;
    int currentThread;
    float beginTimeForPending;
    static TileLoaderRef create( MapProviderRef provider )
    {
        return TileLoaderRef( new TileLoader( provider ) );
    }
    
	void processQueue( vector<Coordinate> &queue );
	
	void transferTextures( map<Coordinate,ofMesh> &images);
    void transferTextures(std::map<Coordinate,vector<ofPolyline> > &images);
    void transferTextures(std::map<Coordinate, ofTexture> &images);
	bool isPending(const Coordinate &coord);
    
    void setMapProvider( MapProviderRef _provider );
    
};
    
