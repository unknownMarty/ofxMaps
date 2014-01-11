/*
 *  TileLoader.cpp
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include "TileLoader.h"


#include <objc/objc-auto.h>

#include "ofThread.h"

TileLoader::TileLoader()
{

}
void TileLoader::setup()
{
    threadedLoaders.assign(4, new ofxThreadedImageLoader());
    threshold = 1;
    
    
    currentThread = 0;
}
void TileLoader::doThreadedPaint( const Coordinate &coord )
{    
    if (provider) {
        
        std::vector<std::string> urls = provider->getUrls(coord);
    
        if (urls.size() > 0) {
            try {
                memoryImages.push_back(std::make_pair(new ofImage(),coord));
                threadedLoaders[ofRandom(0, threadedLoaders.size())]->loadFromURL(*memoryImages.back().first, urls[0]);
                
                    currentThread++;
                if(currentThread >= threadedLoaders.size())
                   currentThread = 0;
            }
            catch( ... ) {
                std::cout << "Failed to load: " << urls[0] << std::endl;
            }
        }
    }
    
  
    
  
}

void TileLoader::processQueue(std::vector<Coordinate> &queue )
{
	while (pending.size() < MAX_PENDING && queue.size() > 0) {
        
        //allocate key
		Coordinate key = *(queue.begin());
        //cout<<key<<endl;
        // remove from que
		queue.erase(queue.begin());
        // add in pending list
        pending.insert(key);
     
        doThreadedPaint(key);
        beginTimeForPending = ofGetElapsedTimef();
	}
    
    // pending sometimes overflows. cleaning it fixes the problem.
    if(ofGetElapsedTimef() - beginTimeForPending > 2)
    {
        pending.clear();
        beginTimeForPending = 0;
    }

 
    for(int x=0; x < memoryImages.size(); x++)
    {
        if( memoryImages[x].first->isAllocated())
        {
//            cout<<x << " is allocated"<<endl;
            completed[memoryImages[x].second] = *memoryImages[x].first;
            memoryImages[x].first->clear();
            pending.erase(memoryImages[x].second);
          
            memoryImages.erase(memoryImages.begin()+x);
        }
    }
   
}

void TileLoader::transferTextures(std::map<Coordinate, ofFbo> &images)
{
    // use try_lock because we can just wait until next frame if needed
   // if (pendingCompleteMutex.tryLock()) {
        if (!completed.empty()) {
            std::map<Coordinate, ofImage>::iterator iter = completed.begin();
            if (iter->second.isAllocated()) {
                //iter->second.setUseTexture(true);
                
                getContours(iter->second, images[iter->first]);
                
            }
            completed.erase(iter);
        }
     //   pendingCompleteMutex.unlock();
   // }
}
void TileLoader::getContours(ofImage &image, ofFbo & fbo)
{
    colorImg.allocate(image.width,image.height);
    grayImage.allocate(image.width,image.height);
    colorImg.setFromPixels(image.getPixels(), image.width,image.height);
    grayImage = colorImg;
    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    //contourFinder.findContours(grayImage, 20, (image.width*image.height)/3, 10, true);	// find holes
    contourFinder.findContours(grayImage, 0, (image.width*image.height), 200, true);	// find holes
    fbo.allocate(image.width,image.height);
    
    fbo.begin();
    ofPushMatrix();
    glClearColor(1, 1, 1, 1);
    ofBackground(0, 0, 0);
    ofSetColor(255);

    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(0, 0);
    }
    ofPopMatrix();
    fbo.end();
    
}
bool TileLoader::isPending(const Coordinate &coord)
{
    bool coordIsPending = false;
  //  pendingCompleteMutex.lock();
    coordIsPending = (pending.count(coord) > 0);
   // pendingCompleteMutex.unlock();
    return coordIsPending;
}
    
void TileLoader::setMapProvider( MapProviderRef _provider )
{
//	pendingCompleteMutex.lock();
    completed.clear();
    pending.clear();
//	pendingCompleteMutex.unlock();
    provider = _provider;
}

