#pragma once

#include <cmath>

#include "ofMain.h"
#include "Transformation.h"
#include "Coordinate.h"
#include "Location.h"


    
class AbstractProjection {

public:

	double zoom;
	Transformation transformation;

	AbstractProjection(double _zoom): zoom(_zoom), transformation(Transformation()) { }
	AbstractProjection(double _zoom, Transformation _t): zoom(_zoom), transformation(_t) { }

	virtual ~AbstractProjection() { std::cout << "Abstract Projection destroyed" << std::endl; }
	
	virtual ofVec2f rawProject(const ofVec2f &point)=0;
	virtual ofVec2f rawUnproject(const ofVec2f &point)=0;

	ofVec2f project(const ofVec2f &point) {
		return transformation.transform(rawProject(point));
	}

	ofVec2f unproject(const ofVec2f &point) {
		return rawUnproject(transformation.untransform(point));
	}

	Coordinate locationCoordinate(const Location &location) {
		ofVec2f point = project(ofVec2f((M_PI/180.0) * location.lon, (M_PI/180.0) * location.lat));
		return Coordinate(point.y, point.x, zoom);
	}

	Location coordinateLocation(const Coordinate &coordinate) {
		// TODO: is this built into Cinder anyplace?
		static const double rad2deg = 180.0/M_PI;
		Coordinate zoomed = coordinate.zoomTo(zoom);
		ofVec2f point = unproject(ofVec2f(zoomed.column, zoomed.row));
		return Location(rad2deg * point.y, rad2deg * point.x);
	}

};

 typedef std::shared_ptr<AbstractProjection> ProjectionRef;
    
