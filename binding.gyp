{
    "targets": [
        {
            "target_name": "geopix",
            "sources": [
                "src/geotiff.cc",
                "src/xtiff.c",
                "src/geo_new.c",
				"src/geo_tiffp.c",
				"geo_simpletags.c"
            ],
            "libraries": [
                "-ltiff"
            ]
        }
    ]
}