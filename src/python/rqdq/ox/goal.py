import logging
import os
import shutil
import sys

from rqdq.os import maybe_mkdir
from rqdq.ox.config import DIST_DIR
from rqdq.ox.misc import check_it_yo
from rqdq.twisted import async_check_output
from twisted.internet import defer
from twisted.python import log

# logging setup
logger = logging.getLogger()
log.startLogging(sys.stdout)


class Goal(object):
    def __init__(self, work_dir):
        self._work_dir = work_dir

    def prepare(self):
        pass

    def run(self, target):
        raise NotImplementedError()

    def finalize(self):
        pass


class ResolveGoal(Goal):
    def __init__(self, work_dir, bg):
        super(ResolveGoal, self).__init__(work_dir)
        self._work_dir = work_dir
        self._bg = bg

    @defer.inlineCallbacks
    def run(self, target):
        check_it_yo(self._bg, target)
        if False:
            yield None
        defer.returnValue(True)


class CompileGoal(Goal):
    def __init__(self, work_dir, bg):
        super(CompileGoal, self).__init__(work_dir)
        self._work_dir = work_dir
        self._bg = bg
        self.resolve = ResolveGoal(work_dir, bg)
        self.target_count = 0

    @defer.inlineCallbacks
    def run(self, target):
        if 'compile' not in target.goals:
            return

        result = yield self.resolve.run(target)
        try:
            logger.debug('will compile %s in %s', target.selector, self._work_dir)
            built = yield target.compile(self._work_dir)
        except Exception as err:
            logger.critical('unhandled exception while building %s: %s',
                            target.selector, str(err))
            raise
        if built:
            logger.info('%s OK', target.selector)
            # todo: add dist goal
            maybe_mkdir(DIST_DIR)
            dist_exe = target._path.replace('/', '.') + '.' + target._name + '.exe'
            shutil.copy(built, os.path.join(DIST_DIR, dist_exe))
            self.target_count += 1
            defer.returnValue(built)

    def finalize(self):
        logger.info('%d target(s) built', self.target_count)


class RunGoal(Goal):
    def __init__(self, work_dir, bg):
        super(RunGoal, self).__init__(work_dir)
        self._compile = CompileGoal(work_dir, bg)
        self._work_dir = work_dir
        self._bg = bg

    @defer.inlineCallbacks
    def run(self, target):
        if 'run' not in target.goals:
            return

        maybe_exe_path = yield self._compile.run(target)
        if maybe_exe_path:
            cmd = [maybe_exe_path,]
            print('>>> executing "%s"' % (maybe_exe_path,))
            exit_code, stdout, stderr = yield async_check_output(cmd)
            stdout = stdout.decode('cp437').replace('\r\n', '\n')
            print(stdout)
            print('<<< terminated with code %d' % (exit_code,))
        defer.returnValue(True)

    def finalize(self):
        self._compile.finalize()


class TestGoal(Goal):
    def __init__(self, work_dir, bg):
        super(TestGoal, self).__init__(work_dir)
        self._compile = CompileGoal(work_dir, bg)
        self._work_dir = work_dir
        self._bg = bg

    @defer.inlineCallbacks
    def run(self, target):
        if 'test' not in target.goals:
            defer.returnValue(True)

        maybe_exe_path = yield self._compile.run(target)
        if maybe_exe_path:
            cmd = [maybe_exe_path,]
            print('>>> executing "%s"' % (maybe_exe_path,))
            exit_code, stdout, stderr = yield async_check_output(cmd)
            stdout = stdout.decode('cp437').replace('\r\n', '\n')
            print(stdout)
            print('<<< terminated with code %d' % (exit_code,))
            if exit_code == 0:
                defer.returnValue(True)
        defer.returnValue(False)

    def finalize(self):
        self._compile.finalize()
