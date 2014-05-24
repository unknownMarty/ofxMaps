Original git from RandomEtc https://github.com/RandomEtc/modestmaps-cinder

This is an updated version of modestMapsOf with extra features
Modest maps for Openframeworks. Currently only tested on iOS
- Rotating map
- Map boundaries
- Replaced threading system with dispatch_async (Fast image loading without stutter)

Experimental features:

- Uses ofxOpenCv to detect borders for fake vector tiles
- Use ofPolylines to display lines

Future possiblities (would like some help with it):
- creating algorithm for straight lines.
- Adding Tilestache geoJSON data as source
