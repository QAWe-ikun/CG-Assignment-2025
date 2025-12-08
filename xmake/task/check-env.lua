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

		function _check_tool_python()
			return (find_tool("python3") or find_tool("python")) ~= nil
		end

		function _check_python_module()
			local tool = find_tool("python3") or find_tool("python")
			local script_path = path.join(os.projectdir(), "script", "test-import.py")
			local stdout, _ = os.iorunv(tool.program, {script_path})
			return stdout:trim() == "1"
		end

		function _check_cpp23_std_expected()
			return has_cxxtypes("std::expected<int, int>", {includes = "expected", configs = {languages = "c++23"}})
		end

		function _check_cpp23_monadic()
			local test_code = [[
				#include <expected>
				#include <optional>

				std::optional<int> foo()
				{
					return 42;
				}

				std::optional<int> add_optional(int a)
				{
					return a + 42;
				}

				std::expected<int, int> bar()
				{
					return std::unexpected(1);
				}

				std::expected<int, int> add_expected(int a)
				{
					return a + 42;
				}

				int main()
				{
					auto result1 = foo().and_then(add_optional);
					auto result2 = bar().and_then(add_expected);
					return result1.value() + result2.value();
				}
			]]

			local ok, err = check_cxxsnippets(test_code, {configs={languages = "c++23"}})

			return ok
		end

		local has_glslang = _check_tool_ui(_check_tool_glslang, "glslangValidator/glslc")
		local has_python = _check_tool_ui(_check_tool_python, "Python")
		local has_python_module = has_python and _check_tool_ui(_check_python_module, "Python Libraries") or false
		local has_cpp23_std_expected = _check_tool_ui(_check_cpp23_std_expected, "C++23 std::expected")
		local has_cpp23_monadic = _check_tool_ui(_check_cpp23_monadic, "C++23 std::optional/std::expected Monads")

		local test_pass = 
			has_glslang 
			and has_python 
			and has_python_module 
			and has_cpp23_std_expected 
			and has_cpp23_monadic

		if test_pass then
			cprint("${green}Pass!")
		else
			cprint("${color.error}Failed!")

			if not has_glslang then
				cprint("=> ${white}glslangValidator/glslc${reset}:")
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

			if not has_cpp23_std_expected then
				cprint("=> ${white}C++23 std::expected${reset}: Install latest compiler")
			end

			if not has_cpp23_monadic then
				cprint("=> ${white}C++23 std::optional/std::expected Monads${reset}: Install latest compiler")
			end
		end
	end)

	set_menu {
		usage = "xmake check-env",
		description = "Check build environment",
		options = {}
	}