
Minimal Asynchronous Logger (MAL)
-----------
A performant asynchronous data logger with acceptable feature-bloat.

## Credit ##

 - To my former employer **Diadrom AB.** for allowing me to share this code with
   a BSD license. They funded most of the development of this project.
 - To Dmitry Vjukov for all the knowledge he has spread on the internets,
   including the algorithms for the two queues on this project.
 - To my girlfriend for coexisting with me when I become temporarily autistic
   after having been "in the zone" for too long.

## Motivation ##

This started with the intention to just develop an asynchronous logger that
could be used from different dynamically loaded libraries on the same binary
without doing link-time hacks like being forced to link static, hiding symbols
and some other "niceties".

Then at some point way after the requirements were met I just improved it for
fun.

## Design rationale ##

To be:

 - Simple. Not over abstracted and feature bloated, explicit, easy to figure out
   what the code is doing, easy to modify (2017 self-comment: not that easy
   after having switched to raw C coding for a while :D).
 - Very low latency. Fast for the caller.
 - Asynchronous (synchronous calls can be made on top for special messages, but
   they are way slower than using a synchronous logger in the first place).
 - Have minimum string formatting on the calling thread for the most common use
   cases.
 - Don't use thread-local-storage, the user threads are assumed as external
   and no extra info is attached to them.
 - Have termination functions to allow blocking until all the logs have been
   written to disk in program exit (or signal/fault) scenarios.

## Various features ##

 - Targeting g++4.7 and VS 2010(can use incomplete or broken C+11 features from
   boost).
 - Nanosecond performance.
 - No singleton by design, usable from dynamically loaded libraries. The user
   provides the logger instance either explicitly or by a global function
   (Koenig  lookup).
 - Suitable for soft-realtime work. Once it's initialized the fast-path can be
   clear from heap allocations (if properly configured to).
 - File rotation-slicing.
 - One conditional call overhead for inactive logging levels.
 - Able to strip log levels at compile time (for Release builds).
 - Lazy parameter evaluation (as usual with most logging libraries).
 - No ostreams(*) (a very ugly part of C++: dynamically allocated, verbose and
   stateful), just format strings checked at compile time (if the compiler
   supports it) with type safe values.
 - The logger severity threshold can be externally changed outside of the
   process. The IPC mechanism is the simplest, the log worker periodically polls
   some files when idling (if configured to).
 - Fair blocking behaviour (configurable by severity) when the bounded queue is
   full and the heap queue is disabled. The logger smoothly starts to act as a
   synchronous logger. If non-blocking behavior is desired an error is returned
   instead.
 - Small, you can actually compile it as a part of your application.

(*)An on-stack ostream adapter is available as a last resort, but its use is
 more verbose and has more overhead than the format literals.

> see this [example](https://github.com/RafaGago/mini-async-log/blob/master/example/overview/main.cpp)
that more or less showcases all available features.

## How does it work ##

It just borrows ideas from many of the loggers out there.

As an asynchronous logger its main objetive is to be as fast and to have as low
latency as possible for the caller thread.

When the user is to write a log message, the producer task is to serialize the
data to a memory chunk which then the logging backend (consumer) can decode,
format and write. No expensive operations occur on the consumer, and if they do
it's when using secondary features.

The format string is required to be a literal (compile time constant), so when
encoding something like the entry below...

> log_error ("File:a.cpp line:8 This is a string that just displays the next number {}, int32_val);

...the memory requirements are just a pointer to the format string and a deep
copy of the integer value. The job of the caller is just to serialize some bytes
to a memory chunk and to insert the chunk into a queue.

The queue is a mix of two famous lockfree queues of Dmitry Vyukov (kudos to this
genious) for this particular MPSC case. The queue is a blend of a fixed capacity
and fixed element size array based preallocated queue and an intrusive node
based dynamically allocated queue. The resulting queue is still linearizable.

The format string is type-safe and validated at compile time for compilers that
support "constexpr" and "variadic template parameters". Otherwise the errors
are caught at run time on the logged output (Visual Studio 2010 mostly).

There are other features: you can block the caller thread until some message has
been dequeued by the logger thread, to do C++ stream formatting on the caller
thread, etc.

## File rotation ##

The library can rotate log files.

Using the current C++11 standard files can just be created, modified and
deleted. There is no way to list a directory, so the user is required to pass
the list of files generated by previous runs.at startup time.

> There is an [example](https://github.com/RafaGago/mini-async-log/blob/master/example/rotation/main.cpp)
here.

## Initialization ##

The library isn't a singleton, so the user should provide a reference to the
logger instance on each call.

There are two methods to pass the instance to the logging macros, one is to
provide it explicitly and the other one is by providing it on a global function.

If no instance is provided, the global function "get_mal_logger_instance()" will
be called without being namespace qualified, so you can use Koenig lookup/ADL.
This happens when the user calls the macros with no explicit instance suffix, as
e.g. "log_error(fmt string, ...)".

To provide the instance explictly the macros with the "_i" suffix need to be
called, e.g. "log_error_i(instance, fmt_string, ...)"

The name of the function can be changed at compile time, by defining
MAL_GET_LOGGER_INSTANCE_FUNCNAME.

## Termination ##

The worker blocks on its destructor until its work queue is empty when normally
exiting a program.

When a signal is caught you can call the frontend function [on termination](https://github.com/RafaGago/mini-async-log/blob/master/include/mal_log/frontend.hpp) in
your signal handler. This will flush the logger queue and early abort any
synchronous calls.

## Errors ##

As of now, every log call returns a boolean to indicate success.

The only possible failures are either to be unable to allocate memory for a log
entry or an asynchronous call that was interrupted by "on_termination". A
filtered out call returns true".

The logging functions never throw.

## Compiler macros ##

Those that are self-explanatory won't be explained.

 - *MAL_GET_LOGGER_INSTANCE_FUNC*: See the "Initialization" chapter above.
 - *MAL_STRIP_LOG_SEVERITY*: Removes the entries of this severity and below at
    compile time, so they are not evaluated and don't take code space. 0 is the
    "debug" severity, 5 is the "critical" severity.
    Stripping at level 5 leaves no log entries at all. Yo can define e.g.
    MAL_STRIP_LOG_DEBUG, MAL_STRIP_LOG_TRACE, etc. instead. If you define
    MAL_STRIP_LOG_TRACE all the severities below will be automatically defined
    for you (in this case MAL_STRIP_LOG_DEBUG).
 - *MAL_DYNLIB_COMPILE*: Define it when __compiling__ MAL as a dynamic
    library/shared object.
 - *MAL_DYNLIB*: Define it when __using__ MAL as a dynamic library. Don't define
    it if you are static linking or compiling the library with your project.
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

## compilation ##

You can compile the files in the "src" folder and make a library or just compile
everything under /src in your project.

Otherwise you can use cmake.

On Linux there are GNU makefiles in the "/build/linux" folder too. They respect
the GNU makefile conventions. "DESTDIR", "prefix", "includedir" and "libdir"
can be used.

REMEMBER (legacy uses that use boost): That the if the library is compiled with
e.g. the "MAL_USE_BOOST_THREAD" and "MAL_USE_BOOST_CHRONO" preprocessor
variables the client code must define them too.

## Windows compilation ##

There is a Visual Studio 2010 Solution in the "/build/windows" folder, but you
need to do a step before opening:

If you don't need the Boost libraries you should run the
"build\windows\mal-log\props\from_empty.bat" script. If you need them you should
run the "build\windows\mal-log\props\from_non_empty.bat" script.

If you don't need the Boost libraries you can open and compile the solution,
otherwise you need to edit (with a text editor) the newly generated file
""build\windows\mal-log\props\mal_dependencies.props" before and to update the
paths in the file. You can do this through the Visual Studio Property Manager
too.

## Performace ##

These are some tests I have done for fun to see how this code is aging.

> [Here is the benchmark code.](https://github.com/RafaGago/mini-async-log/blob/master/example/benchmark/main.cpp).

To build them on Linux you don't need to install any of the libraries, all of
them are downloaded and build for you by the makefile, just run:

> make -f build/linux/Makefile.examples.benchmark

You can "make install" or search for the executable under
"build/linux/build/stage"

### Test methodology ###

It consists on enqueueing either 1 million or 100K messages distributing them
evenly accross a variying number of threads (1, 2, 4, 8, 16) for each test. Each
test is run 75 times and then averaged (best and worst latencies aren't
averaged, the best and worst of all runs is taken).

The different message counts (1M and 100K) are intended to show the behavior
of the bounded queue loggers. On the 100K msgs tests the bounded queue loggers
(spdlog and some mal variants) have a big enough queue to contain all the
messages. On the 1M msgs test its queue will get full and they will need to
back-off.

On the latency tests the mean, standard deviation, best and worst case is shown.
The standard deviation gives an idea of the jitter. The less the better.

The latency tests are measured with both a thread clock and a wall clock. The
thread clock shows only cycles spent on the thread, not when the thread is
suspended. The wall clock gives an idea of the "real" timing of the logger when
the OS scheduler puts threads to sleep, so the different loggers will show its
worst case in a more realistic way.

Keep in mind that measuring latencies has its caveats. The OS clocks aren't
good enough for measuring individual calls at the nanosecond scale. I do these
latency tests because I can see value in seeing latency standard deviations for
very long runs and the worst latencies allow to detect blocked/starved producers
(e.g.due to backoff unfairness).

The "average latency" column shown on the latency tests is the one measured on
individual calls (mostly unreliable). The "average latency" column shown on the
throughput tests is taken just using two clock meaurements for the whole batch
of messages (reliable).

The test is run in a system using Ubuntu 16.04 server, no X server and having
all the network interfaces disabled. The machine is a downclocked and
undervolted AMD Phenom x4 965 with an SSD disk. Expect more performance with a
modern machine.

The code is compiled with gcc -O3. Compiling with -Os has a lot of impact (re-
duction) in raw performance.

The test takes some hours to run.

The tests are done against the master branch of each project on March 2017.

### Library configuration ###

Each logger configuration summary is as follows:

|logger|sync/async|queue type|unbounded queue entries|
|:-:|:-:|:-:|:-:|
|mal-heap|async|dynamic|-|
|mal-hybrid|async|dynamic + fixed|32768 ((8Mb/32) / 8)|
|spdlog-async|async (blocks on full queue)|fixed|262144 (8Mb/32)|
|g3log|async (blocks on full queue)|fixed (?)|0.000|-|
|nanolog|async|dynamic|-|
|glog|half-async|fixed|(?)|
|mal-blocking|async (blocks on full queue)|fixed|262144 (8Mb/32)|
|spdlog-sync|sync|fixed (?)|(?)|
|mal-bounded|async|fixed|262144 (8Mb/32)|

### Result conclusions ###

I have observed some variability (5-10% ?) on the test results between test
runs, which shows that 75 averaged runs is not enough. I want to be able to run
the benchmark overnight, so I won't increase this number.

The variability between machines (AMD-Intel) is big too.

Said that, and being as biased as I am, I consider the mal variants the only
asynchronous loggers of the ones tested here that can be severely stressed and
still have a controlled behavior in all the tested parameters: the performance
is always respectable and the worst case latency never goes out of control.

## Result data ##

### threads: 1, msgs: 1M ###

#### Throughput (threads: 1, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.109|9216.825|0.108|1.139|877.644|0.109|0.000|
|mal-hybrid|0.147|6804.861|0.147|1.190|840.444|0.147|0.000|
|nanolog|0.351|2851.872|0.351|3.728|268.254|0.351|0.000|
|spdlog-async|1.071|933.572|1.071|1.450|691.153|1.071|0.000|
|mal-blocking|0.853|1172.615|0.853|1.134|881.477|0.853|0.000|
|mal-bounded|0.049|5757.068|0.174|0.325|859.745|0.049|720391.240|
|g3log|8.745|114.379|8.743|0.000|0.000|8.745|0.000|
|spdlog-sync|0.931|1073.579|0.931|0.932|1073.545|0.931|0.000|
|glog|2.537|394.237|2.537|2.537|394.232|2.537|0.000|

#### "Real" latency (threads: 1, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19318.500|0.267|39.921|
|mal-hybrid|15901.500|0.259|39.349|
|nanolog|24013.250|0.483|53.867|
|spdlog-async|32125.750|1.061|86.033|
|mal-blocking|15868.750|0.883|23.453|
|mal-bounded|18929.250|0.176|32.453|
|g3log|36125.000|9.018|178.542|
|spdlog-sync|1255.500|0.975|2.185|
|glog|6734.500|2.635|4.936|

#### CPU latency (threads: 1, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|77.840|0.241|0.045|
|mal-hybrid|74.429|0.230|0.055|
|nanolog|11009.518|0.349|4.640|
|spdlog-async|81.843|0.591|0.169|
|mal-blocking|76.194|0.217|0.216|
|mal-bounded|78.905|0.200|0.037|
|g3log|1761.118|3.590|1.383|
|spdlog-sync|1236.188|1.113|2.176|
|glog|1291.511|2.748|4.452|

### threads: 2, msgs: 1M ###

#### Throughput (threads: 2, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.128|7801.788|0.128|1.185|843.719|0.196|0.000|
|mal-hybrid|0.119|8467.794|0.118|1.182|846.224|0.191|0.000|
|nanolog|0.274|3656.557|0.273|3.705|269.938|0.464|0.000|
|spdlog-async|1.065|939.304|1.065|1.383|726.134|1.624|0.000|
|mal-blocking|0.827|1208.857|0.827|1.108|902.144|1.264|0.000|
|mal-bounded|0.039|7346.663|0.136|0.316|884.742|0.053|720277.187|
|g3log|3.549|281.830|3.548|0.000|0.000|5.383|0.000|
|spdlog-sync|1.324|755.433|1.324|1.325|755.380|2.609|0.000|
|glog|5.122|195.677|5.110|5.122|195.673|10.237|0.000|

#### "Real" latency (threads: 2, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19891.750|0.326|35.996|
|mal-hybrid|26413.750|0.330|36.097|
|nanolog|36015.750|0.564|48.461|
|spdlog-async|409461.250|1.490|386.980|
|mal-blocking|20813.000|1.552|28.030|
|mal-bounded|26525.000|0.172|27.278|
|g3log|102171.750|5.602|195.783|
|spdlog-sync|1598.000|2.568|4.215|
|glog|48001.000|8.152|11.827|

#### CPU latency (threads: 2, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|471.494|0.356|0.273|
|mal-hybrid|79.977|0.340|0.153|
|nanolog|16958.183|0.518|16.114|
|spdlog-async|2027.961|0.881|1.532|
|mal-blocking|76.879|0.333|0.503|
|mal-bounded|76.634|0.233|0.109|
|g3log|40738.198|2.938|16.366|
|spdlog-sync|1539.387|2.342|2.987|
|glog|613.643|6.275|5.870|

### threads: 4, msgs: 1M ###

#### Throughput (threads: 4, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.128|7861.920|0.127|1.199|833.823|0.428|0.000|
|mal-hybrid|0.109|9141.102|0.109|1.192|839.246|0.387|0.000|
|nanolog|0.292|3439.325|0.291|3.722|268.672|1.086|0.000|
|spdlog-async|1.002|1002.037|0.998|1.278|785.531|2.895|0.000|
|mal-blocking|0.826|1210.152|0.826|1.108|902.838|2.541|0.000|
|mal-bounded|0.042|6962.221|0.144|0.319|881.417|0.125|718960.373|
|g3log|2.067|483.802|2.067|0.000|0.000|5.613|0.000|
|spdlog-sync|1.642|609.837|1.640|1.642|609.792|6.484|0.000|
|glog|4.355|229.647|4.355|4.355|229.639|17.247|0.000|

#### "Real" latency (threads: 4, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19942.000|0.458|29.300|
|mal-hybrid|19914.750|0.460|29.692|
|nanolog|25280.250|1.024|55.861|
|spdlog-async|819813.750|2.962|745.080|
|mal-blocking|19909.000|2.806|32.933|
|mal-bounded|26155.000|0.255|22.625|
|g3log|48199.250|5.731|165.496|
|spdlog-sync|57704.000|5.753|44.167|
|glog|11939.250|15.843|16.677|

#### CPU latency (threads: 4, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|92.069|0.465|0.183|
|mal-hybrid|493.823|0.463|0.226|
|nanolog|36191.740|0.902|29.925|
|spdlog-async|498.251|2.045|1.913|
|mal-blocking|100.290|0.600|0.913|
|mal-bounded|72.658|0.300|0.201|
|g3log|54542.518|3.541|44.219|
|spdlog-sync|51530.026|4.625|12.504|
|glog|1880.552|10.097|7.252|

### threads: 8, msgs: 1M ###

#### Throughput (threads: 8, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.115|8739.771|0.114|1.210|826.403|0.666|0.000|
|mal-hybrid|0.106|9431.106|0.106|1.206|829.017|0.658|0.000|
|nanolog|0.393|2602.147|0.384|3.818|261.993|2.671|0.000|
|spdlog-async|0.932|1074.291|0.931|1.295|774.318|5.164|0.000|
|mal-blocking|0.847|1181.234|0.847|1.128|886.879|5.185|0.000|
|mal-bounded|0.056|5103.887|0.196|0.332|852.660|0.319|717013.547|
|g3log|1.389|720.190|1.389|0.000|0.000|7.904|0.000|
|spdlog-sync|1.347|742.697|1.346|1.348|742.234|9.888|0.000|
|glog|5.027|198.975|5.026|5.028|198.946|38.467|0.000|

#### "Real" latency (threads: 8, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|24013.500|0.798|71.716|
|mal-hybrid|24014.750|0.801|71.292|
|nanolog|243440.000|2.364|202.190|
|spdlog-async|823334.750|5.126|1016.887|
|mal-blocking|24171.750|5.705|58.239|
|mal-bounded|24006.000|0.486|53.890|
|g3log|60338.000|9.104|190.362|
|spdlog-sync|62936.750|10.502|257.781|
|glog|83065.500|40.376|642.256|

#### CPU latency (threads: 8, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|495.434|0.463|0.219|
|mal-hybrid|494.577|0.469|0.353|
|nanolog|83046.371|1.180|77.468|
|spdlog-async|512.649|1.595|1.636|
|mal-blocking|94.368|0.589|0.853|
|mal-bounded|83.857|0.295|0.210|
|g3log|47289.073|3.563|31.973|
|spdlog-sync|34478.808|2.718|8.802|
|glog|3920.326|6.840|11.054|

### threads: 16, msgs: 1M ###

#### Throughput (threads: 16, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.110|9167.650|0.109|1.210|826.296|1.202|0.000|
|mal-hybrid|0.096|10459.100|0.096|1.210|826.832|1.152|0.000|
|nanolog|0.598|1701.988|0.588|4.000|250.049|7.805|0.000|
|spdlog-async|0.925|1082.522|0.924|1.245|807.420|9.790|0.000|
|mal-blocking|0.866|1154.990|0.866|1.147|871.996|10.609|0.000|
|mal-bounded|0.066|4264.649|0.234|0.342|817.064|0.727|720542.280|
|g3log|1.123|891.511|1.122|0.000|0.000|14.420|0.000|
|spdlog-sync|1.356|737.996|1.355|1.358|736.730|19.452|0.000|
|glog|5.133|194.917|5.130|5.134|194.868|76.668|0.000|

#### "Real" latency (threads: 16, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|36009.250|1.484|138.879|
|mal-hybrid|40014.000|1.530|140.186|
|nanolog|274224.250|7.101|602.806|
|spdlog-async|1012947.000|9.640|1499.456|
|mal-blocking|36007.250|11.533|102.610|
|mal-bounded|36011.750|0.892|107.341|
|g3log|80436.000|16.401|256.023|
|spdlog-sync|128116.250|21.135|449.278|
|glog|191780.500|81.865|1299.472|

#### CPU latency (threads: 16, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|468.671|0.461|0.187|
|mal-hybrid|487.831|0.472|0.204|
|nanolog|69313.157|1.834|130.108|
|spdlog-async|1081.075|1.509|1.616|
|mal-blocking|103.980|0.667|0.956|
|mal-bounded|91.847|0.294|0.222|
|g3log|26775.972|3.601|34.357|
|spdlog-sync|19404.306|2.464|8.409|
|glog|2237.171|6.267|10.515|

### threads: 1, msgs: 100k ###

#### Throughput (threads: 1, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6661.134|0.150|0.125|800.620|0.016|0.000|
|mal-hybrid|0.006|15591.701|0.064|0.128|779.201|0.006|0.000|
|nanolog|0.016|6453.745|0.155|0.362|276.298|0.016|0.000|
|spdlog-async|0.080|1245.301|0.803|0.136|740.019|0.080|0.000|
|mal-blocking|0.004|26094.860|0.038|0.125|800.975|0.004|0.000|
|mal-bounded|0.004|26063.684|0.038|0.124|807.341|0.004|0.000|
|g3log|0.812|123.322|8.109|0.000|0.000|0.812|0.000|
|spdlog-sync|0.096|1045.710|0.956|0.096|1045.451|0.096|0.000|
|glog|0.275|363.511|2.751|0.275|363.498|0.275|0.000|

#### "Real" latency (threads: 1, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19489.750|0.183|27.565|
|mal-hybrid|15879.500|0.156|21.740|
|nanolog|12050.750|0.310|32.350|
|spdlog-async|24022.250|0.931|73.344|
|mal-blocking|18640.250|0.128|19.431|
|mal-bounded|19520.250|0.120|15.645|
|g3log|28033.750|8.248|174.525|
|spdlog-sync|83.250|0.993|0.768|
|glog|638.500|2.874|4.996|

#### CPU latency (threads: 1, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|21.932|0.254|0.053|
|mal-hybrid|19.694|0.226|0.061|
|nanolog|506.157|0.308|0.515|
|spdlog-async|76.181|0.598|0.183|
|mal-blocking|52.951|0.205|0.062|
|mal-bounded|73.361|0.205|0.063|
|g3log|538.863|3.362|1.433|
|spdlog-sync|73.275|1.139|0.746|
|glog|630.668|2.975|4.686|

### threads: 2, msgs: 100k ###

#### Throughput (threads: 2, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.017|6461.484|0.155|0.123|812.225|0.026|0.000|
|mal-hybrid|0.013|7712.721|0.130|0.128|781.342|0.013|0.000|
|nanolog|0.022|4465.725|0.224|0.355|281.401|0.022|0.000|
|spdlog-async|0.050|2012.625|0.497|0.125|805.472|0.086|0.000|
|mal-blocking|0.009|11202.494|0.089|0.122|817.632|0.009|0.000|
|mal-bounded|0.009|11409.536|0.088|0.124|806.955|0.009|0.000|
|g3log|0.326|306.989|3.257|0.000|0.000|0.513|0.000|
|spdlog-sync|0.120|832.269|1.202|0.120|831.610|0.192|0.000|
|glog|0.479|209.815|4.766|0.479|209.781|0.925|0.000|

#### "Real" latency (threads: 2, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|87.500|0.176|0.242|
|mal-hybrid|79.500|0.178|0.242|
|nanolog|198.500|0.273|0.427|
|spdlog-async|24016.500|0.931|47.250|
|mal-blocking|107.000|0.145|0.220|
|mal-bounded|56.000|0.138|0.218|
|g3log|40273.250|5.587|170.160|
|spdlog-sync|141.500|1.999|2.570|
|glog|661.500|7.462|8.744|

#### CPU latency (threads: 2, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|79.075|0.390|0.279|
|mal-hybrid|47.747|0.354|0.257|
|nanolog|6499.022|0.414|3.760|
|spdlog-async|98.454|0.855|0.481|
|mal-blocking|74.206|0.293|0.227|
|mal-bounded|54.019|0.292|0.224|
|g3log|9382.525|3.037|7.585|
|spdlog-sync|133.803|2.046|2.023|
|glog|687.909|6.686|6.120|

### threads: 4, msgs: 100k ###

#### Throughput (threads: 4, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.018|5781.139|0.173|0.123|812.776|0.049|0.000|
|mal-hybrid|0.014|7115.702|0.141|0.126|794.667|0.039|0.000|
|nanolog|0.024|4267.693|0.234|0.352|284.254|0.061|0.000|
|spdlog-async|0.038|2670.144|0.375|0.118|847.126|0.111|0.000|
|mal-blocking|0.021|4813.408|0.208|0.125|800.694|0.060|0.000|
|mal-bounded|0.020|5080.244|0.197|0.123|810.513|0.056|0.000|
|g3log|0.179|560.854|1.783|0.000|0.000|0.492|0.000|
|spdlog-sync|0.144|695.919|1.437|0.144|695.416|0.455|0.000|
|glog|0.450|222.322|4.498|0.450|222.265|1.644|0.000|

#### "Real" latency (threads: 4, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|121.250|0.455|0.431|
|mal-hybrid|123.000|0.457|0.478|
|nanolog|4349.750|0.613|3.654|
|spdlog-async|16030.250|1.140|30.704|
|mal-blocking|100.500|0.484|0.418|
|mal-bounded|158.500|0.478|0.433|
|g3log|36020.000|5.664|125.962|
|spdlog-sync|706.750|4.352|6.799|
|glog|2295.250|15.197|17.262|

#### CPU latency (threads: 4, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|72.314|0.541|0.368|
|mal-hybrid|98.614|0.523|0.378|
|nanolog|951.634|0.688|1.441|
|spdlog-async|103.435|1.140|0.691|
|mal-blocking|96.815|0.494|0.343|
|mal-bounded|94.035|0.492|0.343|
|g3log|15168.028|3.801|17.532|
|spdlog-sync|132.129|3.975|4.117|
|glog|2205.162|9.900|8.379|

### threads: 8, msgs: 100k ###

#### Throughput (threads: 8, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6297.593|0.159|0.126|793.678|0.055|0.000|
|mal-hybrid|0.020|5110.275|0.196|0.125|798.860|0.078|0.000|
|nanolog|0.029|3657.853|0.273|0.362|276.470|0.132|0.000|
|spdlog-async|0.035|2905.908|0.344|0.117|856.914|0.160|0.000|
|mal-blocking|0.025|4061.749|0.246|0.131|764.182|0.104|0.000|
|mal-bounded|0.024|4224.710|0.237|0.129|777.518|0.099|0.000|
|g3log|0.146|685.904|1.458|0.000|0.000|0.771|0.000|
|spdlog-sync|0.150|669.974|1.493|0.150|667.209|0.804|0.000|
|glog|0.504|198.813|5.030|0.504|198.749|3.380|0.000|

#### "Real" latency (threads: 8, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|12040.500|0.621|27.671|
|mal-hybrid|14972.500|0.725|31.968|
|nanolog|24769.500|1.247|80.002|
|spdlog-async|14110.500|1.698|71.066|
|mal-blocking|14916.750|0.825|42.719|
|mal-bounded|19687.250|0.830|43.403|
|g3log|34450.500|8.819|126.256|
|spdlog-sync|37555.250|8.364|175.083|
|glog|63764.750|36.931|579.794|

#### CPU latency (threads: 8, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|86.771|0.576|0.368|
|mal-hybrid|99.840|0.560|0.398|
|nanolog|14229.456|0.837|26.972|
|spdlog-async|1736.617|1.134|1.047|
|mal-blocking|91.449|0.552|0.357|
|mal-bounded|87.792|0.547|0.360|
|g3log|8501.732|3.827|19.887|
|spdlog-sync|152.350|3.135|4.065|
|glog|2181.181|7.289|11.654|

### threads: 16, msgs: 100k ###

#### Throughput (threads: 16, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.015|6742.716|0.148|0.126|791.385|0.071|0.000|
|mal-hybrid|0.020|5013.416|0.199|0.127|790.356|0.126|0.000|
|nanolog|0.037|2852.462|0.351|0.373|268.062|0.330|0.000|
|spdlog-async|0.033|3077.488|0.325|0.121|830.847|0.208|0.000|
|mal-blocking|0.024|4263.428|0.235|0.130|770.330|0.174|0.000|
|mal-bounded|0.023|4439.139|0.225|0.130|773.291|0.182|0.000|
|g3log|0.124|809.439|1.235|0.000|0.000|1.349|0.000|
|spdlog-sync|0.162|621.487|1.609|0.163|616.928|1.707|0.000|
|glog|0.543|184.644|5.416|0.543|184.394|6.693|0.000|

#### "Real" latency (threads: 16, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|16540.250|0.982|69.769|
|mal-hybrid|16662.750|1.162|67.606|
|nanolog|64541.500|3.525|253.794|
|spdlog-async|22786.250|2.151|105.994|
|mal-blocking|16374.000|1.366|80.866|
|mal-bounded|16313.250|1.339|77.278|
|g3log|29539.000|15.140|184.956|
|spdlog-sync|50913.250|18.929|339.724|
|glog|167705.250|75.012|1179.944|

#### CPU latency (threads: 16, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|1746.151|0.565|1.786|
|mal-hybrid|85.952|0.571|0.379|
|nanolog|19987.002|1.291|66.231|
|spdlog-async|2538.479|1.125|1.613|
|mal-blocking|80.050|0.570|0.340|
|mal-bounded|98.740|0.571|0.339|
|g3log|4626.937|3.897|17.189|
|spdlog-sync|138.740|3.360|4.353|
|glog|2171.168|7.050|11.818|

> Written with [StackEdit](https://stackedit.io/).
