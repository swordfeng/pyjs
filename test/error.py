def willraise():
    raise TypeError('test')


def willraiseUpperframe(n):
    if not (n > 0):
        willraise()
    else:
        willraiseUpperframe(n - 1)
