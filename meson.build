project('pmt_converter', 'cpp', 
  version : '0.0.0.1',
  license : 'BSD3',
  default_options : ['cpp_std=c++20'])

libpmtv = subproject('pmt')
gtest_dep = dependency('gtest', main : true, version : '>=1.10')
pmt_dep = libpmtv.get_variable('pmt_dep')


# executable('pmt_converter_example',
#            'main.cpp',
#            dependencies : [pmt_dep, gtest_dep],
#            include_directories: ['include'],
#            install : false)

pmt_converter_lib = library('pmt_converter',
        ['src/pmt_legacy_codec.cpp'],
        include_directories: 'include',
        install: true,
        link_language: 'cpp',
        dependencies: [pmt_dep])


pmt_converter_dep = declare_dependency(include_directories : 'include',
					   link_with : pmt_converter_lib,
                       dependencies : pmt_dep)

meson.override_dependency('pmt_converter', pmt_converter_dep)

subdir('tests')