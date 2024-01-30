# process_cpp

C++ library for executing shell commands in Linux with support for
both stdout and stderr output, along with stdin input.

[![C++ CI build](https://github.com/oskarirauta/process_cpp/actions/workflows/build.yml/badge.svg)](https://github.com/oskarirauta/process_cpp/actions/workflows/build.yml)

### fork of cpp-subprocess

Project started as it's own project, but is very similar to [cpp-subprocess](https://github.com/tsaarni/cpp-subprocess)
of Tero Saarni; this is because I was initially writing my completely
own design of similar application (without stdin input) but had some difficulties
with it; after a while I found cpp-subprocess and with some minimal changes
I was able to get it to work (had to rename stdin to in() and stdout to out() as
in my compiler, names stdin and stdout seemed to be reserved and caused strange
behavior) and it seemed very simple and was written in so little amount of
code that I decided not to re-invent wheel; and so, I adopted parts of code
from that project; but to not steal this code, I made this repository a fork
of original code instead of secretly using someone else's code.

Yet, process_cpp is not a direct replacement of cpp-subprocess, even though
it has all the same features and some more; it is used differently. Not a lot
differently, but still, to replace cpp-subprocess with process_cpp, it will
require modification on host project.

### Streams

stdout can be seen by using class as a input stream. There is also a
subscript operator helping to select stderr; also stdin works like a
output stream directly pointing to class.
You can also redirect stdin from another process execution.

### Examples

Same examples are available in provided example code, but
here they are again with explanations.

```
process_t proc("ls");
std::cout << "stdout:\n" << proc << "\nexit code: " << proc.status() << std::endl;
```

In this example, we use ls command to list contents of current directory.
As seen, output is received with a stream.
Exit code is available with method status();

```
process_t proc("/bin/sh", { "-c", "./test.sh" );
std::cout << "stdout:\n" << proc[STREAM_OUT] << std::endl;
std::cout << "stderr:\n" << proc[STREAM_ERR] << std::endl;
std::cout << "exit code: " << proc[STREAM_STATUS] << std::endl;
```

In this example, we use a test.sh script, which outputs to both;
stdout and stderr, and gives different exit code as 0- we use
subscript operators to decide stream that we want to use.
proc[STREAM_OUT] could be just proc, and status could be
given with status() method as in previous example, but this
example lists all possible streams availbale with subscript
operator.

```
process_t *proc = new process_t("/bin/lh");
std::cout << "stdout:\n" << (*proc)[STREAM_OUT] << std::endl;
delete proc;
```

This example shows how to use process as a pointer and how
to use subscript operator with pointer. Do not forget to use
delete.

```
process_t *proc = new process_t("/bin/ls");
std::cout << "stdout:\n" << *proc << std::endl;
delete proc;
```

This is same example, but different output method for stdout..

```
#if __cplusplus > 201803L
process_t proc("ls", "-l");
#else
process_t proc("ls", { "-l" });
#endif
```

if you use c++20 standard (or newer), you do not need to give
args as a vector, you can just add them as series of strings.
This method uses parameter pack using std::convertible_to<>
which is not available in standards earlier than c++20.

Note: as this uses throws.cpp, which requires c++20, you
actually cannot choose any of lower standards.

```
process_t proc("sort", { "-r" });
proc >> "a\nb\nc" >> std::endl;
```

in this example, we use sort command to reverse what
is sent to stdin, stdin works the same way; except...
this time direction is changed to >> - cpp-subprocess
also had this feature; though in different way-
there was a close() method that was used necessary input
was given, in process_cpp std::endl closes, that's why
\n is used to separate rows.

```
process_t proc("sort", { "-r" });
process_t cat("cat", {}, sort);
cat >> "a\nb\nc" >> std::endl;
std::cout << sort << std::endl;
```

This example uses 2 processes, other is sending it's output
to other's input. Here we use cat, send a b c to it,
which then is directed to sort that reverses it.

```
process_t proc("echo", { "hello world" });
std::string out = proc;
int code = proc;
std::cout << out << "\ncode: " << code << std::endl;
```

in this example, we turn process_t to string and int.
string contains stdout and int contains exit code.

```
process_t proc(...);
std::string out = proc[STREAM_OUT];
std::string err = proc[STREAM_ERR];
int code = proc[STREAM_OUT];
```

This works also with subscript operator. When
retrieving exit code with subscript operator,
it doesn't matter which stream you use, all give
same exit code.

### Exiting / zombie processes

At end of process, if process_t is not destroyed, a zombie (defunct) process
leaves hanging around. You should call process_t::status() even if you are
not interested in status code from execution.

example:
```
process_t *proc = new process_t(...);
...
proc -> status();
```

calling status() ends and scrubs executed process completely, this means
that after calling status() - there is no way anymore to interact with
your process anymore, any sign of it's execution from system disappears.
This also removes zombie process that can be seen with ```ps -ax``` command.

status() is automatically called on destruction of process_t; and it is not
dangerous that zombie processes exist. They disappear from system anyway
when your main program exits; and they simply are dead processes, but
ofcourse, if you run process_t a lot.. It won't look very nice in the list
of processes by system. And even that they are dead left-overs; I think
it's better to get rid of them.

### Methods

Instead of using subscript operator to get desired stream, you
can alternatively use methods out() and err().
Method status() outputs exit code, and waits process to finish.

### Depencies

process_cpp depends on [throws_cpp](https://github.com/oskarirauta/throws_cpp)
which is a small utility to throw with stream.

### Importing
 - import throws_cpp as a submodule to throws
 - import process_cpp as a submodule to process
 - include throws_cpp's Makefile.inc and logger_cpp's Makefile.inc in your Makefile
 - link with THROWS_OBJS and PROCESS_OBJS

or check example Makefile, notice that default for PROCESS_DIR is process instead of process_cpp.
Don't forget to #include "process.hpp" in your project's code where you plan to use it.
Remember to clone with recursive submodules, using --recursive-submodules argument.

Note: with default build system, you need objs directory in your project's root, this is path
where object files are built.

### Example

Sample code is provided.

### Other

MIT-license
