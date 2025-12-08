-- 获取具体路径
function main(target)
	import("core.project.config")

	local gen_header_root = path.join(target:autogendir(), "codegen-include")
	local gen_header_path = path.join(gen_header_root, "asset")
	local gen_cpp_path = path.join(target:autogendir(), ".pack-temp", config.get("mode"))
	local script_path = path.join(os.projectdir(), "script", "pack2cpp.py")

	return {
		header_root = gen_header_root,
		header = gen_header_path,
		archive = gen_temp_path,
		cpp = gen_cpp_path,
		script = script_path
	}
end