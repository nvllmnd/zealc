project(
  'zeal',
  'c',
  meson_version : '>= 1.3.0',
  version : files('.version'),
  default_options : [
    'c_std=c23,c2y,c2x,gnu2x,gnu23,gnu2y',
    'werror=true',
    'b_ndebug=if-release',
    'default_library=static',
    'warning_level=3'
  ],
)

add_global_arguments('-Wno-static-in-inline', language: 'c')
add_global_arguments('-Wno-gnu-statement-expression', language: 'c')


# mimalloc_proj = subproject('mimalloc')
# mimalloc_dep = mimalloc_proj.get_variable('mi_dep')



hedley_dep = dependency('hedley')

nv_dep = dependency('libnv')



dependencies = [
  hedley_dep,
  nv_dep,
  # mimalloc_dep,
]

inc = include_directories(
  'include',
  'include/core',
)

ast_lib = static_library(
  'zast',
  files(
    'src/ast/lex.c',
    'src/ast/expr.c',
    'src/ast/parser.c',
    'src/ast/keywords.c',
    'src/ast/ast.c',
  ),
  include_directories: inc,
  dependencies: dependencies,
  install: true,
  pic: true,
  c_pch: 'pch/zeal_pch.h'
)

ast_dep = declare_dependency(
  include_directories: inc,
  link_with: ast_lib
)

dependencies += ast_dep


core_lib = static_library(
  'zcore',
  files(
    'src/core/runes.c',

    'src/core/strpad.c',

  ),
  include_directories: inc,
  dependencies: dependencies,
  install: true,
  pic: true,
  c_pch: 'pch/zeal_pch.h',
)

core_dep = declare_dependency(
  include_directories: inc,
  link_with: core_lib,
)

dependencies += core_dep


compiler_lib = static_library(
  'zealc',
  files(
    'src/compiler/compiler.c',
    'src/compiler/eval.c',
  ),
  include_directories: inc,
  dependencies: dependencies,
  install: true,
  pic: true,
  c_pch: 'pch/zeal_pch.h',
)

compiler_dep = declare_dependency(
  include_directories: inc,
  link_with: compiler_lib,
)

dependencies += compiler_dep


exe = executable(
  'zealc',
  'src/main.c',
  include_directories: inc,
  c_pch: 'pch/zeal_pch.h',
  dependencies : dependencies,
  install : true,
)


subdir('tests')

# test('basic', exe)
