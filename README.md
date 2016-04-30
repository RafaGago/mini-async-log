
Minimal Asynchronous Logger (MAL)
-----------
A non feature-bloated asynchronous data logger. Sponsored by my employer **Diadrom AB.**

We just wanted an asynchronous logger that can be used from many dynamically loaded libraries without doing link-time hacks like linking static and hiding symbols and some other "niceties".

After having maintained a slightly modified fork of google log (glog) and given the fact that this is a very small project we decided that existing wheels weren't round enough.

## Design rationale ##

 - Simple. Not over abstracted and feature bloated, explicit, easy to figure out what the code is doing, easy to modify.
 - Very low latency. Fast for the caller. Lock-free.
 - Asynchronous (synchronous calls can be made on top for special messages, but they are way slower than using a synchronous logger in the first place).
 - Minimum string formatting in the calling thread for the most common use cases.
 - Keeps ordering between threads.
 - Doesn't use thread-local-storage, the user threads are assumed as external and no extra info is attached to them.

## Various features ##

 - Targeting g++4.7 and VS 2010
 - Boost dependencies just for parts that will eventually go to the C++ standard.
 - No singleton by design, usable from dynamically loaded libraries. The user provides the instance either explicitly or by a global function (Koenig lookup).
 - Suitable for soft-realtime work. Once it's initialized the fast-path can be clear from heap allocations if properly configured.
 - File rotation-slicing (needs some external help at initialization time until std::filesystem is implemented on some compilers, see below).
 - One conditional call overhead for inactive severities.
 - Lazy parameter evaluation (as usual with most logging libraries).
 - No ostreams (a very ugly part of C++ for my liking), just format strings checked at compile time (if the compiler supports it) with type safe values. An on-stack ostream adapter is available as a last resort, but its use is more verbose and has more overhead.
 - The log severity can be externally changed outside of the process. The IPC mechanism is the simplest, the log worker periodically polls some file descriptors when idle (if configured to).
 - Small, you can actually compile it as a part of your application.
 
## How does it work ##

It just borrows ideas from many of the loggers out there.

As with any half-decent asynchronous logger its main objetive is to be as fast and to have as low latency as possible for the caller thread.

When the user is to write a log message, the logging frontend task is to encode the passed data to a memory chunk which the backend can then decode, format and write. No string formatting occurs on the caller thread.

The format string is required to be a literal (compile time constant), so when encoding something like the entry below...

> log_error ("File:a.cpp line:8 This is a string that just displays the next number {}, int32_val);

...the memory requirements are just a pointer to the format string and a deep copy of the integer value. The job of the caller is just to encode some bytes to a memory chunk and to insert the chunk into a queue.

The queue is a custom modification of two famous lockfree queues of Dmitry Djukov (kudos to this genious) for this particular MPSC case. The queue is a blend of a fixed capacity and fixed element size array based preallocated queue and an intrusive node based dynamically allocated queue which can contain elemnts of heterogenous sizes (which can be disabled). The resulting queue is still linearizable.

The worker thread pops from the queue, decodes the message and writes the data. Any time consuming operation is masked with the slow file IO.

The format string is type-safe and validated at compile time for compilers that support "constexpr" and "variadic template parameters" available. Otherwise the errors are caught at run time on the logged output.

There are other features, as to block the caller thread until some message has been written (as in an asynchronous logger) or as to do C++ stream formatting on the caller thread. Both of these are features should be better avoided if possible, as they defeat the purpose of this desing and are hacks on top of this design.

> see this [example](https://github.com/RafaGago/mini-async-log/blob/master/example/overview.cpp) that more or less shows all available features.

## Benchmarks ##

I used to have some benchmarks here, but as the benchmarks were mostly an apples-to-oranges comparison (e.g. comparing with glog which is half-synchronous) I just removed them.

With the actual performance and if the logger is configured to generate the timestamps at the client/producer side it takes longer for a client thread to retrieve the timestamp with std::chrono/boost::chrono than to build and enqueue a simple message composed of e.g. a format string and three integers.

> [These were the benchmarks that I had](https://github.com/RafaGago/mini-async-log/blob/master/example/benchmark.cpp).

## File rotation ##

The library can rotate fixed size log files.

Using the current C++11 standard files can just be created, modified and deleted. There is no way to list a directory, so the user is required to pass at start time the list of files generated by previous runs. I may add support for boost::filesystem/std::filesystem, but just as an optional (but ready to use) external code, so everyone can skip this heavy dependency. There is an example using boost::filesystem in the "/extras" folder

> There is an [example](https://github.com/RafaGago/mini-async-log/blob/master/example/rotation.cpp) here.

## Initialization ##

The library isn't a singleton, so the user should provide a reference to the logger instance. Even if many modules call the initialization function only one of them will succeed.

There are two methods to get the instance when enqueuing a log entry, one is to provide it explicitly and the other one is by providing it on a global function.

If no instance is provided, the global function "get_mal_logger_instance()" will be called without being namespace qualified, so you can use Koenig lookup/ADL and provide it from there. This happens when the user calls the macros with no explicite instance suffix, as e.g. "log_error(fmt string, ...)".

To provide the instance explictly the macros with the "_i" suffix need to be called, e.g. "log_error_i(instance, fmt_string, ...)"

The name of the function can be changed at compile time, by defining MAL_GET_LOGGER_INSTANCE_FUNCNAME.

Be aware that it's dangerous to have a dynamic library or executable loaded multiple times logging to the same folder and rotating files each other. Workarounds exists, you can prepend the folder name with the process name and ID, disable rotation and manage rotation externally (e.g. by using logrotate), etc.

## Termination ##

The worker blocks on its destructor until its work queue is empty when normally exiting a program.

When a signal is sent you can call the frontend function  [on termination](https://github.com/RafaGago/mini-async-log/blob/master/include/mal_log/frontend.hpp). This will early interrupt any synchronous calls you made.

## Errors ##

As for now, every function returns a boolean if it succeeded or false if it didn't. A filtered out/below severity call returns true.

The only possible failures are either to be unable to allocate memory for a log entry, an asynchronous call that was interrupted by "on_termination" or a string byte that was about to be deep copied but would overflow the length variable (mostly a bug, I highly doubt that someone will log messages that take 4GB).

The functions never throw.

## Restrictions ##

 1. Just ASCII.
 2. Partial C++ ostream support. (not sure if it's a good or a bad thing...). Swapping logger in an existing codebase may not be worth the effort in some cases.
 3. Limited formatting abilities (it can be improved with more parser complexity).
 4. No way to output runtime strings/memory regions without deep-copying them. This is inherent to the fact that the logger is asynchronous and that I prefer to avoid hacking the reference count on "shared_ptr" using placement new. I avoid this because I think that ouputting memory regions to a log file through a deferred (asynchronous) logger is wrong by design in most if not all cases.
 5. Some ugly macros, but unfortunately the same syntax can't be achieved in any other way AFAIK.
 6. Format strings need to be literals. A const char* isn't enough (constexpr can't iterate them at compile time).

## Compiler macros ##

Those that are self-explanatory won't be explained.

 - *MAL_GET_LOGGER_INSTANCE_FUNC*: See the "Initialization" chapter above.
 - *MAL_STRIP_LOG_SEVERITY*: Removes the entries of this severity and below at compile time. 0 is the "debug" severity, 5 is the "critical" severity. Stripping at level 5 leaves no log entries at all. Yo can define e.g. MAL_STRIP_LOG_DEBUG, MAL_STRIP_LOG_TRACE, etc. instead. If you define MAL_STRIP_LOG_TRACE all the severities below will be automatically defined for you (in this case MAL_STRIP_LOG_DEBUG).
 - *MAL_DYNLIB_COMPILE*: Define it when compiling as a dynamic library/shared object.
 - *MAL_DYNLIB*: Define it when using MAL as a dynamic library. Don't define it if you are static linking or compiling the library with your project.
 - *MAL_CACHE_LINE_SIZE*: The cache line size of the machine you are compiling for. This is just used for data structure padding. 64 is defaulted when undefined.
 - *MAL_USE_BOOST_CSTDINT*: If your compiler doesn't have <cstdint> use boost.
 - *MAL_USE_BOOST_ATOMIC*
 - *MAL_USE_BOOST_CHRONO*
 - *MAL_USE_BOOST_THREAD*
 - *MAL_NO_VARIABLE_INTEGER_WIDTH*: Integers are encoded ignoring the number trailing bytes set to zero, not based on its data type size. So when this isn't defined e.g. encoding an uint64 with a value up to 255 takes one byte (plus 1 byte header). Otherwise all uint64 values will take 8 bytes (plus header), so encoding is less space efficient in this way but it frees the CPU and allows the compiler to inline more.
 
## Linux compilation ##

You can compile the files in the "src" folder and make a library or just use compile everything in your project.

Otherwise you can use the makefile in the "/build/linux" folder, one example of command invocation could be:

    make CXXFLAGS="-DNDEBUG -DMAL_USE_BOOST_THREAD -DMAL_USE_BOOST_CHRONO -DBOOST_ALL_DYN_LINK -DBOOST_CHRONO_HEADER_ONLY" LDLIBS="-lboost_thread" CXX="arm-linux-gnueabihf-g++"
    
REMEMBER: That the if the library is compiled with e.g. "MAL_USE_BOOST_THREAD" and "MAL_USE_BOOST_CHRONO" the client code must define them too. This needs to be improved in future versions by directly modifying the include files.

## Windows compilation ##

There is a Visual Studio 2010 Solution the "/build/windows" folder, but you need to do a step before opening. 

If you don't need the Boost libraries you should run the "build\windows\mal-log\props\from_empty.bat" script. If you need them you should run the "build\windows\mal-log\props\from_non_empty.bat" script. 

If you don't need the Boost libraries you can open and compile the solution, otherwise you need to edit (with a text editor) the newly generated file ""build\windows\mal-log\props\mal_dependencies.props" before and to update the paths in the file. You can do this through the Visual Studio Property Manager too.

> Written with [StackEdit](https://stackedit.io/).
