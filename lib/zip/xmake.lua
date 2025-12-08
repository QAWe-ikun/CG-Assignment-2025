-- GZIP Decompression
target("lib::zip")
	set_kind("static")
	set_languages("c++23", {public=true})

	add_packages("gzip-hpp", {public=true})

	add_files("src/*.cpp")
	add_includedirs("include", {public=true})
	add_headerfiles("include/**.hpp")
	
	add_deps("lib::util", {public=true})