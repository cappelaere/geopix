#ifndef NODE_GEOTIFF_H_
#define NODE_GEOTIFF_H_

#include <node.h>
#include <node_object_wrap.h>

#include "tiffio.h"

	class GEOTIFFFile : public node::ObjectWrap {
	public:
		static void Init(v8::Handle<v8::Object> exports);

	private:
		explicit GEOTIFFFile(TIFF *tifHandle);
		~GEOTIFFFile();

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	  
		static void LatLng(const v8::FunctionCallbackInfo<v8::Value>& args);
			
		TIFF* tif;
		
		static v8::Persistent<v8::Function> constructor;
  };


#endif
