def canonicalize_one(text, path=None):
    if text.startswith(':'):
        if path:
            return path + text
        else:
            msg = 'refusing to canonicalize "%s" without path' % (text,)
            raise ValueError(msg)

    if ':' in text:
        return text
    else:
        default_target_name = text.split('/')[-1]
        fixed = '%s:%s' % (text, default_target_name)
        return fixed


def canonicalize(data, path=None):
    if isinstance(data, (list, tuple)):
        return [canonicalize_one(item, path) for item in data]
    elif isinstance(data, str):
        return canonicalize_one(data, path)
    else:
        msg = 'don\'t know how to canonicalize %s' % (type(data),)
        raise Exception(msg)

