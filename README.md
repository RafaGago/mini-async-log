
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
   what the code is doing, easy to modify (2017 self-comment: not that easy :)).
 - Very low latency. Fast for the caller. Lock-free.
 - Asynchronous (synchronous calls can be made on top for special messages, but
   they are way slower than using a synchronous logger in the first place).
 - Minimum string formatting in the calling thread for the most common use
   cases.
 - Keeps ordering between threads.
 - Doesn't use thread-local-storage, the user threads are assumed as external
   and no extra info is attached to them.
 - Have termination functions to allow blocking until all the logs have been
   written to disk in program exit (or signal/fault) scenarios.

## Various features ##

 - Targeting g++4.7 and VS 2010
 - Boost dependencies just for parts that will eventually go to the C++
   standard.
 - No singleton by design, usable from dynamically loaded libraries. The user
   provides the instance either explicitly or by a global function (Koenig
   lookup).
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

To build it on Linux you don't need to install any of the libraries, all of them
are downloaded and build for you by the makefile, just run:

> make -f build/linux/Makefile.examples.benchmark

You can "make install" or search for the executable under
"build/linux/build/stage"

### Masuring an asynchronous loggers worst caselatency in a non-RTOS

TL; DR: Pointless: You don't use a non RTOS for meeting hard-deadlines.

IMO caring about worst case latency in a non RTOS is pointless, why? because
any thread can be suspended-preempted by the OS scheduler when it wishes for the
number of time quantas that it wants. In a non-RTOS the main source of
latency/jitter is the OS scheduler itself. This is the real worst case unless
the code is a _real_ piece of garbage.

Then there are other dubious things about the clock source itself:

Many OSes have diferent types of clocks, wall, CPU, thread...

Thread clocks and some CPU clocks stop counting when a thread get suspended so
"diff" in this code...

> auto start = threadclock_time();
> mutex_lock (&mutex);
> diff = threadclock_time() - start;

...will have a ceiling value of the time that it takes to enter the Kernel and
saving the context (roughly explained). It doesn't matter if the thread was
suspended. These clocks are the best used to measure single-threaded algorithm
performance, but not optimal to measure multithreaded code in a high-contention
scenario.

Then the clocks themselves are not well suited to the task. I have confirmed
myself that mini-async-log is fastest to enqueue than to get the timestamp
for the most simple (and common) log entries. Some OS clocks are slow, others
are too coarse-grained to measure in the nanosecond range, etc.

### Test metodology/configuration

The benchmark is a very though but unrealistic load.

It consists in enqueueing 1 million messages as fast as possible with different
thread counts. There are throughput and latency tests. This is run 75 times and
then averaged (except for maximum and minimum latencies). The test takes 7:35 on
my machine. The threads are tried to be evenly distributed between the cores
from the start.

The latency tests are measured with both a CPU clock and a wall clock. The wall
clock gives an idea of the real behavior of the logger when the OS scheduler
puts threads to sleep. In this way loggers using OS mutexes will show its worst
case in a more realistic way.

In the latency tests the mean, standard deviation, best and worse case of those
1 million log entries are presented. The standard deviation gives an idea of
the jitter. The less the better.

It is run in a system with Ubuntu 16.04 server, no X server and disabled network
interfaces. The machine is a downclocked and undervolted AMD Phenom x4 965 with
an SSD disk.

The test takes many hours to complete.

Each logger configuration summary is as follows:

- spdlog (async*1, bounded): (8MB / 32) queue entries.

- spdlog (sync, bounded?): spdlog in synchronous mode.

- glog (half-sync, bounded?): Google log stock cfg.

- g3log (async*1, heap?): g3log stock cfg.

- nanolog: (asynchronous, heap). Using the guaranteed version. The
    non-guaranteed is IMO unusable if the user can't know at the calling
    point if the call succeeded or not. There is no way to establish a
    comparison.

- mal (async, heap): mal using only the heap (unbounded). (8MB queue, 32 bytes
    each element).

- mal (async, heap + bounded): mal using a hybrid bounded queue + heap strategy
    (8MB/16 queue, 32 bytes each element).

- mal (sync*1, bounded): mal using a bounded queue and blocking on full queue
      (synchronous mode just added for fun on this test). (1MB queue, 32 bytes
      each element).

- mal (async, bounded): mal with a bounded queue (8MB queue, 32 bytes each
      element). will reject _a lot_ of messages. This is _very_ different of
      nanolog non-guaranteed because the messages not sent are known to fail
      at the caller point. Faults are substracted from the results and show as a
      penalties on the benchmark.

*1: All these loggers/configurations are asynchronous but they become blocking
when the queue is full. This isn't bad per se, as the size should be dimensioned
for the load to handle. This benchmark is a stress test that overflows the
queues.

### Results

These are done against the master branch of each project on March 2017.

### threads: 1 ###
#### Throughput (threads=1) ####

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.126|7945.230|1.349|741.506|0.126|0.000|
|mal hybrid|0.197|5070.545|1.438|695.412|0.197|0.000|
|spdlog async|1.557|642.433|0.000|0.000|1.557|0.000|
|g3log|8.825|113.337|0.000|0.000|8.825|0.000|
|nanolog|0.508|1975.716|0.000|0.000|0.508|0.000|
|glog|5.174|193.283|5.174|193.282|5.174|0.000|
|mal sync|2.061|491.328|2.414|418.300|2.061|0.000|
|spdlog sync|1.412|709.052|0.000|0.000|1.412|0.000|
|mal bounded|0.084|3495.853|0.420|698.437|0.084|706516.347|

#### Latency with thread clock (threads=1) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.270|0.078|0.000|77.447|
|mal hybrid|0.268|0.084|0.000|75.310|
|spdlog async|0.850|0.199|0.000|85.209|
|g3log|3.503|0.537|0.000|641.111|
|nanolog|0.403|6.766|0.000|22161.255|
|glog|2.882|4.671|0.000|15048.301|
|mal sync|0.804|154.365|0.000|978844.613|
|spdlog sync|1.567|2.430|0.000|1365.975|
|mal bounded|0.222|0.065|0.000|77.728|

#### Latency with wall clock (threads=1) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.347|7.243|0.000|20016.000|
|mal hybrid|0.356|7.305|0.000|12020.750|
|spdlog async|1.535|14.460|0.000|12030.750|
|g3log|9.094|26.285|0.000|24116.250|
|nanolog|0.585|59.257|0.000|26397.250|
|glog|3.743|146.206|0.000|887690.500|
|mal sync|1.837|65.380|0.000|551069.750|
|spdlog sync|1.464|2.482|0.000|1390.500|
|mal bounded|0.255|3.668|0.000|213.250|

### threads: 2 ###

#### Throughput (threads=2) ####

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.157|6390.438|1.420|704.097|0.235|0.000|
|mal hybrid|0.149|6704.804|1.398|715.368|0.243|0.000|
|spdlog async|1.378|726.001|0.000|0.000|2.109|0.000|
|g3log|3.482|287.276|0.000|0.000|5.505|0.000|
|nanolog|0.398|2523.710|0.000|0.000|0.592|0.000|
|glog|4.100|244.018|4.100|244.009|6.376|0.000|
|mal sync|1.681|610.757|2.024|503.646|3.006|0.000|
|spdlog sync|1.779|562.598|0.000|0.000|3.500|0.000|
|mal bounded|0.051|5589.147|0.378|743.404|0.085|719122.107|

#### Latency with thread clock (threads=2) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.382|0.258|0.000|590.523|
|mal hybrid|0.380|0.145|0.000|75.543|
|spdlog async|1.155|2.065|0.000|497.021|
|g3log|3.107|14.504|0.000|35621.502|
|nanolog|0.542|13.919|0.000|33780.853|
|glog|7.155|10.945|0.000|19209.166|
|mal sync|1.039|124.486|0.000|836462.805|
|spdlog sync|3.086|3.414|0.000|1687.710|
|mal bounded|0.259|0.117|0.000|74.806|

#### Latency with wall clock (threads=2) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.396|39.426|0.000|24018.250|
|mal hybrid|0.395|38.711|0.000|16027.500|
|spdlog async|1.933|386.506|0.000|402869.750|
|g3log|5.869|194.717|0.000|96512.750|
|nanolog|0.620|52.444|0.000|27627.750|
|glog|6.322|161.424|0.000|841916.750|
|mal sync|3.161|228.813|0.000|818299.750|
|spdlog sync|3.260|4.707|0.000|1770.250|
|mal bounded|0.228|29.513|0.000|16084.000|

### threads: 4 ###

#### Throughput (threads=4) ####

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.127|7913.696|1.395|716.917|0.439|0.000|
|mal hybrid|0.122|8235.414|1.429|699.693|0.414|0.000|
|spdlog async|1.259|795.823|0.000|0.000|3.694|0.000|
|g3log|2.118|472.352|0.000|0.000|6.041|0.000|
|nanolog|0.405|2512.369|0.000|0.000|1.381|0.000|
|glog|4.123|242.560|4.123|242.550|15.559|0.000|
|mal sync|1.524|667.417|1.880|538.434|5.344|0.000|
|spdlog sync|1.743|575.084|0.000|0.000|6.862|0.000|
|mal bounded|0.063|4557.211|0.398|718.455|0.220|713963.427|

#### Latency with thread clock (threads=4) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.481|0.296|0.000|492.147|
|mal hybrid|0.485|0.200|0.000|102.109|
|spdlog async|2.228|1.970|0.000|408.481|
|g3log|3.764|45.104|0.000|56006.180|
|nanolog|0.880|29.042|0.000|24509.118|
|glog|9.891|13.717|0.000|13993.848|
|mal sync|1.632|48.004|0.000|8463.396|
|spdlog sync|5.664|5.018|0.000|2005.818|
|mal bounded|0.314|0.205|0.000|72.929|

#### Latency with wall clock (threads=4) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.488|31.253|0.000|20019.500|
|mal hybrid|0.489|30.215|0.000|16031.000|
|spdlog async|3.610|717.037|0.000|817889.000|
|g3log|6.167|165.291|0.000|49672.250|
|nanolog|0.929|55.725|0.000|39097.500|
|glog|14.632|91.052|0.000|24396.250|
|mal sync|5.996|434.340|0.000|850038.250|
|spdlog sync|6.549|9.813|0.000|2205.000|
|mal bounded|0.306|24.554|0.000|16035.500|

### threads: 8 ###

#### Throughput (threads=8) ####

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.115|8725.011|1.449|690.239|0.733|0.000|
|mal hybrid|0.114|8813.223|1.451|689.169|0.751|0.000|
|spdlog async|1.188|842.420|0.000|0.000|6.508|0.000|
|g3log|1.462|684.855|0.000|0.000|8.763|0.000|
|nanolog|0.548|1872.165|0.000|0.000|3.701|0.000|
|glog|4.759|210.207|4.760|210.194|36.373|0.000|
|mal sync|1.497|678.098|1.843|548.220|10.048|0.000|
|spdlog sync|1.745|574.548|0.000|0.000|11.835|0.000|
|mal bounded|0.067|4230.673|0.402|696.510|0.434|719967.040|

#### Latency with thread clock (threads=8) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.480|0.206|0.000|467.871|
|mal hybrid|0.489|0.238|0.000|482.395|
|spdlog async|1.959|1.876|0.000|1569.762|
|g3log|3.714|33.133|0.000|51278.157|
|nanolog|1.315|94.957|0.000|113730.770|
|glog|6.110|19.168|0.000|15323.714|
|mal sync|1.777|82.498|0.000|423814.612|
|spdlog sync|3.335|4.561|0.000|5286.152|
|mal bounded|0.309|0.211|0.000|84.510|

#### Latency with wall clock (threads=8) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.876|66.039|0.000|24010.500|
|mal hybrid|0.882|66.726|0.000|24019.750|
|spdlog async|6.519|1164.560|0.000|1000879.750|
|g3log|9.784|197.660|0.000|96865.500|
|nanolog|2.492|230.070|0.000|240000.750|
|glog|35.093|1189.785|0.000|1061686.000|
|mal sync|11.753|585.439|0.000|602712.500|
|spdlog sync|12.417|250.868|0.000|44341.750|
|mal bounded|0.559|51.964|0.000|24003.750|

### threads: 16 ###

#### Throughput (threads=16) ####

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.109|9221.551|1.429|699.848|1.221|0.000|
|mal hybrid|0.103|9722.275|1.447|691.009|1.274|0.000|
|spdlog async|1.184|844.823|0.000|0.000|12.449|0.000|
|g3log|1.203|833.128|0.000|0.000|15.845|0.000|
|nanolog|0.940|1080.626|0.000|0.000|13.339|0.000|
|glog|4.822|207.438|4.823|207.399|71.735|0.000|
|mal sync|1.540|651.859|1.881|532.950|20.764|0.000|
|spdlog sync|1.727|579.247|0.000|0.000|22.563|0.000|
|mal bounded|0.069|4033.660|0.404|682.638|0.806|724294.933|

#### Latency with thread clock (threads=16) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.482|0.199|0.000|80.373|
|mal hybrid|0.492|0.234|0.000|485.819|
|spdlog async|1.836|1.945|0.000|2637.872|
|g3log|3.809|36.726|0.000|30658.330|
|nanolog|2.049|131.082|0.000|71946.083|
|glog|5.759|19.261|0.000|16127.556|
|mal sync|1.612|67.974|0.000|18048.726|
|spdlog sync|2.890|4.411|0.000|5324.756|
|mal bounded|0.304|0.214|0.000|81.663|

#### Latency with wall clock (threads=16) ####

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|1.596|126.657|0.000|44007.000|
|mal hybrid|1.596|126.213|0.000|44007.250|
|spdlog async|12.070|1823.004|0.000|1020022.750|
|g3log|17.553|260.136|0.000|88671.250|
|nanolog|7.617|641.032|0.000|321992.250|
|glog|71.834|2094.555|0.000|1404892.000|
|mal sync|23.215|1072.886|0.000|490287.750|
|spdlog sync|24.159|436.705|0.000|63665.500|
|mal bounded|1.004|100.148|0.000|44005.500|

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

As for now, every function returns a boolean if it succeeded or false if it
didn't. A filtered out/below severity call returns true.

The only possible failures are either to be unable to allocate memory for a log
entry, an asynchronous call that was interrupted by "on_termination" or a string
byte that was about to be deep copied but would overflow the length variable,
which would mostly be a bug, I highly doubt that someone will log messages that
take 4GB).

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
