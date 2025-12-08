task("check-env")
	on_run(function ()
		import("lib.detect.find_tool")
		import("lib.detect.has_cxxtypes")
		import("lib.detect.check_cxxsnippets")
		import("core.project.config")

		config.load()
		
		function _check_tool_ui(check_func, display_name)
			cprintf("> %s ... ", display_name)

			local result = check_func()

			if result then
				cprint("${color.success}ok")
			else
				cprint("${color.error}failed")
			end

			return result
		end

		function _check_tool_glslang()
			return (find_tool("glslangValidator") or find_tool("glslc")) ~= nil
		end

		function _check_tool_spirv_opt()
			return find_tool("spirv-opt") ~= nil
		end

		function _check_tool_python()
			return (find_tool("python3") or find_tool("python")) ~= nil
		end

		function _check_python_module()
			local tool = find_tool("python3") or find_tool("python")
			local script_path = path.join(os.projectdir(), "script", "test-import.py")
			local stdout, _ = os.iorunv(tool.program, {script_path})
			return stdout:trim() == "1"
		end

		function _check_cpp23_feature()
			local test_code = [[
				#include <version>

				static_assert(__cpp_lib_ranges_enumerate >= 202302L);
				static_assert(__cpp_lib_ranges_chunk >= 202202L);
				static_assert(__cpp_lib_ranges_slide >= 202202L);
				static_assert(__cpp_lib_expected >= 202211L);
				static_assert(__cpp_lib_optional >= 202110L);

				int main() 
				{
					return 0;
				}
			]]

			local ok, err = check_cxxsnippets(test_code, {configs={languages = "c++23"}})

			return ok
		end

		local has_glslang = _check_tool_ui(_check_tool_glslang, "glslangValidator/glslc")
		local has_spirv_opt = _check_tool_ui(_check_tool_spirv_opt, "spirv-opt")
		local has_python = _check_tool_ui(_check_tool_python, "Python")
		local has_python_module = has_python and _check_tool_ui(_check_python_module, "Python Libraries") or false
		local has_cpp23_features = _check_tool_ui(_check_cpp23_feature, "C++23 Required Features")

		local test_pass = 
			has_glslang 
			and has_python 
			and has_python_module 
			and has_cpp23_features

		if test_pass then
			cprint("${green}Pass!")
		else
			cprint("${color.error}Failed!")

			if not has_glslang or not has_spirv_opt then
				cprint("=> ${white}glslangValidator/glslc/spirv-opt${reset}:")
				cprint("    - Windows: Install Vulkan SDK => https://vulkan.lunarg.com/sdk/home")
				cprint("    - Ubuntu/Debian: Install using APT => ${cyan onblack}sudo apt install glslang-tools")
				cprint("    - ${color.warning}[!]${reset} If installed glslangValidator/glslc/Vulkan SDK but still failed, add to system PATH and retry")
			end

			if not has_python then
				cprint("=> ${white}Python${reset}: Install Python => https://www.python.org/downloads/")
			end

			if not has_python_module and has_python then
				cprint("=> ${white}Python Libraries${reset}: Install Python >= 3.10")
			end

			if not has_cpp23_features then
				cprint("=> ${white}C++23 Required Features${reset}: Install latest compiler")
			end
		end
	end)

	set_menu {
		usage = "xmake check-env",
		description = "Check build environment",
		options = {}
	}