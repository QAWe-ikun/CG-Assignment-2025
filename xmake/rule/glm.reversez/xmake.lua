-- GLM Reverse Z
rule("glm.reversez")
	on_load(function (target)
		target:add("defines", "GLM_FORCE_DEPTH_ZERO_TO_ONE")
	end)