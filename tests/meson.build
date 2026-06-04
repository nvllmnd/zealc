unity_dep = dependency('unity')

dependencies += unity_dep

strings_test = executable(
  'tests_strings',
  files('../tests/strings.c'),
  include_directories: inc,
  c_pch: '../pch/zeal_pch.h',
  dependencies: dependencies,
  install: true,
)

test('strings', strings_test)

# format_io_test = executable(
#   'tests_fomat_io',
#   files('../tests/format_io.c'),
#   include_directories: inc,
#   c_pch: '../pch/zeal_pch.h',
#   dependencies: dependencies,
#   install: true,
# )
#
# test('format_io', format_io_test)

allocators_test = executable(
  'tests_allocators',
  files('../tests/allocators.c'),
  include_directories: inc,
  c_pch: '../pch/zeal_pch.h',
  dependencies: dependencies,
  install: true,
)

test('allocators', allocators_test)

lex_parse_test = executable(
  'tests_lex_parse',
  files('../tests/lex_parse.c'),
  include_directories: inc,
  c_pch: '../pch/zeal_pch.h',
  dependencies: dependencies,
  install: true,
)

test('lex_parse', lex_parse_test)
