add_rules("mode.debug", "mode.release", "mode.releasedbg")	-- 编译模式
set_policy("build.warning", true)                   		-- 显示编译警告
set_policy("build.intermediate_directory", false)   		-- 不使用中间目录，目标文件直接放在build目录
set_policy("run.autobuild", true)                   		-- 运行前自动编译

-- 全局编译设置
set_encodings("utf-8")				-- 文件编码
set_warnings("all", "pedantic") 	-- 打开所有警告
set_languages("c++23")				-- C++23标准	

add_cxflags("-Wnull-dereference")
add_cxflags("-ffunction-sections", "-fdata-sections") -- 启用函数和数据段分离
add_ldflags("-Wl,--gc-sections") 	-- 启用链接器

-- 自定义规则
includes("xmake/rule", "xmake/task/*.lua")
add_rules("glm.reversez") -- GLM Reverse Z

-- 第三方库
add_requires(
	"libsdl3 3.2.22", 
	"glm 1.0.1", 
	"gzip-hpp v0.1.0", 
	"stb 2025.03.14"
)
add_requires("fastgltf v0.9.0", {configs={cxx_standard="20"}})
add_requires("imgui v1.92.1-docking", {configs={sdl3=true, sdl3_gpu=true, wchar32=true}})

-- 项目元信息
set_project("CG-Project")
set_description("计算机图形学课程作业")

includes("project", "lib")
