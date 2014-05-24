#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "ofMain.h"
#include "AbstractMapProvider.h"
#include "MercatorProjection.h"

#include "functions.h"

#include "proj_api.h"
	
class OpenStreetMapProvider : public AbstractMapProvider {
	
private:

	std::vector<std::string> subdomains;
	    
	OpenStreetMapProvider(): 
        // this is the projection and transform you'll want for any Google-style map tile source:
        AbstractMapProvider( MercatorProjection::createWebMercator() )
    {
		// TODO: is there a better way to initialize a constant size vector or array of strings?
		subdomains.push_back("");
		subdomains.push_back("a.");
		subdomains.push_back("b.");
		subdomains.push_back("c.");
	}
    
public:
	
    static MapProviderRef create()
    {
        return MapProviderRef( new OpenStreetMapProvider() );
    }
		
	std::vector<std::string> getTileUrls(const Coordinate &rawCoordinate) {
		std::vector<std::string> urls;
		if (rawCoordinate.row >= 0 && rawCoordinate.row < pow(2, rawCoordinate.zoom)) {
            
            
            int zoom  = ofClamp(rawCoordinate.zoom, getMinZoom(), getMaxZoom());
			Coordinate coordinate = sourceCoordinate(rawCoordinate);
			std::stringstream url;
           
			url << "http://" <<"{YOUROWNSERVER.COM}:8888/v2/{MAPNAME}/" << zoom << "/" << (int)coordinate.column << "/" << (int)coordinate.row << ".png";
           // std::cout << rawCoordinate << " --> " << url.str() << std::endl;
			urls.push_back(url.str());
            url.clear();
            url.str("");
		}
		return urls;
	}
	
    
    
   
};

