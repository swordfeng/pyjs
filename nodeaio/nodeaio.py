#!/usr/bin/python3
import asyncio
import threading
class LoopThread(threading.Thread):
    def __init__(self):
        super(LoopThread, self).__init__()
        self.loop = asyncio.new_event_loop()
    def run(self):
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()
    def stop(self):
        self.loop.call_soon_threadsafe(self.loop.stop)

loopt = LoopThread()
loopt.start()

class Thenable:
    def __init__(self, coro):
        self.resolve_handlers = []
        self.reject_handlers = []
        self.done = False
        self.result = None
        self.exception = None
        self.coro = coro
    def then(self, resolve, reject):
        if self.done:
            if self.exception != None:
                reject(self.exception)
            else:
                resolve(self.result)
        else:
            self.resolve_handlers.append(resolve)
            self.reject_handlers.append(reject)
    async def run(self):
        try:
            self.result = await self.coro
        except BaseException as e:
            self.exception = e
        self.done = True
        # should have no exceptions thrown from node.js?
        if self.exception != None:
            for handler in self.reject_handlers:
                handler(self.exception)
        else:
            for handler in self.resolve_handlers:
                handler(self.result)
        # in case of circular reference
        del self.resolve_handlers
        del self.reject_handlers

def ensure_coroutine(coro):
    promise = Thenable(coro)
    loopt.loop.call_soon_threadsafe(asyncio.ensure_future, promise.run())
    return promise
