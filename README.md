# pyjs
Pyjs - Call Python code from Node.js in process  
Under heavy development

### TODO
+ Error handling for Javascript functions called in Python
+ Check Python functions' result for each time
+ Add assertions in native functions
+ Make convenient ways to do python add, sub, ... expressions in javascript
+ Resolve circular reference between Python and JavaScript (may be difficult!)
+ Fix python read from stdin in node repl (get EOF error currently)
+ ~~Use `SetNamedPropertyHandler` instead of set seperately for each PyObject~~
+ ~~Use `SetCallAsFunctionHandler` instead of wrap it for each callable PyObject~~
+ ~~RAII wrapper for Python objects~~
+ ~~Call JavaScript from Python side~~
+ ~~Multithread support (check calling from another thread is worked well)~~
