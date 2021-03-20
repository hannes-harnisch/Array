workspace "Array"
	architecture		"x64"
	configurations		{ "Debug", "Release" }
	flags				{ "MultiProcessorCompile" }
	startproject		"Array"

outputdir = "%{cfg.buildcfg}_%{cfg.architecture}_%{cfg.system}"

project "Tests"
	location			"Tests"
	kind				"ConsoleApp"
	language			"C++"
	cppdialect			"C++latest"
	staticruntime		"On"
	objdir				(".bin_int/" .. outputdir .. "/%{prj.name}")
	targetdir			(".bin/"	 .. outputdir .. "/%{prj.name}")
	files				{ "**.cc", "**.hh" }
	includedirs			""

	filter "configurations:Debug"
		runtime			"Debug"
		symbols			"On"

	filter "configurations:Release"
		runtime			"Release"
		optimize		"Speed"
		defines			"NDEBUG"