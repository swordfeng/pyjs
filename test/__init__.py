import time

a = 5.0
def function1():
    return 'hello'

def function2(x):
    return x * 2 + 1

def callFunction(func, arg):
    return func(arg)

def waitThenCall(secs, func):
    time.sleep(secs)
    func()

def willraise():
    raise TypeError('test')


def willraiseUpperframe(n):
    if not (n > 0):
        willraise()
    else:
        willraiseUpperframe(n - 1)

class testclass:
    a = 2.0
