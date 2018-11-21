"""
ox

usage:
    ox resolve <selector> [--log-level=<level>]
    ox compile <selector> [--log-level=<level>]
    ox run <selector> [--log-level=<level>]
    ox test <selector> [--log-level=<level>]
    ox (-h | --help)

options:
    -h --help   Show this screen.
    --log-level=<level>  Set log level [default: info]
"""
import logging
import os
import sys

from docopt import docopt
from rqdq.os import ancestor_dirs
from rqdq.os import maybe_mkdir
from rqdq.ox.config import *
from rqdq.ox.goal import CompileGoal
from rqdq.ox.goal import ResolveGoal
from rqdq.ox.goal import RunGoal
from rqdq.ox.goal import TestGoal
from rqdq.ox.graph import BuildGraph
from rqdq.ox.graph import target_type_registry
from rqdq.ox.os import o2i
from rqdq.twisted import ET_EXCEPTION
from rqdq.twisted import ET_VALUE
from rqdq.twisted import run_one
from twisted.internet import defer
from twisted.python import log

# logging setup
logger = logging.getLogger()
log.startLogging(sys.stdout)

# register cxx targets
import rqdq.ox.cxx


@defer.inlineCallbacks
def main(arguments):
    logger.setLevel(logging.INFO)
    stdout_handler = logging.StreamHandler(sys.stdout)
    stdout_handler.setLevel(logging.DEBUG)
    logger.addHandler(stdout_handler)

    log_level_name = arguments.get('--log-level').upper()
    log_level_number = getattr(logging, log_level_name, None)
    if log_level_number is None:
        msg = 'unknown log level "%s"' % (log_level_name,)
        raise Exception(msg)
    logger.setLevel(log_level_number)

    root_path = find_root()
    logger.debug('found repository root at "%s"', root_path)
    os.chdir(root_path)

    bg = load_build_graph()

    if arguments.get('compile'):
        # XXX rnd = str(uuid.uuid4())[0:5]
        work_dir = os.path.join(root_path, '.ox')
        maybe_mkdir(work_dir)
        goal = CompileGoal(work_dir=work_dir, bg=bg)
    elif arguments.get('run'):
        # XXX rnd = str(uuid.uuid4())[0:5]
        work_dir = os.path.join(root_path, '.ox')
        maybe_mkdir(work_dir)
        goal = RunGoal(work_dir=work_dir, bg=bg)
    elif arguments.get('test'):
        # XXX rnd = str(uuid.uuid4())[0:5]
        work_dir = os.path.join(root_path, '.ox')
        maybe_mkdir(work_dir)
        goal = TestGoal(work_dir=work_dir, bg=bg)
    elif arguments.get('resolve'):
        # XXX rnd = str(uuid.uuid4())[0:5]
        work_dir = os.path.join(root_path, '.ox')
        maybe_mkdir(work_dir)
        goal = ResolveGoal(work_dir=work_dir, bg=bg)
    else:
        raise NotImplementedError()

    goal.prepare()

    matched = list(bg.find_many(arguments.get('<selector>')))
    for match in sorted(matched):
        # logger.debug('running %s for target "%s"', goal, match)
        target = bg.find_one(match)
        yield goal.run(target)

    goal.finalize()
    defer.returnValue(0)


def load_build_graph():
    bg = BuildGraph()
    for path, _, files in os.walk('.'):
        if BUILD in files:
            # XXX detect windows/linux paths here?
            path = o2i(path)
            path = path[2:]  # remove './' prefix
            if path.startswith('src/') or path.startswith('3rdparty/'):
                load_targets(path, bg)
    return bg


def load_targets(path, bg):
    default_name = path.split('/')[-1]

    env = make_build_environment(default_name, path, bg)

    build_file_name = os.path.join(path, BUILD)
    with open(build_file_name, 'r') as fd:
        build_code = fd.read()
    bytecode = compile(build_code, build_file_name, 'exec')
    eval(bytecode, env)


def make_build_environment(default_name, path, bg):
    env = {}

    def make_wrapper(klass_):
        def defwrapper(**kwargs):
            if 'name' not in kwargs:
                kwargs['name'] = default_name
            kwargs['path'] = path
            instance = klass_.define(**kwargs)
            bg.add_node(instance, raise_if_duplicate=True)
        return defwrapper

    for token, klass in target_type_registry.items():
        env[token] = make_wrapper(klass)
    return env


def find_root():
    for path, files in ancestor_dirs():
        if ROOT_FILE in files:
            return path
    msg = 'failed to find build-system root (searching for "%s")' % (ROOT_FILE,)
    raise Exception(msg)


arguments_ = docopt(__doc__, argv=sys.argv[1:])
exit_status = None
try:
    result_type, result_data = run_one(main, arguments_)
except Exception as err:
    logger.critical(err)
    exit_status = 1
else:
    if result_type == ET_VALUE:
        exit_status = result_data
    elif result_type == ET_EXCEPTION:
        from pprint import pprint
        print('BEGIN run_one exception')
        pprint(result_data)
        print('END run_one exception')
        exit_status = 1

assert exit_status is not None
sys.exit(exit_status)
