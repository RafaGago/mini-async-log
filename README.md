
Minimal Asynchronous Logger (MAL)
-----------
A non overly-bloated and performant asynchronous data logger.

## Credit ##

 - To my former employer **Diadrom AB.** for allowing me to share this code with
   a BSD license. They funded most of the development of this project.
 - To Dmitry Vjukov for all the knowledge he has spread on the internets,
   including the algorithms for the two queues on this project.
 - To my girlfriend for coexisting with me when I become temporarily autistic
   after having been "in the zone" for too long.

## Motivation ##

This started with the intention to just develop an asynchronous logger that
could be used from many dynamically loaded libraries without doing link-time
hacks like linking static, hiding symbols and some other "niceties".

Then at some point way after the requirements were met I just improved the thing
for fun.

Who needs a fast asynchronous logger? Anyone using an asynchronous logger,
otherwise they would just use a synchronous one.

## Design rationale ##

 - Simple. Not over abstracted and feature bloated, explicit, easy to figure out
   what the code is doing, easy to modify (2017 self-comment: not that easy
   after having switched to raw C coding for a while :D).
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

 - Targeting g++4.7 and VS 2010 (can take parts from boost)
 - Nanosecond performance.
 - No singleton by design, usable from dynamically loaded libraries. The user
   provides the instance either explicitly or by a global function (Koenig
   lookup).
 - Suitable for soft-realtime work. Once it's initialized the fast-path can be
   clear from heap allocations if properly configured.
 - File rotation-slicing (needs some external help at initialization time until
   std::filesystem is implemented on some compilers, see below).
 - One conditional call overhead for inactive severities.
 - Able to strip log levels at compile time (for Release builds).
 - Lazy parameter evaluation (as usual with most logging libraries).
 - No ostreams (a very ugly part of C++: verbose and stateful), just format
   strings checked at compile time (if the compiler supports it) with type safe
   values.
   An on-stack ostream adapter is available as a last resort, but its use is
   more verbose and has more overhead than the format literals.
 - The logger severity threshold can be externally changed outside of the
   process. The IPC mechanism is the simplest, the log worker periodically polls
   some files when idlíng (if configured to).
 - Fair blocking behaviour (configurable by severity) when the bounded queue is
   full and the heap queue is disabled. The logger smoothly starts to act as a
   synchronous logger. If not blocking is desired an error is returned.
 - Small, you can actually compile it as a part of your application.

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
copy of the integer value. The job of the caller is just to serialize some bytes
to a memory chunk and to insert the chunk into a queue.

The queue is a custom modification of two famous lockfree queues of Dmitry
Vyukov (kudos to this genious) for this particular MPSC case. The queue is a
blend of a fixed capacity and fixed element size array based preallocated queue
and an intrusive node based dynamically allocated queue. The resulting queue is
still linearizable.

The file worker thread pops from the queue, decodes the message and writes the
data. Any time consuming operation is masked with the slow file IO.

The format string is type-safe and validated at compile time for compilers that
support "constexpr" and "variadic template parameters" available. Otherwise the
errors are caught at run time on the logged output (Visual Studio 2010 mostly).

There are other features, as to block the caller thread until some message has
been written (as in an asynchronous logger) or as to do C++ stream formatting on
the caller thread.

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
logger instance on each call. Even if many modules call the initialization
function of such instance only one of them will succeed.

There are two methods to pass the instance to the logging macros when enqueuing
a message, one is to provide it explicitly and the other one is by providing it
on a global function.

If no instance is provided, the global function "get_mal_logger_instance()" will
be called without being namespace qualified, so you can use Koenig lookup/ADL
and provide it from there. This happens when the user calls the macros with no
explicit instance suffix, as e.g. "log_error(fmt string, ...)".

To provide the instance explictly the macros with the "_i" suffix need to be
called, e.g. "log_error_i(instance, fmt_string, ...)"

The name of the function can be changed at compile time, by defining
MAL_GET_LOGGER_INSTANCE_FUNCNAME.

## Termination ##

The worker blocks on its destructor until its work queue is empty when normally
exiting a program.

When a signal is caught you can call the frontend function [on termination](https://github.com/RafaGago/mini-async-log/blob/master/include/mal_log/frontend.hpp).
This will early interrupt any synchronous calls you made.

## Errors ##

As for now, every function returns a boolean if it succeeded or false if it
didn't. A filtered out/below severity call returns true.

The only possible failures are either to be unable to allocate memory for a log
entry or an asynchronous call that was interrupted by "on_termination".

The logging functions never throw.

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

## Performace ##

These are some test I have done for fun to see how this code is aging.

> [Here is the benchmark code.](https://github.com/RafaGago/mini-async-log/blob/master/example/benchmark/main.cpp).

To build it on Linux you don't need to install any of the libraries, all of them
are downloaded and build for you by the makefile, just run:

> make -f build/linux/Makefile.examples.benchmark

You can "make install" or search for the executable under
"build/linux/build/stage"

### Test metodology ###

It consists in enqueueing 1 million and 100K messages distributing them evenly
accross a variying number of threads (1, 2, 4, 8, 16). Each test is run 75 times
and then averaged (best and worst latencies aren't averaged, the best and worst
of all runs is taken).

The different message counts (1M and 100K) are intended to show the behavior
of the bounded queue loggers. With 100K the bounded queue loggers (spdlog and
some mal variants) have a big enough queue with room for all the messages. With
the 1M test they get its queue full and need to back-off.

On the latency tests the mean, standard deviation, best and worst case is shown.
The standard deviation gives an idea of the jitter. The less the better.

The latency tests are measured with both a thread clock and a wall clock. The
thread clock shows only cycles spent on the thread, not when the thread is
suspended. The wall clock gives an idea of the "real" timing of the logger when
the OS scheduler puts threads to sleep, so the different loggers will show its
worst case in a more realistic way.

Keep in mind that measuring latencies have its quirks. The clock sources aren't
reliable either for measuring individual calls in the nanosecond scale. I do
them because I can see value in seeing standard deviations over very long runs
and the worst latencies allow to detect blocked producers (e.g.due to backoff
unfairness).

The average latency shown in the latency tests is the one shown by the OS
clocks. The average latency shown on the throughput test is more reliable, as
its taken just using two clock time points for the whole batch of messages.

The test is run in a system using Ubuntu 16.04 server, no X server and with all
network interfaces disabled. The machine is a downclocked and undervolted AMD
Phenom x4 965 with an SSD disk. Expect more performance with a modern machine.

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

The results are after these comments, otherwise this would be buried in tables.

I have observed some variability (5-10% ?) on the test results between test
runs, which shows that 75 averaged runs is not enough. I want to be able to run
the benchmark overnight, so I won't increase this number.

The variability between machines (AMD-Intel) is big too.

Said that, and being as biased as I am, I consider the mal variants the only
asynchronous loggers of the ones tested here that can be severely stressed and
still have a controlled behavior in all the tested parameters: the performance
is always respectable and the worst case latency never goes out of control.

An analisys logger by logger is presented below.

#### mal-heap ####

It shows very stable results in the throughput and latency measurements. The
latency increases with more contention but it's still lower than all non "mal"
variants except for the synchronous loggers with low contention/thread count.

The only competing logger using a heap based queue is nanolog.

On the 1M msgs test a sensible throughput degradation is seen in nanolog when
the thread count increases. For the single thread case mal is significantly
faster, the difference grows with more threads. It's outclassed by a wide
margin by mal-heap on every metric, e.g the throughput with 16 threads is 535%
higher on mal-heap.

On the 100k test they start on a similar level (nanolog has a 5% edge) but
nanolog degrades with the thread count too. At 16 threads mal is 255% faster.

With the worst case latency it's the same. nanolog starts with

#### mal-hybrid ####

"mal-hybrid" is just adding a small bounded queue to "mal-heap".

On the 1M test having the extra bounded queue seems to increase the performance
except in the single threaded case.

In the 100k single threaded test it's the opposite, the bounded queue increases
performance up to two 4 threads (CPU core count), after that the performance is
less than "mal-heap".

The latencies are both very similar.

Against nanolog "mal-hybrid" is superior in every case.

Note that 100k messages and <4 threads is maybe the most "realistic" load on
this benchmark.

#### spdlog-async ####

Shows decent performance for a bounded queue logger.

"mal-blocking" is the other logger having a similar configuration: bounded queue
with blocking behavior when the queue is full, so it's fair to establish a com-
parison.

On the 1M (queue full) tests and depending on the thread count "mal-blocking"
seems to be around 5-20% faster and to have significantly shorter worst case
latency (2000-4000%).

On the 100k (queue big enough) single threaded test with low thread count (more
realistic load) it's significantly slower, specially on the uncontended single
threaded case, where "mal-sync" seems to be around 2100% faster. The worst case
latencies are on par.

The performances at 100k start to converge with more contention/threads (while
mal is still 20% faster with 16 threads), this is logical as both are based on
the D.Vjukov queue (historically the spdlog dev borrowed the idea from this
project when spdlog was still using mutexes), so when the queue is on extremely
high contention the differences decrease.

#### g3log ####

It shows a very modest performance. On the single threaded test with 1M messages
it's not even performing above the synchronous loggers. It starts to take off
with more threads but it never reaches the standard of other loggers tested
here.

I wonder if I have something misconfigured.

The only positive performance aspect that I can see given these results it's
that it never reaches the 800ms worst-case latency that is seen on spdlog-async
and stays at 100ms.

#### nanolog ####

Just a look at the code reveals what the results are going to be. In case the
producers are contended they just spin burning cycles, always, without backing
off, it just busy-waits burning CPU and starving other threads. You can see it
when looking at latency tables: both tables show less divergence than the other
loggers.

The code shows that when it needs to allocate it uses a very big chunk (8MB
according to the code). It is good to preallocate, but in unconfigurable 8MB
chunks?

Even the file worker doesn't back-off for long. There is no idling concept on
nanolog.

So I can safely says that it achieves its performance by burning cycles and
resources left and right.

It shows good performance compared with the bounded queue contenders on the 1M
messages test when the thread count is low. This is hardly a surprise because
it uses an unbounded algorithm, so it doesn't need to deal with the full-queue
case.

With 16 threads and 1M messages its throughput is around 70% better than
"mal-blocking" and its worst-case latency is 1100% worse. Keep in mind that
"mal-sync" doesn't use a bounded queue and its producers sleep under contention.

Against mal-heap/mal-hybrid it loses in every metric in all the tests, the only
exception being the 100k single threaded test, where it has a slight throughput
edge over mal-heap and a better worst case latency. On the 16 thread case
it reaches a 366ms worst case latency.

Compared against the mal bounded variants, with 16 threads and 1M messages its
throughput is around 60% better than "mal-blocking" and its worst-case latency
is much worse. Keep in mind that "mal-sync" doesn't use a bounded queue and
its producers sleep under contention. This is hardly an acomplishment.

On the 100k test, when the bounded queue versions of mal have enough room on the
queue (average use case) it's outperformed in every metric. "spdlog-async"
catches up on the 16 thread case too.

#### glog ####

Included for reference. It never claimed to be fast, but its worst-case latency
(according to the wall clock) when the thread count is low is the best after
spdlog-sync.

#### mal-blocking / mal-bounded ####

These are mal with no heap, just using Vjukovs queue. The difference is that
mal-blocking blocks on full queue and mal-bounded reports a failure and lets the
caller decide if it has something more useful to do before retrying.

When there are no allocation faults (faults column: 100k msgs) mal-blocking
is exactly the same as mal-bounded.

On the 100k case with low thread count mal-blocking/mal-bounded are the fastest.

For the 1M msgs case when "mal-blocking" needs to back-off and wait it's
throughput drops but the worst-case latency stays controlled and lower than
all the other asynchronous loggers. The worst case latency is still better than
the one achieved by the synchronous variants when the thread count is high (!).

Keep in mind that in mal the same bounded FIFO queue doubles as a FIFO and
as a memory allocator (using customizations on D.Vjukov algorithm), so if the
serialized message size doesn't fit on the queue entry size the messages are
discarded. This requires either a good entry size selection that you know that
fits all the messages on the program (dangerous) or to use "mal-hybrid" (which
is what the library intends).

This requirement may be dropped on the future if modifying the queue to do
multiple pushes in one atomic operation doesn't screw up the cache (producers
touching the same cache line more often because the storage is on a separated
contiguoud chunk).

#### spdlog-sync ####

Not very fast, but this is the ĺogger showing the best worst-case latency
(according to the wall clock) with only one thread.

## Benchmark data ##

### threads: 1, msgs: 1M ###

#### Throughput (threads: 1, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.103|9731.264|0.103|1.152|868.403|0.103|0.000|
|mal-hybrid|0.145|6897.777|0.145|1.199|833.894|0.145|0.000|
|nanolog|0.352|2845.275|0.351|3.682|271.628|0.352|0.000|
|spdlog-async|1.075|930.029|1.075|1.462|685.156|1.075|0.000|
|mal-blocking|0.852|1173.752|0.852|1.140|877.349|0.852|0.000|
|mal-bounded|0.051|5568.995|0.180|0.331|846.101|0.051|719814.667|
|g3log|9.014|110.982|9.010|0.000|0.000|9.014|0.000|
|spdlog-sync|0.948|1055.204|0.948|0.948|1055.171|0.948|0.000|
|glog|2.936|340.600|2.936|2.936|340.596|2.936|0.000|

#### "Real" latency (threads: 1, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|15921.000|0.268|39.898|
|mal-hybrid|15761.250|0.262|39.545|
|nanolog|36017.000|0.486|54.040|
|spdlog-async|32118.750|1.056|86.077|
|mal-blocking|20316.000|0.887|37.433|
|mal-bounded|15876.000|0.177|32.495|
|g3log|36454.500|9.258|178.717|
|spdlog-sync|1177.000|1.001|2.109|
|glog|11548.000|3.014|4.993|

#### CPU latency (threads: 1, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|77.223|0.245|0.060|
|mal-hybrid|75.189|0.231|0.057|
|nanolog|11776.148|0.353|4.767|
|spdlog-async|85.956|0.595|0.134|
|mal-blocking|85.462|0.210|0.141|
|mal-bounded|76.186|0.202|0.044|
|g3log|1636.519|3.607|1.340|
|spdlog-sync|1164.870|1.137|2.101|
|glog|586.736|3.175|4.443|

### threads: 2, msgs: 1M ###

#### Throughput (threads: 2, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.128|7840.205|0.128|1.197|835.254|0.195|0.000|
|mal-hybrid|0.120|8412.708|0.119|1.198|834.998|0.192|0.000|
|nanolog|0.281|3577.053|0.280|3.640|274.707|0.478|0.000|
|spdlog-async|1.082|924.198|1.082|1.386|725.570|1.657|0.000|
|mal-blocking|0.838|1193.415|0.838|1.126|888.372|1.276|0.000|
|mal-bounded|0.039|7289.059|0.137|0.321|873.047|0.051|720091.333|
|g3log|3.710|269.568|3.710|0.000|0.000|5.668|0.000|
|spdlog-sync|1.360|736.678|1.357|1.360|736.616|2.679|0.000|
|glog|4.079|245.282|4.077|4.080|245.276|8.150|0.000|

#### "Real" latency (threads: 2, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19836.750|0.325|36.154|
|mal-hybrid|26139.750|0.337|37.015|
|nanolog|24013.500|0.573|48.382|
|spdlog-async|415772.000|1.476|398.271|
|mal-blocking|20185.500|1.458|44.916|
|mal-bounded|26745.500|0.168|25.945|
|g3log|100278.750|6.053|198.654|
|spdlog-sync|1571.000|2.582|4.125|
|glog|2083.500|7.362|8.571|

#### CPU latency (threads: 2, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|460.821|0.360|0.259|
|mal-hybrid|466.929|0.344|0.200|
|nanolog|12064.467|0.523|16.216|
|spdlog-async|496.562|0.869|1.320|
|mal-blocking|95.882|0.359|0.817|
|mal-bounded|74.926|0.235|0.105|
|g3log|39482.076|3.024|12.121|
|spdlog-sync|1476.047|2.377|2.942|
|glog|1279.743|6.653|5.651|

### threads: 4, msgs: 1M ###

#### Throughput (threads: 4, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.128|7837.804|0.128|1.215|823.157|0.429|0.000|
|mal-hybrid|0.112|8954.537|0.112|1.212|825.141|0.392|0.000|
|nanolog|0.295|3405.411|0.294|3.658|273.351|1.097|0.000|
|spdlog-async|1.003|1000.610|0.999|1.274|788.426|2.879|0.000|
|mal-blocking|0.835|1198.380|0.834|1.124|890.025|2.539|0.000|
|mal-bounded|0.046|6265.497|0.160|0.327|864.487|0.145|717804.507|
|g3log|2.088|479.075|2.087|0.000|0.000|5.717|0.000|
|spdlog-sync|1.655|605.254|1.652|1.655|605.212|6.539|0.000|
|glog|4.021|248.691|4.021|4.022|248.682|15.928|0.000|

#### "Real" latency (threads: 4, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19908.750|0.459|29.895|
|mal-hybrid|26003.750|0.461|30.348|
|nanolog|38808.750|1.005|55.039|
|spdlog-async|817352.500|3.023|781.755|
|mal-blocking|19936.750|2.908|59.686|
|mal-bounded|19910.250|0.261|23.042|
|g3log|48071.750|6.013|170.018|
|spdlog-sync|58366.250|5.803|36.602|
|glog|50625.250|14.996|19.378|

#### CPU latency (threads: 4, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|487.331|0.469|0.228|
|mal-hybrid|109.995|0.468|0.191|
|nanolog|26829.132|0.906|30.808|
|spdlog-async|452.612|2.038|1.936|
|mal-blocking|140.048|0.678|1.432|
|mal-bounded|71.866|0.301|0.196|
|g3log|52397.746|3.616|43.709|
|spdlog-sync|41810.485|4.631|9.294|
|glog|2019.530|10.083|7.214|

### threads: 8, msgs: 1M ###

#### Throughput (threads: 8, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.111|9046.775|0.111|1.226|815.967|0.653|0.000|
|mal-hybrid|0.106|9477.125|0.106|1.230|813.039|0.652|0.000|
|nanolog|0.386|2641.245|0.379|3.743|267.190|2.604|0.000|
|spdlog-async|0.935|1070.040|0.935|1.300|771.821|5.164|0.000|
|mal-blocking|0.854|1171.466|0.854|1.142|875.573|5.240|0.000|
|mal-bounded|0.056|5120.124|0.195|0.336|841.482|0.319|717274.760|
|g3log|1.412|708.666|1.411|0.000|0.000|8.144|0.000|
|spdlog-sync|1.337|748.162|1.337|1.338|747.865|9.866|0.000|
|glog|5.330|187.693|5.328|5.330|187.684|41.027|0.000|

#### "Real" latency (threads: 8, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|24014.250|0.795|70.512|
|mal-hybrid|26144.250|0.791|69.942|
|nanolog|238768.250|2.403|213.334|
|spdlog-async|822317.500|5.163|1026.767|
|mal-blocking|22524.000|5.821|104.872|
|mal-bounded|24006.250|0.479|53.588|
|g3log|96784.750|9.441|201.723|
|spdlog-sync|71575.500|10.561|264.474|
|glog|91003.500|42.554|659.028|

#### CPU latency (threads: 8, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|81.930|0.468|0.176|
|mal-hybrid|489.042|0.473|0.248|
|nanolog|115831.987|1.178|79.945|
|spdlog-async|818.419|1.592|1.668|
|mal-blocking|582.754|0.657|1.300|
|mal-bounded|102.818|0.297|0.207|
|g3log|36160.787|3.595|33.296|
|spdlog-sync|31160.451|2.678|9.349|
|glog|1641.988|7.242|11.057|

### threads: 16, msgs: 1M ###

#### Throughput (threads: 16, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.106|9491.509|0.105|1.222|818.084|1.134|0.000|
|mal-hybrid|0.096|10434.305|0.096|1.234|810.435|1.149|0.000|
|nanolog|0.581|1752.232|0.571|3.930|254.501|7.519|0.000|
|spdlog-async|0.940|1065.202|0.939|1.283|782.497|9.865|0.000|
|mal-blocking|0.879|1137.470|0.879|1.170|854.898|11.198|0.000|
|mal-bounded|0.067|4215.358|0.237|0.346|809.264|0.726|720138.560|
|g3log|1.109|902.396|1.108|0.000|0.000|14.161|0.000|
|spdlog-sync|1.353|739.699|1.352|1.354|738.820|19.418|0.000|
|glog|5.459|183.264|5.457|5.460|183.220|82.224|0.000|

#### "Real" latency (threads: 16, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|36006.250|1.470|138.611|
|mal-hybrid|36011.250|1.475|140.046|
|nanolog|366640.500|6.729|581.031|
|spdlog-async|826169.250|9.640|1554.331|
|mal-blocking|32934.500|12.582|176.227|
|mal-bounded|36008.000|0.878|106.028|
|g3log|103242.750|16.630|266.320|
|spdlog-sync|92228.250|21.237|456.754|
|glog|236054.750|85.820|1348.902|

#### CPU latency (threads: 16, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|75.721|0.466|0.179|
|mal-hybrid|508.669|0.474|0.222|
|nanolog|68220.222|1.780|119.141|
|spdlog-async|970.301|1.550|1.682|
|mal-blocking|304.119|0.667|1.841|
|mal-bounded|75.527|0.292|0.212|
|g3log|31950.738|3.653|37.502|
|spdlog-sync|33243.511|2.466|8.753|
|glog|4864.109|6.688|10.551|

### threads: 1, msgs: 100k ###

#### Throughput (threads: 1, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.017|6229.091|0.161|0.128|780.942|0.017|0.000|
|mal-hybrid|0.006|15803.907|0.063|0.131|765.356|0.006|0.000|
|nanolog|0.015|6532.717|0.153|0.361|277.290|0.015|0.000|
|spdlog-async|0.082|1215.450|0.823|0.137|730.042|0.082|0.000|
|mal-blocking|0.004|26587.720|0.038|0.127|788.129|0.004|0.000|
|mal-bounded|0.004|26527.742|0.038|0.128|784.324|0.004|0.000|
|g3log|0.845|118.530|8.437|0.000|0.000|0.845|0.000|
|spdlog-sync|0.095|1054.902|0.948|0.095|1054.640|0.095|0.000|
|glog|0.294|340.320|2.938|0.294|340.308|0.294|0.000|

#### "Real" latency (threads: 1, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|15931.000|0.195|30.788|
|mal-hybrid|15668.500|0.157|22.386|
|nanolog|12062.000|0.310|33.138|
|spdlog-async|24020.250|0.921|72.531|
|mal-blocking|15853.750|0.121|16.913|
|mal-bounded|15896.750|0.132|21.390|
|g3log|36035.500|8.516|177.455|
|spdlog-sync|105.500|0.996|0.751|
|glog|1842.500|3.009|5.027|

#### CPU latency (threads: 1, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|77.630|0.238|0.084|
|mal-hybrid|77.902|0.220|0.091|
|nanolog|513.270|0.306|0.517|
|spdlog-async|101.899|0.593|0.183|
|mal-blocking|76.677|0.205|0.075|
|mal-bounded|76.175|0.205|0.068|
|g3log|535.287|3.380|1.582|
|spdlog-sync|106.401|1.130|0.734|
|glog|626.015|3.198|4.688|

### threads: 2, msgs: 100k ###

#### Throughput (threads: 2, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.018|6274.397|0.159|0.126|797.249|0.027|0.000|
|mal-hybrid|0.013|7898.762|0.127|0.129|772.546|0.013|0.000|
|nanolog|0.023|4399.696|0.227|0.353|283.012|0.023|0.000|
|spdlog-async|0.049|2035.741|0.491|0.123|814.354|0.085|0.000|
|mal-blocking|0.009|11294.196|0.089|0.125|798.822|0.009|0.000|
|mal-bounded|0.008|12380.739|0.081|0.126|797.178|0.008|0.000|
|g3log|0.334|300.072|3.333|0.000|0.000|0.528|0.000|
|spdlog-sync|0.125|800.271|1.250|0.125|799.651|0.200|0.000|
|glog|0.366|273.857|3.652|0.366|273.802|0.697|0.000|

#### "Real" latency (threads: 2, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|108.750|0.179|0.254|
|mal-hybrid|16023.500|0.173|5.073|
|nanolog|158.250|0.276|0.426|
|spdlog-async|24021.000|0.911|46.812|
|mal-blocking|121.000|0.142|0.227|
|mal-bounded|95.500|0.141|0.220|
|g3log|36666.500|5.529|168.542|
|spdlog-sync|248.500|2.086|2.638|
|glog|758.250|6.769|8.532|

#### CPU latency (threads: 2, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|75.980|0.362|0.263|
|mal-hybrid|78.123|0.347|0.256|
|nanolog|4280.018|0.409|1.417|
|spdlog-async|84.549|0.849|0.457|
|mal-blocking|70.881|0.292|0.219|
|mal-bounded|98.299|0.294|0.222|
|g3log|11858.539|2.969|8.581|
|spdlog-sync|136.311|2.047|2.019|
|glog|615.372|6.109|5.874|

### threads: 4, msgs: 100k ###

#### Throughput (threads: 4, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.018|5751.547|0.174|0.126|793.999|0.049|0.000|
|mal-hybrid|0.014|7227.660|0.138|0.129|778.236|0.039|0.000|
|nanolog|0.024|4165.695|0.240|0.351|285.307|0.062|0.000|
|spdlog-async|0.037|2682.639|0.373|0.118|848.510|0.109|0.000|
|mal-blocking|0.021|4753.500|0.210|0.125|798.123|0.061|0.000|
|mal-bounded|0.020|5141.856|0.194|0.124|804.476|0.056|0.000|
|g3log|0.183|549.006|1.821|0.000|0.000|0.499|0.000|
|spdlog-sync|0.150|665.711|1.502|0.151|665.252|0.478|0.000|
|glog|0.395|252.896|3.954|0.396|252.823|1.440|0.000|

#### "Real" latency (threads: 4, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|117.750|0.444|0.427|
|mal-hybrid|4026.750|0.441|1.358|
|nanolog|5125.250|0.637|3.349|
|spdlog-async|13337.000|1.127|30.566|
|mal-blocking|170.750|0.467|0.431|
|mal-bounded|115.500|0.484|0.424|
|g3log|28021.000|5.662|125.799|
|spdlog-sync|275.500|4.395|6.926|
|glog|2340.250|13.706|16.257|

#### CPU latency (threads: 4, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|86.564|0.541|0.375|
|mal-hybrid|106.038|0.521|0.394|
|nanolog|564.621|0.709|1.185|
|spdlog-async|554.027|1.137|0.700|
|mal-blocking|91.428|0.493|0.346|
|mal-bounded|100.992|0.489|0.341|
|g3log|13303.077|3.728|17.151|
|spdlog-sync|136.298|3.961|4.186|
|glog|2127.903|9.528|8.686|

### threads: 8, msgs: 100k ###

#### Throughput (threads: 8, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.017|6066.315|0.165|0.129|777.246|0.059|0.000|
|mal-hybrid|0.019|5332.438|0.188|0.126|792.341|0.076|0.000|
|nanolog|0.027|3874.368|0.258|0.361|277.096|0.119|0.000|
|spdlog-async|0.034|2937.436|0.340|0.116|864.422|0.159|0.000|
|mal-blocking|0.025|4167.024|0.240|0.132|757.416|0.099|0.000|
|mal-bounded|0.024|4157.706|0.241|0.131|764.742|0.101|0.000|
|g3log|0.150|669.797|1.493|0.000|0.000|0.797|0.000|
|spdlog-sync|0.150|669.810|1.493|0.150|667.060|0.791|0.000|
|glog|0.504|198.900|5.028|0.505|198.646|3.418|0.000|

#### "Real" latency (threads: 8, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|12047.750|0.623|27.575|
|mal-hybrid|15081.750|0.734|33.938|
|nanolog|28373.500|1.205|74.775|
|spdlog-async|14156.000|1.701|70.583|
|mal-blocking|14678.750|0.835|44.900|
|mal-bounded|14902.000|0.831|43.641|
|g3log|24033.250|8.924|136.648|
|spdlog-sync|30953.750|8.534|183.509|
|glog|61332.000|36.396|553.571|

#### CPU latency (threads: 8, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|77.895|0.562|0.375|
|mal-hybrid|127.589|0.551|0.398|
|nanolog|11957.629|0.843|24.888|
|spdlog-async|1472.393|1.134|1.037|
|mal-blocking|93.541|0.543|0.360|
|mal-bounded|92.413|0.540|0.361|
|g3log|8530.451|3.798|20.403|
|spdlog-sync|172.788|3.088|4.045|
|glog|2202.467|7.604|11.743|

### threads: 16, msgs: 100k ###

#### Throughput (threads: 16, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6524.866|0.153|0.130|771.370|0.077|0.000|
|mal-hybrid|0.025|4029.686|0.248|0.131|764.090|0.121|0.000|
|nanolog|0.041|2556.876|0.391|0.378|264.895|0.376|0.000|
|spdlog-async|0.032|3122.716|0.320|0.126|795.609|0.201|0.000|
|mal-blocking|0.026|3877.355|0.258|0.131|762.332|0.165|0.000|
|mal-bounded|0.026|3973.065|0.252|0.134|750.073|0.152|0.000|
|g3log|0.124|807.984|1.238|0.000|0.000|1.331|0.000|
|spdlog-sync|0.167|601.541|1.662|0.169|595.542|1.792|0.000|
|glog|0.547|183.366|5.454|0.548|182.967|6.890|0.000|

#### "Real" latency (threads: 16, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|16016.000|0.800|48.578|
|mal-hybrid|20027.500|1.238|74.507|
|nanolog|55443.000|3.508|242.199|
|spdlog-async|21379.500|2.408|123.336|
|mal-blocking|20022.000|1.276|79.256|
|mal-bounded|16639.500|1.390|84.611|
|g3log|35626.500|15.001|198.524|
|spdlog-sync|67680.000|19.180|355.417|
|glog|137111.000|76.035|1113.100|

#### CPU latency (threads: 16, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|2286.598|0.557|2.984|
|mal-hybrid|105.936|0.576|0.359|
|nanolog|27984.121|1.282|63.513|
|spdlog-async|2306.155|1.123|1.732|
|mal-blocking|92.825|0.562|0.355|
|mal-bounded|75.559|0.568|0.329|
|g3log|5419.923|3.825|18.176|
|spdlog-sync|218.267|3.178|4.271|
|glog|2255.306|7.341|11.911|

> Written with [StackEdit](https://stackedit.io/).
