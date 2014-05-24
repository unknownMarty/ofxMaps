/*
 *  TileLoader.cpp
 *  modestmaps-ci
 *
 *  Created by Tom Carden on 8/27/10.
 *  Modified by Martijn Mellema minusplusminus.com
 *  Copyright 2010 Stamen Design. All rights reserved.
 *
 */

#include "TileLoader.h"

#include "ofxiOS.h"
#include "ofxiOSExtras.h"

struct compare_second
{
    compare_second(string s) : _s(s) { }
    
    bool operator () (std::pair<Coordinate, string> const p)
    {
        return (p.second == _s);
    }
    
    string _s;
};

TileLoader::TileLoader()
{

}
void TileLoader::setup()
{
    threadedLoaders.assign(MAX_PENDING, new ofxThreadedImageLoader());
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
                coordWithUrl.push_back( std::make_pair(coord, urls[0]) );
                
                NSString* geturl = ofxiOSStringToNSString(urls[0]);
               // NSLog(@"%@",geturl);
                
                
                NSURL *imageURL = [NSURL URLWithString:geturl];
                
                
          
                
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
                    
                    NSString* currentUrl = ofxiOSStringToNSString(urls[0]);
                    NSData *imageData = [[NSData alloc] init];
                    
                    imageData =  [NSData dataWithContentsOfURL:imageURL];
                    ofImage * responseImage = new ofImage();
                    ofxiOSUIImageToOFImage([UIImage imageWithData:imageData], *responseImage);
                    
                    for(int x=0; x< coordWithUrl.size(); x++)
                    {
                        if(coordWithUrl[x].second == ofxiOSNSStringToString(currentUrl))
                        {
                            for(int y=0; y< memoryImages.size(); y++)
                            {
                                if(memoryImages[x].second == coordWithUrl[x].first)
                                {
                                    
                                    memoryImages[x].first = responseImage;
                                    memoryImages[x].first->setCompression(OF_COMPRESS_SRGB);
                                }
                            }
                        }
                    }

                    dispatch_async(dispatch_get_main_queue(), ^{
                        
                        
                       // self.imageView.image = [UIImage imageWithData:imageData];
                    });
                });
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
        
        pendingTime[key] = ofGetElapsedTimeMillis();
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
            completed[memoryImages[x].second] = *memoryImages[x].first;
            memoryImages[x].first->clear();
            pending.erase(memoryImages[x].second);
          
            memoryImages.erase(memoryImages.begin()+x);
            coordWithUrl.erase(coordWithUrl.begin()+x);
        }
    }
   
}


void TileLoader::transferTextures(std::map<Coordinate, ofTexture> &images)
{
    // use try_lock because we can just wait until next frame if needed
        if (!completed.empty()) {
            std::map<Coordinate, ofImage>::iterator iter = completed.begin();
            if (iter->second.isAllocated()) {
                    images[iter->first].loadData(iter->second.getPixelsRef());
                }
             completed.erase(iter);
            }
}

void TileLoader::transferTextures(std::map<Coordinate,ofMesh> &images)
{
    // use try_lock because we can just wait until next frame if needed
    if (pendingCompleteMutex.tryLock()) {
        if (!completed.empty()) {
            std::map<Coordinate, ofImage>::iterator iter = completed.begin();
            if (iter->second.isAllocated()) {
                getContours(iter->second, images[iter->first]);
            }
            completed.erase(iter);
        }
        pendingCompleteMutex.unlock();
    }
}

void TileLoader::transferTextures(std::map<Coordinate,vector<ofPolyline> > &images)
{
    // use try_lock because we can just wait until next frame if needed
    if (pendingCompleteMutex.tryLock()) {
        if (!completed.empty()) {
            std::map<Coordinate, ofImage>::iterator iter = completed.begin();
            if (iter->second.isAllocated()) {
                getContours(iter->second, images[iter->first]);
            }
          
            completed.erase(iter);
        }
       pendingCompleteMutex.unlock();
    }
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
    ofBackground(0, 0, 0,0);
    ofSetColor(255);
    ofNoFill();

    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(0, 0);
    }
    ofPopMatrix();
    
    fbo.end();
    
}

void TileLoader::getContours(ofImage &image, ofVbo & vbo)
{
    colorImg.allocate(image.width,image.height);
    grayImage.allocate(image.width,image.height);
    colorImg.setFromPixels(image.getPixels(), image.width,image.height);
    grayImage = colorImg;
    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    //contourFinder.findContours(grayImage, 20, (image.width*image.height)/3, 10, true);	// find holes
    contourFinder.findContours(grayImage, 0, (image.width*image.height), 200, true);	// find holes
   
    
    vbo.bind();
    ofPushMatrix();
    glClearColor(1, 1, 1, 1);
    ofBackground(0, 0, 0,0);
    ofSetColor(255);
    
    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(0, 0);
       
    }
    ofPopMatrix();
    vbo.unbind();
    
    

    
}


void TileLoader::getContours(ofImage &image, vector<ofPolyline> & blob)
{
    if(!colorImg.bAllocated)
        colorImg.allocate(image.width,image.height);
    if(!grayImage.bAllocated)
        grayImage.allocate(image.width,image.height);
    
    colorImg.setFromPixels(image.getPixels(), image.width,image.height);
    grayImage = colorImg;
    
    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    //contourFinder.findContours(grayImage, 20, (image.width*image.height)/3, 10, true);	// find holes
    contourFinder.findContours(grayImage, 5, (image.width*image.height), 200, true);	// find holes
    
    blob.resize(contourFinder.nBlobs);
    
    for(int x=0; x< contourFinder.nBlobs; x++)
    {
        blob[x].addVertices(contourFinder.blobs[x].pts);
        blob[x].getSmoothed(8);
    }
}


void TileLoader::getContours(ofImage &image, ofxAssimpModelLoader & model)
{
    colorImg.allocate(image.width,image.height);
    grayImage.allocate(image.width,image.height);
    colorImg.setFromPixels(image.getPixels(), image.width,image.height);
    grayImage = colorImg;
    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    //contourFinder.findContours(grayImage, 20, (image.width*image.height)/3, 10, true);	// find holes
    contourFinder.findContours(grayImage, 5, (image.width*image.height), 200, true);	// find holes
    
    
    ofMesh mesh;
   
    
    vector<ofPolyline> polys(contourFinder.nBlobs);
    for(int x=0; x< contourFinder.nBlobs; x++)
    {
        polys[x].addVertices(contourFinder.blobs[x].pts);
        polys[x].simplify();
      //  polys[x].getSmoothed(8);
    }
    NSString *filePath = [[[NSBundle mainBundle] resourcePath ]stringByAppendingPathComponent:@"file.ply"];
    
    ofTessellator tess;
    tess.tessellateToMesh(polys, OF_POLY_WINDING_NONZERO ,mesh);

    mesh.save([filePath  UTF8String],true);

    if(contourFinder.nBlobs)
        model.loadModel([filePath UTF8String]);
}


void TileLoader::getContours(ofImage &image, ofMesh & model)
{
    
    float beginTime = ofGetElapsedTimef();
    colorImg.allocate(image.width,image.height);
    grayImage.allocate(image.width,image.height);
    colorImg.setFromPixels(image.getPixels(), image.width,image.height);
    grayImage = colorImg;
    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    //contourFinder.findContours(grayImage, 20, (image.width*image.height)/3, 10, true);	// find holes
    contourFinder.findContours(grayImage, 5, (image.width*image.height), 200, true);	// find holes
    
    
    ofMesh mesh;
    
    
    vector<ofPolyline> polys(contourFinder.nBlobs);
    for(int x=0; x< contourFinder.nBlobs; x++)
    {
        polys[x].addVertices(contourFinder.blobs[x].pts);
        polys[x].simplify();
        //  polys[x].getSmoothed(8);
    }
    NSString *filePath = [[[NSBundle mainBundle] resourcePath ]stringByAppendingPathComponent:@"file.ply"];
    NSLog(@"%@",filePath);
    ofTessellator tess;
    tess.tessellateToMesh(polys, OF_POLY_WINDING_NONZERO ,mesh);
    cout<<"end time: "<<ofGetElapsedTimef() - beginTime <<endl;
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

