
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
thread counts. There are throughput and latency tests. This is run 50 times and
then averaged (except for maximum and minimum latencies).

The latency tests are measured with both a CPU clock and a wall clock. The wall
clock gives an idea of the real behavior of the logger when the OS scheduler
puts threads to sleep. In this way loggers using OS mutexes worst case.

In the latency tests the mean, standard deviation, best and worse case of those
2 million log entries are presented. The standard deviation gives an idea of
the jitter. The less the better.

It is run in a system with Ubuntu 16.04 server, no X server and disabled network
interfaces. The machine is a downclocked and undervolted AMD Phenom x4 965 with
an SSD disk.

The test takes around 4h 15m.

Each logger configuration summary is as follows:

- spdlog async: spdlog in asynchronous mode. (8MB / 32) queue entries.

- spdlog sync: spdlog in synchronous mode.

- glog: Google log (half synchronous). Stock cfg.

- g3log: g3log (asynchronous). Stock cfg.

- nanolog: (asynchronous). 8MB hardcoded internal queue size. Using the
          guaranteed version. What is non-guaranteed logging good for if the
          user can't know at the calling point if the call succeeded or not?

- mal heap: mal using only the heap (unbounded). (8MB queue, 32 bytes each
          element).

- mal hybrid: mal using a hybrid bounded queue + heap strategy (8MB/16 queue, 32
           bytes each element).

- mal sync: mal using a bounded queue and blocking on full queue (synchronous
           mode just added for fun on this test). (1MB queue, 32 bytes each
           element).

- mal bounded: mal with a bounded queue (8MB queue, 32 bytes each element). will
               reject _a lot_ of messages. This is _very_ different of nanolog
               non-guaranteed because the messages not sent are known to fail
               at the caller point. Faults are substracted from the results and
               show as a penalties on the benchmark.

### Results

These are done against the master branch of each project on March 2017.

###threads: 1

#### Throughput (threads=1)

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.130|7711.309|1.221|818.898|0.130|0.000|
|mal hybrid|0.118|8455.209|1.228|814.135|0.118|0.000|
|spdlog async|1.067|937.981|0.000|0.000|1.067|0.000|
|g3log|4.614|216.789|0.000|0.000|4.614|0.000|
|nanolog|0.260|3846.715|0.000|0.000|0.260|0.000|
|glog|2.877|347.588|2.877|347.587|2.877|0.000|
|mal sync|0.950|1074.696|1.288|793.948|0.950|0.000|
|spdlog sync|1.368|731.470|0.000|0.000|1.368|0.000|
|mal bounded|0.046|6318.494|0.342|857.564|0.046|706642.020|

#### Latency with thread clock (threads=1)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.361|0.147|0.000|71.419|
|mal hybrid|0.356|0.144|0.000|64.668|
|spdlog async|1.400|0.674|0.000|80.159|
|g3log|4.879|1.792|0.000|77.168|
|nanolog|0.413|2.528|0.000|6803.896|
|glog|3.015|4.577|0.000|4015.253|
|mal sync|0.712|0.972|0.000|85.270|
|spdlog sync|1.564|2.396|0.000|1407.093|
|mal bounded|0.245|0.093|0.000|65.625|

#### Latency with wall clock (threads=1)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.209|0.179|0.000|67.500|
|mal hybrid|0.210|0.188|0.000|73.250|
|spdlog async|1.149|3.637|0.000|12022.750|
|g3log|4.661|2.070|0.000|4037.500|
|nanolog|0.309|2.512|0.000|878.250|
|glog|2.915|84.996|0.000|596567.250|
|mal sync|0.853|79.810|0.000|409559.000|
|spdlog sync|1.415|2.457|0.000|2413.750|
|mal bounded|0.117|0.144|0.000|66.000|

###threads: 2

#### Throughput (threads=2)

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.170|5880.785|1.242|805.250|0.334|0.000|
|mal hybrid|0.179|5589.200|1.258|795.316|0.355|0.000|
|spdlog async|1.313|763.219|0.000|0.000|2.555|0.000|
|g3log|1.958|510.790|0.000|0.000|3.793|0.000|
|nanolog|0.297|3377.687|0.000|0.000|0.591|0.000|
|glog|4.355|229.915|4.355|229.910|8.701|0.000|
|mal sync|1.006|1047.717|1.335|779.220|1.948|0.000|
|spdlog sync|1.697|590.626|0.000|0.000|3.383|0.000|
|mal bounded|0.066|4710.990|0.362|859.156|0.130|689311.280|

#### Latency with thread clock (threads=2)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.542|0.218|0.000|479.759|
|mal hybrid|0.536|0.144|0.000|72.105|
|spdlog async|1.991|1.232|0.000|511.705|
|g3log|4.049|54.288|0.000|82542.491|
|nanolog|0.692|4.297|0.000|1071.912|
|glog|6.935|7.851|0.000|3270.576|
|mal sync|1.310|2.044|0.000|197.826|
|spdlog sync|3.084|3.479|0.000|4387.442|
|mal bounded|0.353|0.147|0.000|84.074|

#### Latency with wall clock (threads=2)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.434|0.697|0.000|4100.500|
|mal hybrid|0.430|1.777|0.000|12015.750|
|spdlog async|2.388|46.579|0.000|120648.500|
|g3log|3.874|3.494|0.000|3647.500|
|nanolog|0.589|4.977|0.000|12022.750|
|glog|8.758|180.237|0.000|570639.000|
|mal sync|1.658|38.104|0.000|188700.500|
|spdlog sync|3.288|4.897|0.000|4043.250|
|mal bounded|0.205|2.413|0.000|12024.000|

###threads: 4

#### Throughput (threads=4)

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.140|7160.330|1.272|786.811|0.444|0.000|
|mal hybrid|0.139|7251.798|1.269|788.199|0.475|0.000|
|spdlog async|1.127|889.108|0.000|0.000|3.658|0.000|
|g3log|1.437|696.162|0.000|0.000|5.448|0.000|
|nanolog|0.380|2681.219|0.000|0.000|1.403|0.000|
|glog|4.166|240.122|4.166|240.114|16.557|0.000|
|mal sync|0.946|1076.649|1.271|799.819|3.448|0.000|
|spdlog sync|1.715|584.309|0.000|0.000|6.846|0.000|
|mal bounded|0.080|3924.831|0.374|835.601|0.294|687639.620|

#### Latency with thread clock (threads=4)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.513|0.415|0.000|511.855|
|mal hybrid|0.512|0.171|0.000|111.561|
|spdlog async|2.462|2.440|0.000|2628.036|
|g3log|3.689|53.359|0.000|62680.798|
|nanolog|0.891|32.689|0.000|49694.812|
|glog|10.238|10.278|0.000|5594.948|
|mal sync|1.871|2.591|0.000|279.361|
|spdlog sync|5.407|4.937|0.000|1781.881|
|mal bounded|0.341|0.175|0.000|73.406|

#### Latency with wall clock (threads=4)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.543|29.021|0.000|24005.750|
|mal hybrid|0.530|29.526|0.000|24016.250|
|spdlog async|3.678|742.847|0.000|808391.250|
|g3log|5.542|90.621|0.000|101110.750|
|nanolog|0.991|71.331|0.000|102989.000|
|glog|16.232|284.838|0.000|816382.000|
|mal sync|3.223|225.919|0.000|794032.500|
|spdlog sync|6.672|61.045|0.000|210836.500|
|mal bounded|0.292|22.203|0.000|20007.750|

###threads: 8

#### Throughput (threads=8)

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.132|7576.450|1.243|804.729|0.683|0.000|
|mal hybrid|0.127|7895.176|1.271|787.196|0.705|0.000|
|spdlog async|1.069|937.702|0.000|0.000|6.262|0.000|
|g3log|1.190|843.128|0.000|0.000|9.008|0.000|
|nanolog|0.581|1752.534|0.000|0.000|4.218|0.000|
|glog|3.830|261.100|3.830|261.088|30.337|0.000|
|mal sync|1.020|1024.347|1.328|774.919|6.765|0.000|
|spdlog sync|1.735|577.809|0.000|0.000|13.797|0.000|
|mal bounded|0.081|3836.921|0.376|821.530|0.494|690855.880|

#### Latency with thread clock (threads=8)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.496|0.431|0.000|504.111|
|mal hybrid|0.499|0.170|0.000|97.510|
|spdlog async|2.243|2.021|0.000|874.355|
|g3log|3.606|38.342|0.000|54364.680|
|nanolog|1.294|83.357|0.000|105954.363|
|glog|11.813|7.999|0.000|1254.618|
|mal sync|2.093|2.689|0.000|305.088|
|spdlog sync|5.632|5.035|0.000|1744.719|
|mal bounded|0.339|0.168|0.000|87.249|

#### Latency with wall clock (threads=8)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.838|72.221|0.000|32013.500|
|mal hybrid|0.803|68.674|0.000|40010.750|
|spdlog async|6.084|1199.106|0.000|1005115.750|
|g3log|9.970|145.274|0.000|73050.250|
|nanolog|2.698|275.494|0.000|256433.250|
|glog|32.657|51.267|0.000|12302.250|
|mal sync|5.993|370.835|0.000|771183.250|
|spdlog sync|13.784|230.688|0.000|407219.500|
|mal bounded|0.486|57.932|0.000|36007.750|

###threads: 16

#### Throughput (threads=16)

|logger|enqueue(s)|rate(Kmsg/s)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.125|8017.746|1.237|808.630|1.232|0.000|
|mal hybrid|0.121|8275.195|1.264|791.818|1.205|0.000|
|spdlog async|1.093|918.023|0.000|0.000|11.364|0.000|
|g3log|1.043|965.017|0.000|0.000|15.715|0.000|
|nanolog|0.923|1096.771|0.000|0.000|13.436|0.000|
|glog|3.806|262.718|3.807|262.706|59.994|0.000|
|mal sync|1.071|989.880|1.381|752.963|13.724|0.000|
|spdlog sync|1.532|658.223|0.000|0.000|24.354|0.000|
|mal bounded|0.078|3927.956|0.373|819.880|0.741|694552.040|

#### Latency with thread clock (threads=16)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|0.489|1.293|0.000|1247.466|
|mal hybrid|0.491|0.593|0.000|822.522|
|spdlog async|2.258|2.179|0.000|1454.300|
|g3log|3.736|33.966|0.000|32536.103|
|nanolog|2.083|140.071|0.000|64961.240|
|glog|11.940|9.132|0.000|17724.958|
|mal sync|2.183|3.221|0.000|4067.417|
|spdlog sync|5.397|4.933|0.000|1585.444|
|mal bounded|0.335|0.170|0.000|92.873|

#### Latency with wall clock (threads=16)

|logger|mean(us)|standard deviation|min(us)|max(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal heap|1.484|139.507|0.000|86382.250|
|mal hybrid|1.481|140.247|0.000|64014.250|
|spdlog async|10.497|1778.972|0.000|1014318.750|
|g3log|17.291|217.347|0.000|75330.750|
|nanolog|8.193|741.447|0.000|403990.750|
|glog|66.517|93.593|0.000|31383.750|
|mal sync|11.578|154.184|0.000|255123.750|
|spdlog sync|24.803|512.088|0.000|588557.750|
|mal bounded|0.766|106.593|0.000|60007.250|

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
