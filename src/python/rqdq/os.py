import errno
import os


def maybe_mkdir(dir_name):
    try:
        os.mkdir(dir_name)
        return True
    except OSError as err:
        if err.errno == errno.EEXIST:
            pass
        else:
            raise
    return False


def ancestor_dirs(path=None):
    cwd = os.getcwd()
    try:
        if path:
            os.chdir(path)
        while True:
            directory = os.listdir('.')
            yield os.getcwd(), directory
            if os.getcwd() == '/':
                # reached the root dir
                break
            os.chdir('..')
    finally:
        os.chdir(cwd)


def split_extension(name, raise_if_missing=True):
    try:
        dotpos = name.rindex('.')
    except ValueError:
        if raise_if_missing:
            msg = 'src "%s" does not have an extension' % (name,)
            raise Exception(msg)
        else:
            return name, ''
    return name[0:dotpos], name[dotpos + 1:]
