## Geopix
Simple synchronous node.js module which utilize libtiff/libgeotiff to get the pixel value from geotiff and lat, lng.

### Requirements:
1. libtiff (tested on v4).
2. libgeotif

### Installation:
```bash
npm install geopix
```

### Usage example:
```javascript
var geopix 	= require('geopix');
var tif			= geopix('landslide_nowcast_d02_20150213.tif')
var value		= tif.latlng(15.12, -91.70)
 
```
