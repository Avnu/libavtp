project(
	'libavtp',
	'c',
	version: '0.1',
	license: 'BSD-3-Clause',
)

avtp_lib = library(
	'avtp',
	[
	 'src/avtp.c',
	 'src/avtp_aaf.c',
	 'src/avtp_crf.c',
	 'src/avtp_cvf.c',
	 'src/avtp_ieciidc.c',
	 'src/avtp_stream.c',
	],
	version: meson.project_version(),
	include_directories: include_directories('include'),
	install: true,
)

install_headers(
	'include/avtp.h',
	'include/avtp_aaf.h',
	'include/avtp_crf.h',
	'include/avtp_cvf.h',
	'include/avtp_ieciidc.h',
)

pkg = import('pkgconfig')
pkg.generate(
	name: 'avtp',
	description: 'AVTP packetization library',
	version: meson.project_version(),
	url: 'github.com/AVnu/OpenAvnu',
	libraries: avtp_lib,
)

test_avtp = executable(
	'test-avtp',
	'unit/test-avtp.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	dependencies: dependency('cmocka'),
	build_by_default: false,
)

test_aaf = executable(
	'test-aaf',
	'unit/test-aaf.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	dependencies: dependency('cmocka'),
	build_by_default: false,
)

test_crf = executable(
	'test-crf',
	'unit/test-crf.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	dependencies: dependency('cmocka'),
	build_by_default: false,
)

test_stream = executable(
	'test-stream',
	'unit/test-stream.c',
	'src/avtp_stream.c',
	include_directories: include_directories('include', 'src'),
	link_with: avtp_lib,
	dependencies: dependency('cmocka'),
	build_by_default: false,
)

test_cvf = executable(
	'test-cvf',
	'unit/test-cvf.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	dependencies: dependency('cmocka'),
	build_by_default: false,
)

test_ieciidc = executable(
	'test-ieciidc',
	'unit/test-ieciidc.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	dependencies: dependency('cmocka'),
	build_by_default: false,
)

test('AVTP API', test_avtp)
test('Stream API', test_stream)
test('AAF API', test_aaf)
test('CRF API', test_crf)
test('CVF API', test_cvf)
test('IEC61883/IIDC API', test_ieciidc)

cc = meson.get_compiler('c')
mdep = cc.find_library('m', required : false)

executable(
	'aaf-talker',
	'examples/aaf-talker.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	build_by_default: false,
)

executable(
	'aaf-listener',
	'examples/aaf-listener.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	build_by_default: false,
)

executable(
	'crf-talker',
	'examples/crf-talker.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	dependencies : mdep,
	build_by_default: false,
)

executable(
	'crf-listener',
	'examples/crf-listener.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	dependencies : mdep,
	build_by_default: false,
)

executable(
	'cvf-talker',
	'examples/cvf-talker.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	build_by_default: false,
)

executable(
	'cvf-listener',
	'examples/cvf-listener.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	build_by_default: false,
)

executable(
	'ieciidc-talker',
	'examples/ieciidc-talker.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	build_by_default: false,
)
