# pyjs
[![CircleCI](https://circleci.com/gh/swordfeng/pyjs/tree/master.svg?style=svg)](https://circleci.com/gh/swordfeng/pyjs/tree/master)  
Pyjs - Call Python code from Node.js in process  
Work in progress

### TODO
+ Error handling for Javascript functions called in Python
+ Check Python functions' result for each time
+ Add assertions in native functions
+ Make convenient ways to do python add, sub, ... expressions in javascript
+ Resolve circular reference between Python and JavaScript (may be difficult!)
+ Fix python read from stdin in node repl (get EOF error currently) - maybe use asyncio?
