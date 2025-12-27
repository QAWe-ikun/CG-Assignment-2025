package("implot-new")
    set_homepage("https://github.com/epezent/implot")
    set_description("Immediate Mode Plotting")
    set_license("MIT")

    add_urls("https://github.com/epezent/implot/archive/refs/tags/$(version).tar.gz",
             "https://github.com/epezent/implot.git")

    add_versions("v0.17", "0aa3ff4fb97e553608e6758e77980eedf01745628fe6c025e647f941ae674127")

    add_deps("imgui >=1.92")

    on_install(function (package)
        local configs = {}
        io.writefile("xmake.lua", [[
            add_requires("imgui >=1.92")
            add_rules("mode.release", "mode.debug")
            target("implot")
                set_kind("$(kind)")
                set_languages("c++11")
                add_files("*.cpp|implot_demo.cpp")
                add_headerfiles("*.h")
                add_packages("imgui")
                if is_plat("windows") and is_kind("shared") then
                    add_rules("utils.symbols.export_all", {export_classes = true})
                end
        ]])
        import("package.tools.xmake").install(package)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include <implot.h>
            void test() {
                ImPlot::CreateContext();
                ImPlot::DestroyContext();
            }
        ]]}, {configs = {languages = "c++11"}}))
    end)