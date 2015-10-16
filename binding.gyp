{
    "targets": [
        {
            "target_name": "geopix",
            "sources": [
                "src/geotiff.cc"
            ],
			"include_dirs": [
				'/usr/local/include'
			],
            "libraries": [
                "-ltiff",'-L /usr/local/lib'
            ]
        }
    ]
}