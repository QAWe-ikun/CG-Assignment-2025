-- Geometry Algorithms
-- Example: Culling, Clipping
target("graphics.geometry")
	set_kind("static")
	set_languages("c++23", {public=true})

	add_includedirs("include", {public=true})
	add_headerfiles("include/(**.hpp)", {public=true})
	add_files("src/**.cpp")

	add_packages("glm", {public=true})