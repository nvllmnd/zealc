project(
  'zeal',
  'c',
  meson_version : '>= 1.3.0',
  version : files('.version'),
  default_options : [
    'c_std=c23,c2y,c2x',
    'werror=true',
    'b_ndebug=if-release',
    'default_library=static',
    'warning_level=3'
  ],
)

add_global_arguments('-Wno-static-in-inline', language: 'c')
add_global_arguments('-Wno-gnu-statement-expression', language: 'c')


mimalloc_proj = subproject('mimalloc')
mimalloc_dep = mimalloc_proj.get_variable('mi_dep')


hedley_dep = dependency('hedley')

dependencies = [
  hedley_dep,
  mimalloc_dep
]

inc = include_directories(
  'include',
  'include/extern',
  'include/zeal',
)


memory_lib = static_library(
  'zmemory',
  files(
    'src/memory/cstr.c',
    'src/memory/alloc.c',
  ),
  include_directories: inc,
  dependencies: dependencies,
  install: true,
  pic: true,
  c_pch: 'pch/zeal_pch.h',
)

memory_dep = declare_dependency(
  include_directories: inc,
  link_with: memory_lib
)


dependencies += memory_dep


ast_lib = static_library(
  'zast',
  files('src/ast/expr.c'),
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

exe = executable(
  'zeal',
  'src/main.c',
  include_directories: inc,
  c_pch: 'pch/zeal_pch.h',
  dependencies : dependencies,
  install : true,
)

test('basic', exe)
