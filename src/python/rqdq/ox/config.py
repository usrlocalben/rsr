CONFIG_FLAGS = "msvc release sync"
#CONFIG_FLAGS = "msvc release gui sync"
#CONFIG_FLAGS = "msvc release gui sync player"
#CONFIG_FLAGS = "msvc debug console"

BUILD = 'BUILD'

ROOT_FILE = 'ox.bat'

MAX_DEP_LEVEL = 32

WORK_DIR_PREFIX = 'ox-build-'

DIST_DIR = 'dist'

OVERWRITE_DIST = True

RC_EXE = 'rc.exe'

CXX_FLAGS = [
    # execution
    '/FS',          # use mspdbsrc.exe, concurrency support
    '/MP',          # multiprocessing: enable
    '/Gm-',         # minimal rebuild: disable  (why does msvc put this in "codegen" ?)

    # output
    #'/GL-',         # link-time codegen: disable
    '/bigobj',      # extended obj format: enable

    # codegen/language
    '/EHsc',        # C++ exceptions: enabled (also /GX)
    '/EHc',         # extern "C" defaults to nothrow
    '/GS-',         # security checks: disable
    '/std:c++17',   # c++ standard: c++17
    '/permissive-', # strict standard conformance: enabled
    #'/Zc:inline',
    #'/Zc:forScope',
    #'/Zc:wchar_t',

    # warnings
    '/W3',
    '/wd4244',      # double to float: possible loss of data
    '/wd4267',      # size_t to const int
    '/wd4305',      # double to float: truncation

    # fixed defs
    '/D_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING',
    '/D_ENABLE_EXTENDED_ALIGNED_STORAGE',
    ]

CXX_DEFS = {
    '_UNICODE': True,
    'UNICODE': True,
    'WIN32': True,
    }

LD_FLAGS = []

CONFIGS = {
    'release': {
        'CXX_FLAGS': [
            '/Oi',          # intrinsic functions: enable
            '/Ot',          # optimizer mode: speed
            '/O2',          # optimizer level: maximum
            '/Ob2',         # inline expansion level: 2
            '/fp:fast',     # fpu mode: less precise
            '/MD',          # runtime: msvcrt dll, non-debug
        ],
        'CXX_DEFS': {
            'NDEBUG': True,
        }
    },
    'debug': {
        'CXX_FLAGS': [
            '/MDd',         # msvcrt dll, debug
            '/Zi',          # debug: enable
            '/Od',          # disable optimization
            #'/RTC1',        # runtime checks, two kinds...
            #'/Ge',          # stack checking on all funcs
            #'/GR',          # C++ RTTI: enabled (for typeid())
        ],
        'LD_FLAGS': [
            '/debug',
        ]
    },
    'ltcg': {
        'CXX_FLAGS': [
            '/GL',          # link-time codegen: enable
        ],
        'LD_FLAGS': [
            '/ltcg',        # link-time codegen
        ],
    },
    'pgo1': {
        'CXX_FLAGS': [
            '/GL',          # link-time codegen: enable
        ],
        'LD_FLAGS': [
            '/ltcg',        # link-time codegen
            '/genprofile',
        ],
    },
    'pgo2': {
        'CXX_FLAGS': [
            '/GL',          # link-time codegen: enable
        ],
        'LD_FLAGS': [
            '/ltcg',        # link-time codegen
            '/useprofile:pgd=.ox\\abcd_rqv.m.cpp!1.pgc',
        ],
    },
    'gui': {
        'LD_FLAGS': ['/SUBSYSTEM:WINDOWS',],
    },
    'console': {
        'LD_FLAGS': ['/SUBSYSTEM:CONSOLE',],
        'CXX_DEFS': {
            '_CONSOLE': True,
        }
    },
    'sync': {
        'CXX_DEFS': {'ENABLE_MUSIC': True,},
    },
    'player': {
        'CXX_DEFS': {'SYNC_PLAYER': True,},
    }
}


for flag in CONFIG_FLAGS.split():
    if flag == 'msvc':
        CL_EXE = 'cl.exe'
        LINK_EXE = 'link.exe'
    elif flag == 'clang':
        CL_EXE = 'clang-cl.exe'
        LINK_EXE = 'lld-link.exe'
    else:
        data = CONFIGS.get(flag, {})
        CXX_FLAGS += data.get('CXX_FLAGS', [])
        LD_FLAGS += data.get('LD_FLAGS', [])
        CXX_DEFS.update(data.get('CXX_DEFS', {}))
