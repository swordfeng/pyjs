# pyjs
[![CircleCI](https://circleci.com/gh/swordfeng/pyjs/tree/master.svg?style=shield)](https://circleci.com/gh/swordfeng/pyjs/tree/master)
[![AppVeyor](https://ci.appveyor.com/api/projects/status/ms9u389g3i3kwpjd/branch/master?svg=true)](https://ci.appveyor.com/project/swordfeng/pyjs/branch/master)  
Pyjs - Call Python code from Node.js in process  

### TODO
+ Error handling for Javascript functions called in Python
+ Check Python functions' result for each time
+ Add assertions in native functions
+ Make convenient ways to do python add, sub, ... expressions in javascript
+ Resolve circular reference between Python and JavaScript (may be difficult!)
+ Fix python read from stdin in node repl (get EOF error currently) - maybe use asyncio?
