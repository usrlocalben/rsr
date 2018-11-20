import logging
import sys

from rqdq.ox.config import *
from rqdq.ox.exception import DoesNotExist
from twisted.python import log

# logging setup
logger = logging.getLogger()
log.startLogging(sys.stdout)


def check_it_yo(bg, target):
    resolved_deps = set()

    def resolve(node, stack):
        if len(stack) > MAX_DEP_LEVEL:
            for item in reversed(stack):
                logger.critical('resolving %s', item)
            msg = 'reached dep level limit, possible cycle?'
            raise Exception(msg)
        for dep_selector in node.deps:
            try:
                dep_node = bg.find_one(dep_selector)
            except DoesNotExist:
                msg = ('target "%s" not found, needed by "%s"' % (dep_selector, node.selector))
                raise Exception(msg)
            resolve(dep_node, stack + [dep_node.selector])
        resolved_deps.add(node.selector)

    logger.debug('starting dependency resolution for "%s"', target.selector)
    resolve(target, [target.selector])
    logger.info('dependency resolution for "%s" completed, %d targets found',
                target.selector, len(resolved_deps))
    return sorted(list(resolved_deps))


def graph_it_yo(bg, target):
    from graphviz import Digraph
    g = Digraph('deptree', filename='deptree.gv')
    g.attr(size='24,24')
    g.node_attr.update(color='lightblue2', style='filled')

    nodes = check_it_yo(bg, target)
    for node in nodes:
        if node.startswith('3rdparty'):
            continue
        node = bg.find_one(node)
        for dep in node.deps:
            if dep.startswith('3rdparty'):
                continue
            from_name = node.selector.replace(':', '_')
            to_name = dep.replace(':', '_')
            from_name = from_name[4:]
            to_name = to_name[4:]

            if from_name.startswith('rml'):
                from_name = from_name[4:8]
            if to_name.startswith('rml'):
                to_name = to_name[4:8]

            g.edge(from_name, to_name)
    g.render()
