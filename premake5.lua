workspace 'Array'
	architecture		'x64'
	configurations		{ 'Debug', 'Release' }
	flags				{ 'MultiProcessorCompile' }
	startproject		'Tests'

outputdir = '%{cfg.buildcfg}_%{cfg.architecture}_%{cfg.system}'

project 'Tests'
	location			''
	kind				'ConsoleApp'
	language			'C++'
	cppdialect			'C++20'
	staticruntime		'On'
	warnings			'Extra'
	objdir				('.bin_int/' .. outputdir .. '/%{prj.name}')
	targetdir			('.bin/'	 .. outputdir .. '/%{prj.name}')
	files				{
							'include/**',
							'tests/*'
						}
	includedirs			'tests/doctest'

	filter 'configurations:Debug'
		runtime			'Debug'
		symbols			'On'
		defines			'DEBUG'

	filter 'configurations:Release'
		runtime			'Release'
		optimize		'Speed'