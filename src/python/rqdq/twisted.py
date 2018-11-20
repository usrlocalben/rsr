from distutils.spawn import find_executable
import sys

from twisted.internet import defer
from twisted.internet import reactor
from twisted.internet.defer import Deferred
from twisted.internet.error import ProcessDone
from twisted.internet.error import ProcessTerminated
from twisted.internet.protocol import ProcessProtocol
from twisted.python.failure import Failure


class Nothing(object):
    pass


ET_EXCEPTION = 'EXCEPTION'
ET_VALUE = 'VALUE'

_exit_result = Nothing


def run_one(func, *args, **kwargs):
    global _exit_result

    @defer.inlineCallbacks
    def work(*args, **kwargs):
        global _exit_result
        try:
            code = yield func(*args, **kwargs)
        except BaseException as err:
            _exit_result = (ET_EXCEPTION, sys.exc_info())
        else:
            _exit_result = (ET_VALUE, code)
        reactor.stop()

    reactor.callWhenRunning(work, *args, **kwargs)
    reactor.run()

    if _exit_result is Nothing:
        msg = 'reactor stopped before root function'
        raise Exception(msg)
    else:
        return _exit_result


'''
XXX python 3 does not support this syntax for raise
        type_, data = _exit_result
        if type_ == ET_VALUE:
            return data
        elif type_ == ET_EXCEPTION:
            raise data[1], None, data[2]
        else:
            msg = 'unknown exit type "%s"' % (type_,)
            raise Exception(msg)
'''


class PendingEvent(object):
    def __init__(self):
        self.listeners = []
        self._result = None

    def deferred(self):
        if self._result is not None:
            ok, data = self._result
            if ok:
                return defer.succeed(data)
            else:
                return defer.fail(data)
        d = defer.Deferred()
        self.listeners.append(d)
        return d

    def callback(self, result):
        self._result = (True, result)
        l = self.listeners
        self.listeners = []
        for d in l:
            d.callback(result)

    def errback(self, result=None):
        if result is None:
            result = Failure()
        self._result = (False, result)
        l = self.listeners
        self.listeners = []
        for d in l:
            d.errback(result)

    def run(self, func, *args, **kwargs):
        @defer.inlineCallbacks
        def task():
            try:
                result = yield func(*args, **kwargs)
            except Exception as err:
                self.errback(err)
            else:
                self.callback(result)
        reactor.callWhenRunning(task)


class SubprocessProtocol(ProcessProtocol):
    return_code = None
    out_text = b''
    error_text = b''

    def __init__(self, args):
        self._args = args

    def connectionMade(self):
        self.d = Deferred()

    def outReceived(self, data):
        self.out_text += data

    def errReceived(self, data):
        self.error_text += data

    def processEnded(self, reason):
        if reason.check(ProcessDone):
            result = (0, self.out_text, self.error_text)
            self.d.callback(result)
        elif reason.check(ProcessTerminated):
            result = (reason.value.exitCode, self.out_text, self.error_text)
            self.d.callback(result)
        else:
            # logger.debug('processEnded with %s', str(reason))
            self.d.errback(reason)


def async_check_output(args, ireactorprocess=None):
    if ireactorprocess is None:
        from twisted.internet import reactor
        ireactorprocess = reactor

    pprotocol = SubprocessProtocol(args)

    exe = find_executable(args[0])
    if not exe:
        raise Exception('could not find "%s" on path?' % args[0])
    ireactorprocess.spawnProcess(pprotocol, exe, args)
    return pprotocol.d
