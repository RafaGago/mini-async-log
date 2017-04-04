
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

Regarding the worst case latency for the 1 thread case: when looking at
the thread clock it looks like the asynchronous loggers have always a lower
worst-case latency (lower=better), but when looking at the wall clock the
synchronous loggers have a much lower worst-case (lower=better).

There are many hypotheses to explain why this happens, but excluding clock
deficiencies I suspect that the asynchronous loggers are "touching" more memory
and they get minor page faults sometimes. When page faulting the threads get
suspended so this isn't reflected on the thread clock, only on the wall clock.

The results on the "mal" thread clock, which has standard deviations on the
sub 0.01 range, support this hypothesis. The algorithms are fast and have low
variability on the single thread case: something is happening ot the OS level.

So here's the catch, big bounded queues have more chances of page faulting,
smaller have more changes of being overflown: The size has to be fine-tuned
for the expected load.

#### mal-heap ####

It shows very stable results in the throughput and latency measurements. The
latency increases with more contention but it's still lower than all non "mal"
variants except for the synchronous loggers with low contention/thread count.

The only competing logger using a heap based queue is nanolog. In the 1M msgs
test a sensible throughput degradation is seen in nanolog when the thread count
increases. For the single thread case mal is significantly faster, the
difference grows with more threads because nanolog doesn't scale.

On the 100k test they start on a similar level but nanolog degrades worse.

The latency of mal is alwasy better: less standard deviation and shortest
worst-case.

#### mal-hybrid ####

"mal-hybrid" is just adding a small bounded queue to "mal-heap". On the 1M test
having the extra bounded queue seems to increase the performance except in the
single threaded case.

In the 100k single threaded test it's the opposite, the bounded queue increases
performance up to two 4 threads (CPU core count), after that the performance is
less than "mal-heap".

The latencies are both very similar.

Note that 100k messages and <4 threads is maybe the more realistic load on this
benchmark.

#### spdlog-async ####

Shows decent performance for a bounded queue logger.

"mal-blocking" uses a bounded queue that blocks on full input too.

On the 1M (queue full) tests depending and on the thread count "mal-blocking"
seems to be around 3-20% faster and to have around 2x-10x better worst case
latency.

On the 100k (queue big enough) single threaded test with low thread count (more
realistic load) it's significantly outperformed by mal, specially on the
uncontended single threaded case. The worst case latencies in this case are on
par.

This is logical, as a matter of fact "spdlog-async" internals are similar to
"mal-blocking"'s; they both use internally the same D.Vyukov bounded queue
algorithm,(gabime borrowed the idea from this project when spdlog was using
mutexes) so the performance difference is not made in the queue algorithm, but
in its usage. When the queue algorithm is the bottleneck (with high thread count
and a lot of contention), the performance of "spdlog-async" is similar to the
performance of the bounded mal variations.

#### g3log ####

It shows modest performance. Its single threaded performance uncontended (most
common situation) is the lowest of all the contenders on the single threaded
tests.

The good thing is that performance improves along with the thread count and it
doesn't show extreme big worst-case latencies when on very big contention.

#### nanolog ####

Just a look at the code reveals what the results are going to be. In case the
producers are contended they just spin burning cycles, always, without backing
off, it just busy-waits burning CPU and starving other threads. You can see it
when looking at latency tables: both tables show less divergence than the other
loggers.

The code shows that when it needs to allocate it uses a very big chunk (8MB
according to the code). It is good to preallocate, but in unconfigurable 8MB
chunks?

It shows good performance compared with the bounded queue contenders on the 1M
messages test when the thread count is low. This is hardly a surprise because
it uses an unbounded algorithm, so it doesn't need to deal with the full-queue
case.

With 16 threads and 1M messages its throughput is around 70% better than
"mal-blocking" and its worst-case latency is worse. Keep in mind that
"mal-sync" doesn't use a bounded queue and its producers sleep under contention.

On the 100k test, when the bounded queue versions of mal have enough room on the
queue (average use case) it's outperformed in every metric . "spdlog-async"
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

On the 100k case with low thread count mal-blocking/mal-bounded are the fastest
of all.

For the 1M msgs case when "mal-blocking" needs to back-off and wait for the
consumer (the consumer rate is slower) its performance drops and some producers
are more starved than others beacause there is some unfairness on its waiting
scheme, albeit lower than in a traditional exponential backoff scheme.

Keep in mind that in mal the same bounded FIFO queue doubles as a FIFO and
as a memory allocator (using customizations on D.Vjukov algorithm), so if the
serialized message size doesn't fit on the queue entry size the messages are
discarded. This requires either a good entry size selection that you know that
fits all the messages or using "mal-hybrid" (which is what the library intends).

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
|mal-heap|0.106|9439.507|0.106|1.140|877.246|0.106|0.000|
|mal-hybrid|0.146|6878.846|0.145|1.196|836.162|0.146|0.000|
|nanolog|0.353|2833.411|0.353|3.619|276.296|0.353|0.000|
|spdlog-async|1.165|858.319|1.165|1.568|637.590|1.165|0.000|
|mal-blocking|0.851|1175.662|0.851|1.126|888.469|0.851|0.000|
|mal-bounded|0.046|6095.156|0.164|0.328|847.918|0.046|721912.667|
|g3log|8.768|114.080|8.766|0.000|0.000|8.768|0.000|
|spdlog-sync|1.007|993.392|1.007|1.007|993.363|1.007|0.000|
|glog|2.671|374.434|2.671|2.671|374.430|2.671|0.000|

#### "Real" latency (threads: 1, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|15885.000|0.265|39.920|
|mal-hybrid|15898.250|0.266|40.386|
|nanolog|16514.000|0.483|53.491|
|spdlog-async|32123.250|1.137|87.587|
|mal-blocking|38386.000|0.883|133.320|
|mal-bounded|15920.500|0.174|31.906|
|g3log|36139.000|8.981|176.702|
|spdlog-sync|1243.750|1.050|2.219|
|glog|49269.750|2.761|9.851|

#### CPU latency (threads: 1, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|87.821|0.242|0.063|
|mal-hybrid|80.408|0.232|0.071|
|nanolog|7991.661|0.350|3.985|
|spdlog-async|89.969|0.640|0.174|
|mal-blocking|78.286|0.207|0.088|
|mal-bounded|75.265|0.201|0.055|
|g3log|1650.541|3.507|1.336|
|spdlog-sync|1279.821|1.177|2.201|
|glog|668.296|2.916|4.478|

### threads: 2, msgs: 1M ###

#### Throughput (threads: 2, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.128|7809.447|0.128|1.187|842.588|0.196|0.000|
|mal-hybrid|0.116|8640.490|0.116|1.191|839.688|0.188|0.000|
|nanolog|0.276|3640.956|0.275|3.588|278.754|0.471|0.000|
|spdlog-async|1.106|903.978|1.106|1.502|666.165|1.676|0.000|
|mal-blocking|0.848|1180.007|0.847|1.124|889.476|1.247|0.000|
|mal-bounded|0.039|7364.085|0.136|0.319|877.990|0.049|719543.613|
|g3log|3.586|278.890|3.586|0.000|0.000|5.529|0.000|
|spdlog-sync|1.343|745.848|1.341|1.343|745.788|2.643|0.000|
|glog|4.479|223.644|4.471|4.479|223.640|8.952|0.000|

#### "Real" latency (threads: 2, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19947.000|0.322|36.458|
|mal-hybrid|26336.000|0.332|35.851|
|nanolog|16028.000|0.563|47.387|
|spdlog-async|416772.000|1.598|399.793|
|mal-blocking|46722.500|1.545|173.404|
|mal-bounded|26192.250|0.170|25.977|
|g3log|70973.500|5.758|198.342|
|spdlog-sync|1639.500|2.578|4.158|
|glog|51099.500|7.556|12.245|

#### CPU latency (threads: 2, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|90.440|0.357|0.151|
|mal-hybrid|85.092|0.343|0.155|
|nanolog|12113.688|0.519|16.238|
|spdlog-async|509.067|0.991|2.282|
|mal-blocking|94.573|0.325|0.247|
|mal-bounded|83.856|0.234|0.114|
|g3log|36069.776|2.968|14.827|
|spdlog-sync|1549.817|2.384|2.956|
|glog|615.535|6.561|5.879|

### threads: 4, msgs: 1M ###

#### Throughput (threads: 4, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.126|7938.846|0.126|1.202|832.064|0.425|0.000|
|mal-hybrid|0.109|9161.937|0.109|1.205|830.219|0.382|0.000|
|nanolog|0.284|3546.148|0.282|3.594|278.301|1.056|0.000|
|spdlog-async|1.045|959.260|1.042|1.333|753.986|3.002|0.000|
|mal-blocking|0.841|1189.197|0.841|1.119|893.841|2.514|0.000|
|mal-bounded|0.040|7399.144|0.135|0.321|874.220|0.117|719736.373|
|g3log|2.050|487.817|2.050|0.000|0.000|5.750|0.000|
|spdlog-sync|1.622|619.191|1.615|1.622|619.151|6.404|0.000|
|glog|4.139|241.631|4.139|4.139|241.622|16.397|0.000|

#### "Real" latency (threads: 4, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|19922.000|0.458|29.705|
|mal-hybrid|25965.000|0.458|30.505|
|nanolog|24838.750|0.989|53.389|
|spdlog-async|824745.250|3.103|792.755|
|mal-blocking|48373.500|3.204|246.011|
|mal-bounded|19774.750|0.257|22.382|
|g3log|48227.500|5.908|164.397|
|spdlog-sync|48314.000|5.734|38.193|
|glog|40510.500|15.479|19.851|

#### CPU latency (threads: 4, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|484.745|0.466|0.229|
|mal-hybrid|107.481|0.465|0.199|
|nanolog|17853.539|0.882|28.059|
|spdlog-async|471.822|2.085|1.995|
|mal-blocking|119.633|0.593|0.535|
|mal-bounded|92.470|0.300|0.207|
|g3log|52528.471|3.559|41.145|
|spdlog-sync|52104.440|4.450|14.149|
|glog|3326.454|10.150|7.275|

### threads: 8, msgs: 1M ###

#### Throughput (threads: 8, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.112|9016.956|0.111|1.213|824.480|0.650|0.000|
|mal-hybrid|0.105|9556.211|0.105|1.223|818.017|0.649|0.000|
|nanolog|0.401|2555.217|0.391|3.705|269.951|2.770|0.000|
|spdlog-async|0.976|1024.671|0.976|1.368|732.165|5.453|0.000|
|mal-blocking|0.870|1149.549|0.870|1.149|870.495|5.220|0.000|
|mal-bounded|0.057|5048.009|0.198|0.337|837.383|0.323|717636.387|
|g3log|1.382|723.740|1.382|0.000|0.000|8.192|0.000|
|spdlog-sync|1.405|712.198|1.404|1.406|711.949|10.411|0.000|
|glog|5.144|194.471|5.142|5.145|194.441|39.579|0.000|

#### "Real" latency (threads: 8, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|24019.250|0.790|70.386|
|mal-hybrid|24013.750|0.796|71.116|
|nanolog|190358.250|2.384|222.503|
|spdlog-async|828008.250|5.474|1079.145|
|mal-blocking|122138.000|6.294|388.685|
|mal-bounded|24013.250|0.479|54.009|
|g3log|52308.750|9.360|199.095|
|spdlog-sync|46720.750|11.113|251.574|
|glog|85228.500|41.098|642.852|

#### CPU latency (threads: 8, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|452.466|0.465|0.202|
|mal-hybrid|488.421|0.471|0.275|
|nanolog|125720.111|1.170|81.148|
|spdlog-async|577.736|1.672|1.732|
|mal-blocking|131.500|0.566|0.535|
|mal-bounded|87.406|0.296|0.218|
|g3log|41114.183|3.529|32.118|
|spdlog-sync|26581.961|3.054|8.112|
|glog|2613.243|7.000|11.074|

### threads: 16, msgs: 1M ###

#### Throughput (threads: 16, msgs: 1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.107|9431.366|0.106|1.214|823.998|1.103|0.000|
|mal-hybrid|0.094|10598.259|0.094|1.226|815.983|1.129|0.000|
|nanolog|0.589|1730.768|0.578|3.894|256.868|7.656|0.000|
|spdlog-async|0.980|1021.346|0.979|1.339|748.703|10.227|0.000|
|mal-blocking|0.890|1124.271|0.889|1.168|856.152|10.649|0.000|
|mal-bounded|0.067|4200.588|0.238|0.349|802.745|0.737|719960.480|
|g3log|1.076|930.519|1.075|0.000|0.000|13.722|0.000|
|spdlog-sync|1.420|704.480|1.419|1.423|703.348|20.537|0.000|
|glog|5.274|189.690|5.272|5.274|189.660|79.148|0.000|

#### "Real" latency (threads: 16, msgs: 1M, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|36013.000|1.464|137.730|
|mal-hybrid|36013.250|1.470|138.995|
|nanolog|376027.250|6.913|601.028|
|spdlog-async|832372.750|10.155|1590.006|
|mal-blocking|226071.000|12.571|631.377|
|mal-bounded|36011.750|0.897|107.970|
|g3log|99101.500|16.407|267.347|
|spdlog-sync|91268.500|21.815|441.968|
|glog|208788.500|83.016|1309.373|

#### CPU latency (threads: 16, msgs: 1M, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|84.478|0.462|0.186|
|mal-hybrid|499.284|0.473|0.220|
|nanolog|69376.032|1.764|119.433|
|spdlog-async|791.316|1.604|1.738|
|mal-blocking|256.985|0.569|0.591|
|mal-bounded|74.043|0.294|0.227|
|g3log|25403.596|3.588|39.124|
|spdlog-sync|15026.155|2.731|5.545|
|glog|2234.957|6.431|10.603|

### threads: 1, msgs: 100k ###

#### Throughput (threads: 1, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6289.491|0.159|0.124|804.266|0.016|0.000|
|mal-hybrid|0.006|15941.866|0.063|0.128|778.947|0.006|0.000|
|nanolog|0.015|6514.371|0.154|0.354|282.662|0.015|0.000|
|spdlog-async|0.098|1028.634|0.972|0.145|690.070|0.098|0.000|
|mal-blocking|0.004|26442.015|0.038|0.124|805.491|0.004|0.000|
|mal-bounded|0.004|26346.006|0.038|0.124|805.341|0.004|0.000|
|g3log|0.794|126.192|7.924|0.000|0.000|0.794|0.000|
|spdlog-sync|0.097|1034.339|0.967|0.097|1034.083|0.097|0.000|
|glog|0.265|377.234|2.651|0.265|377.219|0.265|0.000|

#### "Real" latency (threads: 1, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|15941.500|0.227|36.744|
|mal-hybrid|15931.500|0.218|37.523|
|nanolog|12082.500|0.306|31.832|
|spdlog-async|24022.000|1.087|78.774|
|mal-blocking|15927.750|0.157|28.639|
|mal-bounded|18961.000|0.165|30.696|
|g3log|36060.750|8.131|176.175|
|spdlog-sync|83.250|1.023|0.775|
|glog|718.750|2.747|5.023|

#### CPU latency (threads: 1, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|76.550|0.242|0.088|
|mal-hybrid|87.976|0.224|0.092|
|nanolog|434.037|0.312|0.542|
|spdlog-async|82.975|0.691|0.210|
|mal-blocking|71.373|0.208|0.072|
|mal-bounded|71.379|0.208|0.074|
|g3log|552.272|3.341|1.570|
|spdlog-sync|96.574|1.146|0.754|
|glog|637.613|2.870|4.711|

### threads: 2, msgs: 100k ###

#### Throughput (threads: 2, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.017|6562.956|0.152|0.123|817.340|0.025|0.000|
|mal-hybrid|0.014|7357.125|0.136|0.125|798.883|0.014|0.000|
|nanolog|0.021|4750.074|0.211|0.346|288.861|0.021|0.000|
|spdlog-async|0.056|1779.670|0.562|0.127|789.977|0.097|0.000|
|mal-blocking|0.009|11605.597|0.086|0.122|816.869|0.009|0.000|
|mal-bounded|0.009|10886.797|0.092|0.122|820.564|0.009|0.000|
|g3log|0.321|312.110|3.204|0.000|0.000|0.513|0.000|
|spdlog-sync|0.123|810.650|1.234|0.124|810.033|0.198|0.000|
|glog|0.533|187.922|5.321|0.533|187.895|1.033|0.000|

#### "Real" latency (threads: 2, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|98.500|0.190|0.261|
|mal-hybrid|86.000|0.189|0.258|
|nanolog|93.000|0.280|0.451|
|spdlog-async|24021.000|1.039|46.513|
|mal-blocking|115.500|0.146|0.227|
|mal-bounded|113.750|0.146|0.231|
|g3log|48020.500|5.462|167.522|
|spdlog-sync|256.500|2.012|2.569|
|glog|1852.250|8.100|9.179|

#### CPU latency (threads: 2, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|89.421|0.344|0.258|
|mal-hybrid|79.162|0.313|0.213|
|nanolog|4521.303|0.422|1.848|
|spdlog-async|82.463|0.941|0.490|
|mal-blocking|96.005|0.284|0.229|
|mal-bounded|70.455|0.285|0.223|
|g3log|4163.162|2.944|6.727|
|spdlog-sync|102.571|2.042|2.032|
|glog|658.081|6.812|6.165|

### threads: 4, msgs: 100k ###

#### Throughput (threads: 4, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.018|5659.060|0.177|0.123|814.564|0.049|0.000|
|mal-hybrid|0.015|6494.585|0.154|0.125|797.198|0.044|0.000|
|nanolog|0.023|4325.884|0.231|0.343|291.547|0.061|0.000|
|spdlog-async|0.039|2571.495|0.389|0.120|836.034|0.118|0.000|
|mal-blocking|0.021|4870.926|0.205|0.125|803.434|0.060|0.000|
|mal-bounded|0.021|4711.819|0.212|0.126|793.449|0.061|0.000|
|g3log|0.179|559.847|1.786|0.000|0.000|0.491|0.000|
|spdlog-sync|0.147|680.953|1.469|0.147|680.462|0.466|0.000|
|glog|0.453|220.645|4.532|0.453|220.589|1.652|0.000|

#### "Real" latency (threads: 4, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|87.500|0.426|0.424|
|mal-hybrid|102.000|0.425|0.474|
|nanolog|662.500|0.638|1.568|
|spdlog-async|16035.500|1.214|30.175|
|mal-blocking|140.500|0.444|0.440|
|mal-bounded|147.250|0.448|0.428|
|g3log|32021.250|5.584|123.366|
|spdlog-sync|245.250|4.301|6.741|
|glog|12380.750|15.366|18.782|

#### CPU latency (threads: 4, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|80.445|0.552|0.403|
|mal-hybrid|102.677|0.537|0.406|
|nanolog|3747.178|0.702|2.373|
|spdlog-async|872.343|1.241|0.788|
|mal-blocking|73.867|0.512|0.371|
|mal-bounded|75.517|0.507|0.367|
|g3log|14192.837|3.672|17.429|
|spdlog-sync|139.781|3.962|4.166|
|glog|2125.836|9.964|8.367|

### threads: 8, msgs: 100k ###

#### Throughput (threads: 8, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6460.841|0.155|0.126|791.689|0.053|0.000|
|mal-hybrid|0.018|5627.070|0.178|0.123|814.300|0.069|0.000|
|nanolog|0.029|3607.094|0.277|0.353|283.549|0.131|0.000|
|spdlog-async|0.036|2789.200|0.359|0.118|847.201|0.176|0.000|
|mal-blocking|0.025|4111.818|0.243|0.130|769.593|0.106|0.000|
|mal-bounded|0.024|4333.825|0.231|0.130|769.272|0.099|0.000|
|g3log|0.145|690.786|1.448|0.000|0.000|0.761|0.000|
|spdlog-sync|0.149|672.811|1.486|0.149|672.123|0.789|0.000|
|glog|0.489|205.205|4.873|0.490|204.942|3.235|0.000|

#### "Real" latency (threads: 8, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|12059.500|0.575|25.011|
|mal-hybrid|13878.500|0.714|32.645|
|nanolog|38012.000|1.208|73.687|
|spdlog-async|16029.250|1.806|74.099|
|mal-blocking|19762.000|0.768|43.870|
|mal-bounded|19837.750|0.760|43.513|
|g3log|28030.000|8.727|127.049|
|spdlog-sync|35685.500|8.400|189.415|
|glog|83449.500|35.175|571.295|

#### CPU latency (threads: 8, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|984.072|0.582|0.668|
|mal-hybrid|92.632|0.570|0.409|
|nanolog|11935.776|0.847|24.631|
|spdlog-async|2443.657|1.211|1.440|
|mal-blocking|84.416|0.579|0.375|
|mal-bounded|75.323|0.577|0.368|
|g3log|10709.557|3.762|21.077|
|spdlog-sync|137.823|3.009|3.976|
|glog|2046.264|7.080|11.887|

### threads: 16, msgs: 100k ###

#### Throughput (threads: 16, msgs: 100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.015|6676.020|0.150|0.129|777.355|0.070|0.000|
|mal-hybrid|0.016|6107.878|0.164|0.130|770.332|0.100|0.000|
|nanolog|0.040|2672.196|0.374|0.368|272.213|0.353|0.000|
|spdlog-async|0.035|2845.103|0.351|0.120|835.918|0.254|0.000|
|mal-blocking|0.026|3927.394|0.255|0.132|755.905|0.180|0.000|
|mal-bounded|0.025|4034.480|0.248|0.131|764.811|0.158|0.000|
|g3log|0.124|811.141|1.233|0.000|0.000|1.334|0.000|
|spdlog-sync|0.166|607.000|1.647|0.166|605.987|1.757|0.000|
|glog|0.524|191.188|5.230|0.526|190.758|6.488|0.000|

#### "Real" latency (threads: 16, msgs: 100k, clock: wall) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|16050.750|0.769|52.206|
|mal-hybrid|15105.000|1.033|63.471|
|nanolog|64017.000|3.385|233.455|
|spdlog-async|23075.000|2.568|133.269|
|mal-blocking|18518.750|1.172|70.478|
|mal-bounded|16041.500|1.177|70.472|
|g3log|27332.000|14.994|163.924|
|spdlog-sync|53792.000|19.435|362.396|
|glog|173164.500|70.735|1161.158|

#### CPU latency (threads: 16, msgs: 100k, clock: thread) ####

|logger|worst(us)|mean(us)|standard deviation|
|:-:|:-:|:-:|:-:|
|mal-heap|2901.548|0.572|3.991|
|mal-hybrid|617.192|0.567|0.477|
|nanolog|19951.827|1.322|64.806|
|spdlog-async|2033.800|1.177|1.681|
|mal-blocking|108.104|0.576|0.452|
|mal-bounded|96.332|0.568|0.441|
|g3log|4659.070|3.811|16.536|
|spdlog-sync|148.064|3.224|4.302|
|glog|2174.871|6.884|11.909|


> Written with [StackEdit](https://stackedit.io/).
