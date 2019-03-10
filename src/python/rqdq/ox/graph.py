import logging

from rqdq.ox.exception import DoesNotExist
from rqdq.ox.selector import canonicalize

logger = logging.getLogger()


class BuildGraph(object):
    def __init__(self):
        self._db = {}

    def add_node(self, node, raise_if_duplicate=True):
        existing = self._db.get(node.selector)
        if existing and raise_if_duplicate:
            msg = 'attempted to add duplicate node "%s"' % (node.selector,)
            raise Exception(msg)

        node._bg = self  # XXX yuck..
        self._db[node.selector] = node
        logger.debug('discovered target %s', node.selector)

    def find_one(self, selector):
        selector = canonicalize(selector)
        try:
            return self._db[selector]
        except KeyError:
            msg = 'node "%s" not found' % (selector,)
            raise DoesNotExist(msg)

    def find_many(self, query):
        out = []
        query = query.replace('\\', '/')  # XXX windows fix
        if query.endswith('::'):
            prefix = query[0:-2]
            for id_ in self._db:
                if id_.startswith(prefix + ':'):
                    out.append(id_)
                elif id_.startswith(prefix + '/'):
                    out.append(id_)
        else:
            try:
                node = self.find_one(query)
            except DoesNotExist:
                pass
            else:
                out.append(node.selector)
        return out


class Target(object):
    goals = ()

    def __init__(self, path, name, deps):
        self._path = path
        self._name = name
        self._deps = deps

    @property
    def selector(self):
        return '%s:%s' % (self._path, self._name)

    @property
    def deps(self):
        for dep in self._deps:
            yield dep

    @property
    def rdeps(self):
        bg = getattr(self, '_bg', None)
        if not bg:
            msg = 'target node not linked to a BuildGraph?'
            raise Exception(msg)
        for dep in self._deps:
            node = bg.find_one(dep)
            yield node


target_type_registry = {}


def register_target_type(klass):
    if klass.token in target_type_registry:
        msg = 'target type with token "%s" already registered' % (klass.token,)
        raise Exception(msg)
    target_type_registry[klass.token] = klass
