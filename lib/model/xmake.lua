-- 3D Model Parser
target("lib::model")
	set_kind("static")
	set_languages("c++23", {public=true})

	add_packages("glm", "fastgltf", "stb", {public=true}) -- GLM数学库

	add_files("src/**.cpp")
	add_includedirs("include", {public=true})
	add_includedirs("internal-include", {public=false})
	add_headerfiles("include/**.hpp", "internal-include/**.hpp")
	
	add_deps("lib::util", "lib::gpu", {public=true})
