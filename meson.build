project(
	'libavtp',
	'c',
	version: '0.2.0',
	license: 'BSD-3-Clause',
	meson_version: '>=0.46.0',
)

avtp_lib = library(
	'avtp',
	[
	 'src/avtp.c',
	 'src/avtp_aaf.c',
	 'src/avtp_crf.c',
	 'src/avtp_cvf.c',
	 'src/avtp_rvf.c',
	 'src/avtp_ieciidc.c',
	 'src/avtp_stream.c',
	 'src/avtp_vsf_stream.c'
	],
	version: meson.project_version(),
	include_directories: include_directories('include'),
	install: true,
)

avtp_dep = declare_dependency(
	link_with: avtp_lib,
	include_directories: include_directories('include'),
)

install_headers(
	'include/avtp.h',
	'include/avtp_aaf.h',
	'include/avtp_crf.h',
	'include/avtp_cvf.h',
	'include/avtp_rvf.h',
	'include/avtp_vsf_stream.h',
	'include/avtp_ieciidc.h',
)

pkg = import('pkgconfig')
pkg.generate(avtp_lib,
	description: 'AVTP packetization library',
	url: 'github.com/AVnu/libavtp',
)

if get_option('tests') == 'disabled'
	cmocka = disabler()
else
	cmocka = dependency('cmocka', required: get_option('tests') == 'enabled')
endif

if cmocka.found()
	test_avtp = executable(
		'test-avtp',
		'unit/test-avtp.c',
		include_directories: include_directories('include'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)

	test_aaf = executable(
		'test-aaf',
		'unit/test-aaf.c',
		include_directories: include_directories('include'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)

	test_crf = executable(
		'test-crf',
		'unit/test-crf.c',
		include_directories: include_directories('include'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)

	test_stream = executable(
		'test-stream',
		'unit/test-stream.c',
		'src/avtp_stream.c',
		include_directories: include_directories('include', 'src'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)

	test_cvf = executable(
		'test-cvf',
		'unit/test-cvf.c',
		include_directories: include_directories('include'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)

	test_rvf = executable(
		'test-rvf',
		'unit/test-rvf.c',
		include_directories: include_directories('include'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)

	test_ieciidc = executable(
		'test-ieciidc',
		'unit/test-ieciidc.c',
		include_directories: include_directories('include'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)

	test_vsf_stream = executable(
		'test-vsf-stream',
		'unit/test-vsf-stream.c',
		include_directories: include_directories('include'),
		link_with: avtp_lib,
		dependencies: cmocka,
		build_by_default: false,
	)
	test('AVTP API', test_avtp)
	test('Stream API', test_stream)
	test('AAF API', test_aaf)
	test('CRF API', test_crf)
	test('CVF API', test_cvf)
	test('RVF API', test_rvf)
	test('IEC61883/IIDC API', test_ieciidc)
	test('VSF Stream API', test_vsf_stream)
endif

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

executable(
	'ieciidc-listener',
	'examples/ieciidc-listener.c',
	'examples/common.c',
	include_directories: include_directories('include'),
	link_with: avtp_lib,
	build_by_default: false,
)
