
Minimal Asynchronous Logger (MAL)
-----------
A non feature-bloated asynchronous data logger. Sponsored by my former employer
**Diadrom AB.**

We just wanted an asynchronous logger that can be used from many dynamically
loaded libraries without doing link-time hacks like linking static and hiding
symbols and some other "niceties".

After having maintained a slightly modified fork of google log (glog) and given
the fact that this is a very small project we decided that existing wheels
weren't round enough.

## Design rationale ##

 - Simple. Not over abstracted and feature bloated, explicit, easy to figure out
   what the code is doing, easy to modify.
 - Very low latency. Fast for the caller. Lock-free.
 - Asynchronous (synchronous calls can be made on top for special messages, but
   they are way slower than using a synchronous logger in the first place).
 - Minimum string formatting in the calling thread for the most common use cases.
 - Keeps ordering between threads.
 - Doesn't use thread-local-storage, the user threads are assumed as external
   and no extra info is attached to them.

## Various features ##

 - Targeting g++4.7 and VS 2010
 - Boost dependencies just for parts that will eventually go to the C++
   standard.
 - No singleton by design, usable from dynamically loaded libraries. The user
   provides the instance either explicitly or by a global function (Koenig lookup).
 - Suitable for soft-realtime work. Once it's initialized the fast-path can be
   clear from heap allocations if properly configured.
 - File rotation-slicing (needs some external help at initialization time until
   std::filesystem is implemented on some compilers, see below).
 - One conditional call overhead for inactive severities.
 - Lazy parameter evaluation (as usual with most logging libraries).
 - No ostreams (a very ugly part of C++ for my liking), just format strings
   checked at compile time (if the compiler supports it) with type safe values.
   An on-stack ostream adapter is available as a last resort, but its use is
   more verbose and has more overhead.
 - The log severity can be externally changed outside of the process. The IPC
   mechanism is the simplest, the log worker periodically polls some file
   descriptors when idle (if configured to).
 - Small, you can actually compile it as a part of your application.

## Performace ##

These are some test I have done for fun to see how this code is aging.

> [Here is the benchmark code.](https://github.com/RafaGago/mini-async-log/blob/master/example/benchmark/main.cpp).

### Some words about measuring an asynchronous loggers latency in a non-RTOS

IMO this is almost pointless when the logger is being written for a non-RTOS,
why? because the thread can be suspended-preempted by the OS scheduler when it
wishes for the number of time quantas that it wants. In a non-RTOS the main
source of latency jitter is the OS scheduler itself.

Then some of these authors may be measuring latencies with a CPU or thread
clock, which stops counting when the thread gets suspended or preempted. This
means that if one of these threads takes a mutex for 10seconds the clock is
discounting those 10seconds (excluding the syscall latencies).

If an asynchronous logger is running in an RTOS one doesn't care a lot about
worst case latency because if the logger is decently written the worst-case will
be masked by the Scheduler, so a fast aslogger

### Some words about guaranteed delivery on asynchronous logger.

There is no such thing. If the logger is asynchronous it has a buffer. If it has
a buffer the buffer has a limited size (either bounded or the system maximum)
and can be overflown: an asynchronous logger is non-guaranteed by definition.

If the logger starts having blocking behavior when the queue is full it isn't
an asynchronous logger.

As a matter of fact I added a blocking mode on full queue just for the
benchmarks.

### Test metodology

The benchmark is _very_ synthetic and very far from a real case, it's just about
enqueuing 2 million messages as fast as possible. This is very unlikely to
happen on real life.

Each logger tries to be configured to have similar settings, e.g. the loggers
that have queues all are set to a generous 1MB.

There are throughput and latency tests.

The throughput tests are just throwing messages like crazy to the logger.

The latency tests are almost the same but they are measured with both a CPU
clock and a wall clock. The wall clock gives an idea of the real behavior of
the logger. Keep in mind that these clock sources are not that good in the
range in which we are measuring latency. As a matter of fact in my system
getting the timestamp takes as long as enqueing the message with mal.

In the latency tests the mean, standard deviation, best and worse case of those
2 million log entries are presented. The standard deviation gives an idea of
the jitter. The less the better.

### Results

These are done against the master branch of each project on March 2017.

- spdlog async: spdlog in asynchronous mode. (1MB/32) queue entries.

- spdlog async: spdlog in synchronous mode. (1MB/32) queue entries.

- glog: Google log (half synchronous).

- mal heap: mal using only the heap (unbounded).

- mal hybrid: mal using a hybrid bounded queue + heap strategy (1MB/16 queue, 32
           bytes each element).

- mal sync: mal using a bounded queue and blocking on full queue (synchronous
           mode just added for fun on this test). (1MB queue, 32 bytes each
           element).

- mal bounded: mal with a bounded queue (1MB), will reject _a lot_ of messages.
           (1MB queue, 32 bytes each element).

#### Throughput

|logger|threads|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|all threads(s)|alloc faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|spdlog async|1|4.131|484.089|0.000|0.000|4.131|0|
|mal heap|1|0.270|7416.506|2.548|784.979|0.270|0|
|mal_hybrid|1|0.274|7305.949|4.228|473.018|0.274|0|
|spdlog sync|1|4.252|470.380|0.000|0.000|4.252|0|
|mal sync|1|2.774|721.014|2.814|710.744|2.774|0|
|glog|1|5.414|369.407|5.414|369.406|5.414|0|
|mal bounded|1|0.079|25311.650|0.118|16977.568|0.079|1907864|
|spdlog async|2|3.955|505.690|0.000|0.000|7.675|0|
|mal heap|2|0.270|7408.917|2.572|777.706|0.478|0|
|mal_hybrid|2|0.319|6262.800|2.635|758.949|0.632|0|
|spdlog sync|2|3.994|500.784|0.000|0.000|6.761|0|
|mal sync|2|3.181|628.828|3.220|621.079|6.222|0|
|glog|2|10.156|196.935|10.156|196.930|20.308|0|
|mal bounded|2|0.048|41458.296|0.088|22708.886|0.095|3840185|
|spdlog async|4|4.047|494.136|0.000|0.000|11.530|0|
|mal heap|4|0.263|7608.924|2.530|790.608|0.932|0|
|mal_hybrid|4|0.265|7551.166|3.417|585.374|0.865|0|
|spdlog sync|4|3.767|530.945|0.000|0.000|12.112|0|
|mal sync|4|2.817|710.090|2.856|700.172|11.009|0|
|glog|4|9.381|213.190|9.381|213.186|37.408|0|
|mal bounded|4|0.043|46897.441|0.381|5245.031|0.127|5807417|
|spdlog async|8|3.823|523.180|0.000|0.000|18.860|0|
|mal heap|8|0.297|6733.570|2.567|779.163|1.854|0|
|mal_hybrid|8|0.216|9273.191|2.744|728.947|1.393|0|
|spdlog sync|8|3.692|541.648|0.000|0.000|20.720|0|
|mal sync|8|3.243|616.685|3.283|609.238|25.492|0|
|glog|8|9.446|211.732|9.446|211.729|74.907|0|
|mal bounded|8|0.044|45199.783|0.083|23993.906|0.197|7750046|
|spdlog async|16|3.513|569.282|0.000|0.000|38.941|0|
|mal heap|16|0.267|7484.848|3.613|553.542|2.963|0|
|mal_hybrid|16|0.220|9070.305|3.036|658.746|2.605|0|
|spdlog sync|16|3.806|525.422|0.000|0.000|37.973|0|
|mal sync|16|3.425|584.008|3.464|577.316|53.784|0|
|glog|16|9.641|207.457|9.641|207.453|153.137|0|
|mal bounded|16|0.059|33700.955|0.099|20239.996|0.205|9674540|

#### Latency

|logger|threads|clock|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|spdlog async|1|thread|1.460|0.853|0.000|252.780|
|spdlog async|1|wall|1.785|37.792|0.000|20209.250|
|mal heap|1|thread|0.376|0.177|0.000|16.649|
|mal heap|1|wall|0.219|0.312|0.000|89.500|
|mal hybrid|1|thread|0.361|0.191|0.000|25.667|
|mal hybrid|1|wall|0.217|0.234|0.000|26.000|
|spdlog sync|1|thread|1.462|0.737|0.000|142.858|
|spdlog sync|1|wall|2.039|426.854|0.000|601281.000|
|mal sync|1|thread|1.044|1.105|0.000|73.382|
|mal sync|1|wall|1.487|449.891|0.000|636241.750|
|glog|1|thread|3.089|4.721|0.000|380.873|
|glog|1|wall|2.858|4.630|0.000|463.500|
|mal bounded|1|thread|0.250|0.109|0.000|21.171|
|mal bounded|1|wall|0.113|0.185|0.000|117.500|
|spdlog async|2|thread|3.036|2.194|0.000|246.313|
|spdlog async|2|wall|4.431|1007.075|0.000|1001563.000|
|mal heap|2|thread|0.538|0.443|0.000|134.996|
|mal heap|2|wall|0.416|0.264|0.000|95.250|
|mal hybrid|2|thread|0.507|0.197|0.000|62.184|
|mal hybrid|2|wall|0.410|6.588|0.000|9289.750|
|spdlog sync|2|thread|2.887|2.304|0.000|247.076|
|spdlog sync|2|wall|3.496|309.296|0.000|401102.250|
|mal sync|2|thread|2.506|931.236|0.000|111743.877|
|mal sync|2|wall|2.418|111.868|0.000|1316963.000|
|glog|2|thread|6.860|7.034|0.000|1335.619|
|glog|2|wall|8.550|9.777|0.000|557.750|
|mal bounded|2|thread|0.284|0.145|0.000|32.087|
|mal bounded|2|wall|0.134|0.229|0.000|91.000|
|spdlog async|4|thread|3.151|2.768|0.000|241.843|
|spdlog async|4|wall|6.723|1930.163|0.000|1612825.500|
|mal heap|4|thread|0.541|0.475|0.000|99.428|
|mal heap|4|wall|0.552|37.856|0.000|12030.500|
|mal hybrid|4|thread|0.515|0.390|0.000|99.088|
|mal hybrid|4|wall|0.512|31.845|0.000|14463.250|
|spdlog sync|4|thread|2.759|2.550|0.000|265.018|
|spdlog sync|4|wall|6.557|1624.704|0.000|1603802.250|
|mal sync|4|thread|3.077|432.390|0.000|547437.234|
|mal sync|4|wall|7.186|1700.772|0.000|1202317.250|
|glog|4|thread|10.423|7.748|0.000|1265.811|
|glog|4|wall|17.478|24.739|0.000|7376.000|
|mal bounded|4|thread|0.231|0.260|0.000|88.922|
|mal bounded|4|wall|0.198|27.440|0.000|24007.000|
|spdlog async|8|thread|2.940|2.539|0.000|182.473|
|spdlog async|8|wall|11.958|3299.345|0.000|2402901.500|
|mal heap|8|thread|0.506|0.530|0.000|259.043|
|mal heap|8|wall|0.840|73.888|0.000|24006.750|
|mal hybrid|8|thread|0.489|0.445|0.000|186.431|
|mal hybrid|8|wall|0.831|84.114|0.000|36010.500|
|spdlog sync|8|thread|2.884|2.631|0.000|420.077|
|spdlog sync|8|wall|10.358|2622.885|0.000|1615655.250|
|glog|8|thread|11.931|6.573|0.000|668.045|
|glog|8|wall|38.170|1264.296|0.000|620448.250|
|mal sync|8|thread|3.123|559.169|0.000|790768.901|
|mal sync|8|wall|10.627|492.155|0.000|244116.500|
|mal bounded|8|thread|0.265|0.257|0.000|85.011|
|mal bounded|8|wall|0.176|28.332|0.000|24003.000|
|spdlog async|16|thread|2.835|2.702|0.000|314.704|
|spdlog async|16|wall|19.706|4739.116|0.000|3003505.500|
|mal heap|16|thread|0.480|0.451|0.000|279.728|
|mal heap|16|wall|1.381|134.147|0.000|48011.250|
|mal hybrid|16|thread|0.477|0.407|0.000|118.778|
|mal hybrid|16|wall|1.441|145.857|0.000|40010.250|
|spdlog sync|16|thread|3.068|2.919|0.000|307.053|
|spdlog sync|16|wall|17.496|4261.013|0.000|2606979.500|
|mal sync|16|thread|3.407|278.522|0.000|393827.636|
|mal sync|16|wall|26.736|2548.222|0.000|936022.250|
|glog|16|thread|12.068|5.910|0.000|164.158|
|glog|16|wall|71.821|88.898|0.000|9318.500|
|mal bounded|16|thread|0.261|0.374|0.000|138.830|
|mal bounded|16|wall|0.314|61.294|0.000|32003.250|

## How does it work ##

It just borrows ideas from many of the loggers out there.

As with any half-decent asynchronous logger its main objetive is to be as fast
 and to have as low latency as possible for the caller thread.

When the user is to write a log message, the logging frontend task is to encode
the passed data to a memory chunk which the backend can then decode, format and
write. No string formatting occurs on the caller thread.

The format string is required to be a literal (compile time constant), so when
encoding something like the entry below...

> log_error ("File:a.cpp line:8 This is a string that just displays the next number {}, int32_val);

...the memory requirements are just a pointer to the format string and a deep
copy of the integer value. The job of the caller is just to encode some bytes to
a memory chunk and to insert the chunk into a queue.

The queue is a custom modification of two famous lockfree queues of Dmitry
Djukov (kudos to this genious) for this particular MPSC case. The queue is a
blend of a fixed capacity and fixed element size array based preallocated queue
and an intrusive node based dynamically allocated queue which can contain
elements of heterogenous sizes (which can be disabled). The resulting queue is
still linearizable.

The worker thread pops from the queue, decodes the message and writes the data.
Any time consuming operation is masked with the slow file IO.

The format string is type-safe and validated at compile time for compilers that
support "constexpr" and "variadic template parameters" available. Otherwise the
errors are caught at run time on the logged output.

There are other features, as to block the caller thread until some message has
been written (as in an asynchronous logger) or as to do C++ stream formatting on
the caller thread. Both of these are features should be better avoided if
possible, as they defeat the purpose of this desing and are hacks on top of this
design.

> see this [example](https://github.com/RafaGago/mini-async-log/blob/master/example/overview/main.cpp)
that more or less shows all available features.

## File rotation ##

The library can rotate fixed size log files.

Using the current C++11 standard files can just be created, modified and
deleted. There is no way to list a directory, so the user is required to pass at
start time the list of files generated by previous runs. I may add support for
boost::filesystem/std::filesystem, but just as an optional (but ready to use)
external code, so everyone can skip this heavy dependency. There is an example
using boost::filesystem in the "/extras" folder

> There is an [example](https://github.com/RafaGago/mini-async-log/blob/master/example/rotation/main.cpp)
here.

## Initialization ##

The library isn't a singleton, so the user should provide a reference to the
logger instance. Even if many modules call the initialization function only one
of them will succeed.

There are two methods to get the instance when enqueuing a log entry, one is to
provide it explicitly and the other one is by providing it on a global function.

If no instance is provided, the global function "get_mal_logger_instance()" will
be called without being namespace qualified, so you can use Koenig lookup/ADL
and provide it from there. This happens when the user calls the macros with no
explicite instance suffix, as e.g. "log_error(fmt string, ...)".

To provide the instance explictly the macros with the "_i" suffix need to be
called, e.g. "log_error_i(instance, fmt_string, ...)"

The name of the function can be changed at compile time, by defining
MAL_GET_LOGGER_INSTANCE_FUNCNAME.

Be aware that it's dangerous to have a dynamic library or executable loaded
multiple times logging to the same folder and rotating files each other.
Workarounds exists, you can prepend the folder name with the process name and
ID, disable rotation and manage rotation externally (e.g. by using logrotate),
etc.

## Termination ##

The worker blocks on its destructor until its work queue is empty when normally
exiting a program.

When a signal is sent you can call the frontend function
[on termination](https://github.com/RafaGago/mini-async-log/blob/master/include/mal_log/frontend.hpp).
This will early interrupt any synchronous calls you made.

## Errors ##

As for now, every function returns a boolean if it succeeded or false if it didn't.
A filtered out/below severity call returns true.

The only possible failures are either to be unable to allocate memory for a log
entry, an asynchronous call that was interrupted by "on_termination" or a string
byte that was about to be deep copied but would overflow the length variable
mostly a bug, I highly doubt that someone will log messages that take 4GB).

The functions never throw.

## Restrictions ##

 1. Just ASCII.
 2. Partial C++ ostream support. (not sure if it's a good or a bad thing...).
    Swapping logger in an existing codebase may not be worth the effort in some
    cases.
 3. Limited formatting abilities (it can be improved with more parser
    complexity).
 4. No way to output runtime strings/memory regions without deep-copying them.
    This is inherent to the fact that the logger is asynchronous and that I
    prefer to avoid hacking the reference count on "shared_ptr" using placement
    new. I avoid this because I think that ouputting memory regions to a log
    file through a deferred (asynchronous) logger is wrong by design in most if
    not all cases.
 5. Some ugly macros, but unfortunately the same syntax can't be achieved in any
    other way AFAIK.
 6. Format strings need to be literals. A const char* isn't enough (constexpr
    can't iterate them at compile time).

## Compiler macros ##

Those that are self-explanatory won't be explained.

 - *MAL_GET_LOGGER_INSTANCE_FUNC*: See the "Initialization" chapter above.
 - *MAL_STRIP_LOG_SEVERITY*: Removes the entries of this severity and below at
    compile time. 0 is the "debug" severity, 5 is the "critical" severity.
    Stripping at level 5 leaves no log entries at all. Yo can define e.g.
    MAL_STRIP_LOG_DEBUG, MAL_STRIP_LOG_TRACE, etc. instead. If you define
    MAL_STRIP_LOG_TRACE all the severities below will be automatically defined
    for you (in this case MAL_STRIP_LOG_DEBUG).
 - *MAL_DYNLIB_COMPILE*: Define it when compiling as a dynamic library/shared
    object.
 - *MAL_DYNLIB*: Define it when using MAL as a dynamic library. Don't define it
   if you are static linking or compiling the library with your project.
 - *MAL_CACHE_LINE_SIZE*: The cache line size of the machine you are compiling
   for. This is just used for data structure padding. 64 is defaulted when
   undefined.
 - *MAL_USE_BOOST_CSTDINT*: If your compiler doesn't have <cstdint> use boost.
 - *MAL_USE_BOOST_ATOMIC*
 - *MAL_USE_BOOST_CHRONO*
 - *MAL_USE_BOOST_THREAD*
 - *MAL_NO_VARIABLE_INTEGER_WIDTH*: Integers are encoded ignoring the number
   trailing bytes set to zero, not based on its data type size. So when this
   isn't defined e.g. encoding an uint64 with a value up to 255 takes one byte
   (plus 1 byte header). Otherwise all uint64 values will take 8 bytes
   (plus header), so encoding is less space efficient in this way but it frees
   the CPU and allows the compiler to inline more.

## Linux compilation ##

You can compile the files in the "src" folder and make a library or just compile
everything under /src in your project.

Otherwise you can use the GNU makefile in the "/build/linux" folder. It respects
the GNU makefile conventions, so compiling and installing can be done as usual.

One example of compile invocation using boost could be:

    make CXXFLAGS="-DMAL_USE_BOOST_THREAD -DMAL_USE_BOOST_CHRONO -DBOOST_ALL_DYN_LINK -DBOOST_CHRONO_HEADER_ONLY" LDLIBS="-lboost_thread" CXX="arm-linux-gnueabihf-g++"

REMEMBER: That the if the library is compiled with e.g. "MAL_USE_BOOST_THREAD"
and "MAL_USE_BOOST_CHRONO" the client code must define them too.

For the install target DESTDIR, prefix, includedir and libdir can be used.

## Windows compilation ##

There is a Visual Studio 2010 Solution the "/build/windows" folder, but you need
to do a step before opening.

If you don't need the Boost libraries you should run the
"build\windows\mal-log\props\from_empty.bat" script. If you need them you should
run the "build\windows\mal-log\props\from_non_empty.bat" script.

If you don't need the Boost libraries you can open and compile the solution,
otherwise you need to edit (with a text editor) the newly generated file
""build\windows\mal-log\props\mal_dependencies.props" before and to update the
paths in the file. You can do this through the Visual Studio Property Manager
too.

> Written with [StackEdit](https://stackedit.io/).
