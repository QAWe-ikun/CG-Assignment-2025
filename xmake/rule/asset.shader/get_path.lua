-- 获取具体路径
function main(target)
	import("core.project.config")

	local gen_header_root_path = path.join(target:autogendir(), "codegen-include")
	local gen_header_path = path.join(gen_header_root_path, "asset", "shader")
	local gen_temp_path = path.join(target:autogendir(),".shader-temp", config.get("mode"), "gen")
	local script_path = path.join(os.projectdir(), "script", "bin2cpp.py")

	return {
		header_root = gen_header_root_path,
		header = gen_header_path,
		temp = gen_temp_path,
		script = script_path
	}
end