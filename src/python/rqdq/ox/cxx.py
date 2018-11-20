import logging
import os

from rqdq.os import split_extension
from rqdq.ox.config import *
from rqdq.ox.graph import register_target_type
from rqdq.ox.graph import Target
from rqdq.ox.selector import canonicalize
from rqdq.ox.os import i2o
from rqdq.twisted import async_check_output
from rqdq.twisted import PendingEvent
from twisted.internet import defer
from twisted.internet.defer import DeferredList
from twisted.internet.defer import returnValue

logger = logging.getLogger()


class CompileConfig(object):
    def __init__(self, obj_inputs=None, include_dirs=None, lib_dirs=None, lib_inputs=None):
        self.obj_inputs = obj_inputs or set()
        self.include_dirs = include_dirs or set()
        self.lib_dirs = lib_dirs or set()
        self.lib_inputs = lib_inputs or set()

    def __add__(self, other):
        assert isinstance(other, CompileConfig)
        return CompileConfig(
            obj_inputs=self.obj_inputs | other.obj_inputs,
            include_dirs=self.include_dirs | other.include_dirs,
            lib_dirs=self.lib_dirs | other.lib_dirs,
            lib_inputs=self.lib_inputs | other.lib_inputs,
        )


class CxxBinaryLibraryTarget(Target):
    token = 'cxx_binary_library'

    def __init__(self, path, name, deps, include_dir, lib_dir, inputs):
        assert not deps
        super(CxxBinaryLibraryTarget, self).__init__(path, name, deps)
        self._include_dir = include_dir
        self._lib_dir = lib_dir
        self._inputs = inputs

    @classmethod
    def define(cls, path, name, deps=None, include_dir=None, lib_dir=None, inputs=None):
        return cls(
            path=path,
            name=name,
            deps=canonicalize(deps or [], path),
            include_dir=include_dir,
            lib_dir=lib_dir,
            inputs=inputs,
        )

    def compile(self, work_dir):
        return defer.succeed((False, CompileConfig(
            include_dirs={os.path.join(i2o(self._path), self._include_dir)},
            lib_dirs={os.path.join(i2o(self._path), self._lib_dir)},
            lib_inputs=set(self._inputs),
        )))


class WinResourceTarget(Target):
    token = 'win_resource'

    def __init__(self, path, name, deps, src):
        super(WinResourceTarget, self).__init__(path, name, deps)
        self._src = src
        self._event = None

    @classmethod
    def define(cls, path, name, deps=None, src=None):
        return WinResourceTarget(
            path=path,
            name=name,
            deps=canonicalize(deps or [], path),
            src=src,
        )

    @defer.inlineCallbacks
    def compile(self, work_dir):
        if self._event:
            result = yield self._event.deferred()
            returnValue(result)

        self._event = pe = PendingEvent()

        @defer.inlineCallbacks
        def work():

            any_deps_modified = False
            sub_tasks = [node.compile(work_dir) for node in self.rdeps]
            sub_tasks = yield DeferredList(sub_tasks)
            sub_config = CompileConfig()
            for success, data_ in sub_tasks:
                if not success:
                    raise data_
                this_modified, this_config = data_
                sub_config += this_config
                any_deps_modified = any_deps_modified or this_modified

            my_objs = set()

            src = self._src

            bsrc = os.path.basename(src)
            name, ext = split_extension(bsrc, raise_if_missing=True)

            src_path = i2o(self._path + '/' + src)
            dep_mtime = os.path.getmtime(src_path)

            output_path = os.path.join(work_dir, 'abcd_' + name)
            obj_path = output_path + '.res'

            my_tasks = []

            if not any_deps_modified and os.path.exists(obj_path) and os.path.getmtime(obj_path) > dep_mtime:
                # print 'up-to-date', obj_path
                any_rebuild = False
                my_objs.add(obj_path)
            else:
                any_rebuild = True
                logger.info('rc %s', src)
                # print 'CL', self._path, src
                my_tasks.append(rc_res(
                    src=src_path,
                    include_dirs=sub_config.include_dirs,
                    output_path=output_path,
                ))

            my_tasks = yield DeferredList(my_tasks)

            for success, data_ in my_tasks:
                if not success:
                    raise data_
                ok, output = data_
                if not ok:
                    raise Exception('one resource src did not compile')
                my_objs.add(output)

            my_config = CompileConfig(
                obj_inputs=my_objs,
                include_dirs=set(),
            )

            all_config = my_config + sub_config
            self._compile_config = all_config
            returnValue((any_rebuild, all_config))

        pe.run(work)
        result = yield pe.deferred()
        returnValue(result)


class CxxLibraryTarget(Target):
    token = 'cxx_library'

    def __init__(self, path, name, deps, srcs, include_dir, hdrs):
        super(CxxLibraryTarget, self).__init__(path, name, deps)
        self._srcs = srcs
        self._hdrs = hdrs
        self._include_dir = include_dir
        self._event = None

    @classmethod
    def define(cls, path, name, deps=None, include_dir='', srcs=None, hdrs=None):
        return cls(
            path=path,
            name=name,
            deps=canonicalize(deps or [], path),
            include_dir=include_dir,
            srcs=srcs or [],
            hdrs=hdrs or [],
        )

    @defer.inlineCallbacks
    def compile(self, work_dir):
        if self._event:
            result = yield self._event.deferred()
            returnValue(result)

        self._event = pe = PendingEvent()

        @defer.inlineCallbacks
        def work():
            any_deps_modified = False
            sub_tasks = [node.compile(work_dir) for node in self.rdeps]
            sub_tasks = yield DeferredList(sub_tasks)
            sub_config = CompileConfig()
            for success, data_ in sub_tasks:
                if not success:
                    raise data_
                this_modified, this_config = data_
                sub_config += this_config
                any_deps_modified = any_deps_modified or this_modified

            my_includes = {os.path.join(i2o(self._path), self._include_dir)}

            my_tasks = []

            hdr_mtime = 0
            for hdr in self._hdrs:
                hdr_path = i2o(self._path + '/' + hdr)
                hdr_mtime = max(hdr_mtime, os.path.getmtime(hdr_path))

            my_objs = set()

            any_rebuild = False
            for src in self._srcs:
                bsrc = os.path.basename(src)
                name, ext = split_extension(bsrc, raise_if_missing=True)

                src_path = i2o(self._path + '/' + src)
                dep_mtime = max(hdr_mtime, os.path.getmtime(src_path))

                output_path = os.path.join(work_dir, 'abcd_' + name)
                obj_path = output_path + '.obj'

                if not any_deps_modified and os.path.exists(obj_path) and os.path.getmtime(obj_path) > dep_mtime:
                    # print 'up-to-date', obj_path
                    my_objs.add(obj_path)
                    continue

                logger.info('cxx %s', src)
                any_rebuild = True
                # print 'CL', self._path, src
                my_tasks.append(cl_obj(
                    src=src_path,
                    include_dirs=sub_config.include_dirs | my_includes,
                    output_path=output_path,
                ))

            my_tasks = yield DeferredList(my_tasks)

            for success, data_ in my_tasks:
                if not success:
                    raise data_
                ok, output = data_
                if not ok:
                    raise Exception('one lib src did not compile')
                my_objs.add(output)

            my_config = CompileConfig(
                obj_inputs=my_objs,
                include_dirs=my_includes,
            )

            all_config = my_config + sub_config
            self._compile_config = all_config
            returnValue((any_rebuild, all_config))

        pe.run(work)
        result = yield pe.deferred()
        returnValue(result)


class CxxExeTarget(Target):
    goals = ('compile', 'run')
    token = 'cxx_exe'

    def __init__(self, path, name, deps, srcs):
        super(CxxExeTarget, self).__init__(path, name, deps)
        self._srcs = srcs
        self._event = None

    @classmethod
    def define(cls, path, name, deps=None, srcs=None):
        return cls(
            path=path,
            name=name,
            deps=canonicalize(deps or [], path),
            srcs=srcs,
        )

    @defer.inlineCallbacks
    def compile(self, work_dir):
        if self._event:
            result = yield self._event.deferred()
            returnValue(result)

        self._event = pe = PendingEvent()

        @defer.inlineCallbacks
        def work():
            sub_tasks = [node.compile(work_dir) for node in self.rdeps]
            sub_tasks = yield DeferredList(sub_tasks)
            sub_config = CompileConfig()
            any_deps_modified = False
            for success, data_ in sub_tasks:
                if not success:
                    raise data_
                this_modified, this_config = data_
                sub_config += this_config
                any_deps_modified = any_deps_modified or this_modified

            my_includes = set()  # XXX should exe's have include dirs?

            my_tasks = []
            for src in self._srcs:
                bsrc = os.path.basename(src)
                name, ext = split_extension(bsrc, raise_if_missing=True)

                output_path = os.path.join(work_dir, 'abcd_' + name)
                logger.info('cxx %s', src)
                # print 'CL', self._path, src
                my_tasks.append(cl_obj(
                    src=i2o(self._path + '/' + src),
                    include_dirs=sub_config.include_dirs | my_includes,
                    output_path=output_path,
                ))

            my_tasks = yield DeferredList(my_tasks)
            my_objs = set()
            for success, data in my_tasks:
                if not success:
                    raise data
                ok, output = data
                if not ok:
                    raise Exception('one exe src did not compile')
                my_objs.add(output)

            my_config = CompileConfig(
                obj_inputs=my_objs,
                include_dirs=my_includes,
            )

            all_config = my_config + sub_config

            my_exe = os.path.join(work_dir, 'abcd_' + src + '.exe')
            logger.info('exe %s', src)
            exe_result = yield link_exe(
                lib_dirs=all_config.lib_dirs,
                inputs=all_config.lib_inputs | all_config.obj_inputs,
                output_path=my_exe,
            )

            self._output_exe = my_exe
            returnValue(my_exe)

        pe.run(work)
        result = yield pe.deferred()
        returnValue(result)


register_target_type(CxxExeTarget)
register_target_type(CxxBinaryLibraryTarget)
register_target_type(CxxLibraryTarget)
register_target_type(WinResourceTarget)


@defer.inlineCallbacks
def cl_obj(src, include_dirs, output_path, flags=CXX_FLAGS, defs=CXX_DEFS):
    cmd = [CL_EXE, src, '/nologo', '/c'] + flags

    for key, value in defs.items():
        if value is False:
            continue
        elif value is True:
            item = key
        else:
            item = key + '=' + str(value)
        cmd.append('/D')
        cmd.append(item)

    for _dir in include_dirs:
        cmd.append('/I' + _dir)

    cmd.append('/Fo' + output_path + '.obj')
    cmd.append('/Fd' + output_path + '.pdb')

    # print 'EXEC', cmd

    exit_code, stdout, stderr = yield async_check_output(cmd)
    stdout = stdout.decode()
    if exit_code == 0:
        if 'warning' in stdout:
            msgs = clean_cl_messages(stdout)
            # print src, 'compiled with warnings:'
            print('>>>> WARNINGS while compiling', src)
            print(msgs)
        returnValue((True, output_path + '.obj'))
    elif exit_code == 2:  # language errors
        msgs = clean_cl_messages(stdout)
        print('>>>> ERRORS while compiling', src)
        print(msgs)
        returnValue((False, None))
    else:
        print('====== begin compiler error report ======')
        print('cmd:', cmd)
        print('exit code:', exit_code)
        print('stdout>>>')
        print(stdout)
        print('<<<')
        print('stderr>>>')
        print(stderr)
        print('<<<')
        print('======= end compiler error report =======')
        raise Exception('compile failed')


@defer.inlineCallbacks
def rc_res(src, include_dirs, output_path):
    cmd = [RC_EXE, '/nologo']

    for _dir in include_dirs:
        cmd.append('/i' + _dir)

    cmd.append('/fo' + output_path + '.res')
    cmd.append(src)  # MUST be last

    exit_code, stdout, stderr = yield async_check_output(cmd)
    stdout = stdout.decode()
    if exit_code == 0:
        returnValue((True, output_path + '.res'))
    else:
        print('====== begin compiler error report ======')
        print('cmd:', cmd)
        print('exit code:', exit_code)
        print('stdout>>>')
        print(stdout)
        print('<<<')
        print('stderr>>>')
        print(stderr)
        print('<<<')
        print('======= end compiler error report =======')
        raise Exception('compile failed')


@defer.inlineCallbacks
def link_exe(inputs, lib_dirs, output_path):
    # inputs.remove('d3dx9.lib')
    cmd = [LINK_EXE, '/nologo']
    cmd += LD_FLAGS
    cmd += ['/out:' + output_path]
    cmd += ['/libpath:'+d for d in lib_dirs]
    cmd += list(inputs)
    # print 'EXEC', ' '.join(cmd)

    exit_code, stdout, stderr = yield async_check_output(cmd)
    stdout = stdout.decode()
    if exit_code == 0:
        return
    else:
        print('====== begin linker error report ======')
        print('cmd:', cmd)
        print('exit code:', exit_code)
        print('stdout>>>')
        print(stdout)
        print('<<<')
        print('stderr>>>')
        print(stderr)
        print('<<<')
        print('======= end linker error report =======')
        raise Exception('compile failed')


def clean_cl_messages(text):
    text = text[text.index('\n') + 1:]
    return text.strip()
