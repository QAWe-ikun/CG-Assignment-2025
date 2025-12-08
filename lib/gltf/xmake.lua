-- GLTF GPU Loader
target("gltf")
	set_kind("static")
	set_languages("c++23", {public=true})

	add_packages(
		"glm", 
		"stb", 
		"libsdl3", 
		"tinygltf",
		"meshoptimizer",
		"paul_thread_pool",
		{public=true}
	) 

	add_files("src/**.cpp")
	add_includedirs("include", {public=true})
	add_headerfiles("include/(**.hpp)")
	
	add_deps(
		"util", 
		"image.algo", 
		"image.compress", 
		"gpu", 
		"graphics.util",
		"graphics.geometry",
		{public=true}
	)
