{
  'targets': [
    {
      'target_name': 'usockets',
      'include_dirs': [
        'src/',
      ],
      'type': 'static_library',
      'direct_dependent_settings':
      {
      },
      'dependencies':
      [
        '../../libuv/uv.gyp:libuv',
        '../../openssl/openssl.gyp:openssl',
      ],
      'sources': [
        'src/socket.c',
        'src/loop.c',
        'src/context.c',
        'src/bsd.c',
        'src/eventing/epoll_kqueue.c',
        'src/eventing/gcd.c',
        'src/eventing/libuv.c',
        'src/crypto/sni_tree.cpp',
        'src/crypto/openssl.c'
      ],
      'conditions': [
        [ 'OS == "linux"', {
          'defines':
          [
            'LIBUS_USE_OPENSSL',
            'LIBUS_USE_LIBUV'
          ]
        }],
        ['OS=="linux"', {
          'cflags_cc': [ '-std=c++17', '-DLIBUS_USE_LIBUV', '-DLIBUS_USE_OPENSSL' ],
        }],
        ['OS=="mac"', {
          'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
            'CLANG_CXX_LANGUAGE_STANDARD': 'c++17',
            'CLANG_CXX_LIBRARY': 'libc++',
            'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_THREADSAFE_STATICS': 'YES',
            'GCC_OPTIMIZATION_LEVEL': '3',
            'GCC_ENABLE_CPP_RTTI': 'YES',
            'OTHER_CFLAGS': [ '-fno-strict-aliasing', '-DLIBUS_USE_LIBUV', '-DLIBUS_USE_OPENSSL' ],
            'OTHER_CPLUSPLUSFLAGS': [ '-DLIBUS_USE_LIBUV', '-DLIBUS_USE_OPENSSL' ],
          }
        }],
        ['OS=="win"', {
          'cflags_cc': [ '/DUSE_LIBUV', '/DBUS_USE_OPENSSL' ],
          'cflags_cc!': []
        }]
       ]
    }
  ]
}
