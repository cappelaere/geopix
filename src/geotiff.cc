#include <math.h>
#include <stdlib.h>

#include "geotiff.hh"

using namespace v8;
Persistent<Function> GEOTIFFFile::constructor;

GEOTIFFFile::GEOTIFFFile(TIFF *tifHandle) { 
  tif   = tifHandle; 
};

GEOTIFFFile::~GEOTIFFFile() { TIFFClose(tif); };

void GEOTIFFFile::Init(Handle<Object> exports) {
  Isolate *isolate = Isolate::GetCurrent();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "GEOTIFFFile"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "LatLng", GEOTIFFFile::LatLng);
  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "GEOTIFFFile"), tpl->GetFunction());
}

void GEOTIFFFile::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  GEOTIFFFile *tifffile;

  if (args.Length() == 1 && args[0]->IsString()) {
    String::Utf8Value filename(args[0]);

    if (args.IsConstructCall()) {
      // Invoked as constructor: `new MyObject(...)`
      
      printf("geopix opening geotif %s...\n", *filename);
      
      TIFF *tif = TIFFOpen(*filename, "r");
      if (tif != NULL) {
      
        tifffile = new GEOTIFFFile(tif);
        tifffile->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
      } else {
        isolate->ThrowException(
            Exception::TypeError(String::NewFromUtf8(isolate,"Can't open tiff file.")));
      }
    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 1;
        Local<Value> argv[argc] = { args[0] };
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        args.GetReturnValue().Set(cons->NewInstance(argc, argv));
      }
  } else {
    printf("Invalid geopix init params %d\n", args.Length());
    isolate->ThrowException(
        Exception::TypeError(String::NewFromUtf8(isolate,"Invalid params (String filename)")));
  }
}

void GEOTIFFFile::LatLng(const FunctionCallbackInfo<v8::Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  GEOTIFFFile *self = ObjectWrap::Unwrap<GEOTIFFFile>(args.This());
  TIFF *tif  = self->tif;
  
  if (args.Length() < 2) {
      printf("Invalid geopix LatLng params %d\n", args.Length());
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong number of arguments")));
      return;
    }

  if (!args[0]->IsNumber() || !args[1]->IsNumber()) {
    
    isolate->ThrowException(
        Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  double lat = args[0]->NumberValue();
  double lng = args[1]->NumberValue();

  if (lat < -90.0 || lat > 90.0) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Invalide Latitude")));
    return;
  }
  
  if (lng < -180.0 || lng > 180.0) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Invalide Longitude")));
    return;
  }
  
	int xsize=0, ysize=0, dtype=0, bs=0;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &xsize);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &ysize);
	TIFFGetField(tif, TIFFTAG_DATATYPE, &dtype);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bs);
	
  printf(" lat: %f lng: %f xsize: %d ysize: %d dtype: %d bs: %d\n", lat, lng, xsize, ysize, dtype, bs);
  
#define TIFFTAG_GEOPIXELSCALE       33550
#define TIFFTAG_GEOTIEPOINTS        33922
  
  double *data;
	int count=0;
	double xmin=0, ymax=0, xres=0, yres=0;
	int verbose = 0;
  
  if (TIFFGetField(tif, TIFFTAG_GEOTIEPOINTS, &count, &data)) {
  	if( verbose ) {
  		printf("TIFFTAG_GEOTIEPOINTS: ");
  		for (int i = 0; i < count; i++) {
  			printf("%-17.15g ", data[i]);
  		}
  		printf("\n");
  	}
  	xmin 	= data[3];	// min long
  	ymax	= data[4];	// max lat
  }

  if (TIFFGetField(tif, TIFFTAG_GEOPIXELSCALE, &count, &data)) {
  	if( verbose ) {
  		printf("TIFFTAG_PIXELSCALE: ");
  		for (int i = 0; i < count; i++) {
  			printf("%-17.15g ", data[i]);
  		}
  		printf("\n");
  	}
  	xres 	= data[0];
  	yres 	= data[1];
  }
  
	// find pixel of interest
	long pixX			= lround((lng - xmin) / xres);
	long pixY			= lround((ymax - lat) / yres);
	
	if( pixX < 0 || pixX > xsize) printf("Invalid pixX %ld\n", pixX);
	if( pixY < 0 || pixY > ysize) printf("Invalid pixY %ld\n", pixY);
	
	// find position in buffer
	long pos			= pixY*xsize + pixX;
	if( pos < 0 || pos > xsize*ysize ) printf("Invalid pos %ld\n", pos);

	printf("geopixel %ld %ld pos %ld\n", pixX, pixY, pos);
	
	tstrip_t numstrips        = TIFFNumberOfStrips(tif);
	tstrip_t stripsize        = TIFFStripSize(tif);
  unsigned long bufferSize  = numstrips * stripsize;
  unsigned long imageOffset = 0;
  long result;

	char* buf                 = (char*)malloc(bufferSize);
  
  //	if( verbose ) printf("strips %u size %u\n", numstrips, stripsize);
	
	for(tstrip_t strip=0; strip < numstrips; strip++) {
    if((result = TIFFReadEncodedStrip (tif, strip,buf + imageOffset,stripsize)) == -1){
         fprintf(stderr, "Read error on input strip number %d\n", strip);
         isolate->ThrowException(Exception::TypeError( String::NewFromUtf8(isolate, "Read error on input strip")));
        return;
    }
    imageOffset += result;
	}

  double pixel_value = - bs;
	if( bs == 16 ) {
		uint16* ubuf  = (uint16*)buf;  
    pixel_value   = ubuf[pos];
  } else if( bs == 8 ) {
    pixel_value   = buf[pos];
  } else {
    printf("invalid bs %d\n", bs);
  }
  
	free(buf);

  Local<Number> val = Number::New(isolate, pixel_value);
  args.GetReturnValue().Set(val);
}

void InitAll(Handle<Object> exports) { 
  GEOTIFFFile::Init(exports); 
}

NODE_MODULE(geopix, InitAll)