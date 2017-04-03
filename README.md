
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
test a sensible throughput degradation is seen in it when the thread count
increases. For the 16 thread case "mal-heap" is 5.35x faster. The latency
behavior is better too: less standard deviation and shortest worst-case.

### mal-hybrid ####

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

On the 1M (queue full) tests depending and on the thread count "mal-blocking"
seems to be around 3-20% faster and to have around 2x-10x better worst case
latency.

On the 100k (queue big enough) single threaded test with low thread count (more
realistic load) it's significantly slower, specially on the uncontended single
threaded case, where "mal-sync" seems to be around 21x faster. The worst case
latencies in this case are on par.

This is logical, as a matter of fact "spdlog-async" internals are similar to
"mal-blocking"'s; they both use internally the same D.Vyukov bounded queue
algorithm,(gabime borrowed the idea from this project when spdlog was using
mutexes) so the performance difference is not made in the queue algorithm, but
in its usage.

When the queue algorithm is the bottleneck (with high thread count and a lot of
contention), the performance of "spdlog-async" is similar to the performance
of the bounded mal variations.

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
"mal-blocking" and its worst-case latency is much worse. Keep in mind that
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

### Result data

### threads: 1, msgs: 1M ###

#### Throughput (threads=1, msgs=1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.105|9499.887|0.105|1.172|853.245|0.105|0.000|
|mal-hybrid|0.146|6881.365|0.145|1.223|818.596|0.146|0.000|
|nanolog|0.349|2865.832|0.349|3.719|268.860|0.349|0.000|
|spdlog-async|1.103|906.483|1.103|1.503|665.403|1.103|0.000|
|mal-blocking|0.876|1141.988|0.876|1.156|864.822|0.876|0.000|
|mal-bounded|0.046|6041.580|0.166|0.335|829.794|0.046|722011.320|
|g3log|8.744|114.392|8.742|0.000|0.000|8.744|0.000|
|spdlog-sync|0.963|1038.046|0.963|0.963|1038.015|0.963|0.000|
|glog|2.880|347.194|2.880|2.880|347.190|2.880|0.000|

#### Latency with thread clock (threads=1, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.241|0.045|0.000|86.502|
|mal-hybrid|0.229|0.054|0.000|78.741|
|nanolog|0.347|4.332|0.000|10262.499|
|spdlog-async|0.618|0.152|0.000|77.650|
|mal-blocking|0.204|0.081|0.000|74.332|
|mal-bounded|0.198|0.036|0.000|74.056|
|g3log|3.516|1.421|0.000|1898.100|
|spdlog-sync|1.141|2.129|0.000|1184.429|
|glog|3.091|4.447|0.000|623.927|

#### Latency with wall clock (threads=1, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.260|39.466|0.000|19783.750|
|mal-hybrid|0.260|39.803|0.000|19887.750|
|nanolog|0.478|53.367|0.000|23828.250|
|spdlog-async|1.095|86.731|0.000|32126.500|
|mal-blocking|0.904|134.936|0.000|38612.750|
|mal-bounded|0.177|32.769|0.000|15909.250|
|g3log|9.027|176.479|0.000|36099.750|
|spdlog-sync|1.008|2.126|0.000|1184.500|
|glog|2.957|4.991|0.000|10870.250|

### threads: 2, msgs: 1M ###

#### Throughput (threads=2, msgs=1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.127|7851.252|0.127|1.213|824.499|0.194|0.000|
|mal-hybrid|0.118|8538.300|0.117|1.217|822.024|0.189|0.000|
|nanolog|0.280|3579.617|0.279|3.684|271.478|0.479|0.000|
|spdlog-async|1.093|914.847|1.093|1.390|722.730|1.658|0.000|
|mal-blocking|0.872|1146.352|0.872|1.155|866.106|1.301|0.000|
|mal-bounded|0.038|7560.663|0.132|0.328|849.906|0.048|721508.667|
|g3log|3.614|276.795|3.613|0.000|0.000|5.550|0.000|
|spdlog-sync|1.296|772.051|1.295|1.296|771.985|2.550|0.000|
|glog|4.145|241.551|4.140|4.145|241.545|8.282|0.000|

#### Latency with thread clock (threads=2, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.356|0.208|0.000|446.603|
|mal-hybrid|0.339|0.145|0.000|74.559|
|nanolog|0.511|14.626|0.000|19604.226|
|spdlog-async|0.871|1.014|0.000|501.413|
|mal-blocking|0.325|0.245|0.000|81.224|
|mal-bounded|0.233|0.104|0.000|73.537|
|g3log|2.959|15.843|0.000|39682.811|
|spdlog-sync|2.385|2.953|0.000|1512.499|
|glog|6.353|5.798|0.000|620.568|

#### Latency with wall clock (threads=2, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.323|35.966|0.000|19945.250|
|mal-hybrid|0.331|36.788|0.000|26595.250|
|nanolog|0.571|48.877|0.000|36013.250|
|spdlog-async|1.541|402.998|0.000|407353.750|
|mal-blocking|1.592|176.466|0.000|47105.000|
|mal-bounded|0.167|26.099|0.000|26374.500|
|g3log|5.776|198.746|0.000|86820.750|
|spdlog-sync|2.531|4.186|0.000|1622.250|
|glog|7.077|8.951|0.000|13852.500|

### threads: 4, msgs: 1M ###

#### Throughput (threads=4, msgs=1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.126|7937.776|0.126|1.230|813.398|0.424|0.000|
|mal-hybrid|0.109|9145.287|0.109|1.229|813.995|0.384|0.000|
|nanolog|0.295|3410.801|0.293|3.694|270.721|1.096|0.000|
|spdlog-async|1.033|971.724|1.029|1.278|785.271|2.931|0.000|
|mal-blocking|0.857|1166.439|0.857|1.142|875.657|2.563|0.000|
|mal-bounded|0.045|6535.512|0.153|0.333|842.747|0.136|719399.893|
|g3log|2.076|481.684|2.076|0.000|0.000|5.793|0.000|
|spdlog-sync|1.611|621.825|1.608|1.611|621.782|6.375|0.000|
|glog|4.064|246.107|4.063|4.064|246.098|16.109|0.000|

#### Latency with thread clock (threads=4, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.462|0.194|0.000|474.910|
|mal-hybrid|0.461|0.187|0.000|109.648|
|nanolog|0.902|29.667|0.000|33627.726|
|spdlog-async|2.066|1.980|0.000|550.799|
|mal-blocking|0.590|0.529|0.000|105.001|
|mal-bounded|0.295|0.200|0.000|100.486|
|g3log|3.574|40.379|0.000|47607.687|
|spdlog-sync|4.502|15.216|0.000|51418.669|
|glog|9.800|7.232|0.000|2016.700|

#### Latency with wall clock (threads=4, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.452|28.741|0.000|19820.000|
|mal-hybrid|0.458|30.415|0.000|19833.750|
|nanolog|1.012|56.331|0.000|37315.750|
|spdlog-async|3.078|786.347|0.000|819219.500|
|mal-blocking|3.256|248.487|0.000|44787.000|
|mal-bounded|0.256|23.307|0.000|25956.000|
|g3log|5.938|166.843|0.000|48068.000|
|spdlog-sync|5.656|45.203|0.000|55947.500|
|glog|15.057|16.018|0.000|3920.500|

### threads: 8, msgs: 1M ###

#### Throughput (threads=8, msgs=1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.112|8969.594|0.111|1.240|806.566|0.653|0.000|
|mal-hybrid|0.106|9499.564|0.105|1.240|806.327|0.650|0.000|
|nanolog|0.386|2625.303|0.381|3.773|265.072|2.607|0.000|
|spdlog-async|0.956|1046.983|0.955|1.338|748.533|5.357|0.000|
|mal-blocking|0.886|1129.817|0.885|1.169|855.640|5.360|0.000|
|mal-bounded|0.054|5278.950|0.189|0.343|822.408|0.304|718061.400|
|g3log|1.393|718.022|1.393|0.000|0.000|8.191|0.000|
|spdlog-sync|1.362|734.524|1.361|1.363|734.224|10.022|0.000|
|glog|5.299|188.800|5.297|5.299|188.775|40.806|0.000|

#### Latency with thread clock (threads=8, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.462|0.193|0.000|485.315|
|mal-hybrid|0.468|0.276|0.000|574.191|
|nanolog|1.251|110.635|0.000|163074.074|
|spdlog-async|1.598|1.671|0.000|537.990|
|mal-blocking|0.564|0.532|0.000|102.706|
|mal-bounded|0.293|0.210|0.000|85.461|
|g3log|3.578|31.940|0.000|36182.697|
|spdlog-sync|2.774|8.095|0.000|32767.595|
|glog|7.241|10.994|0.000|1979.518|

#### Latency with wall clock (threads=8, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.789|70.550|0.000|24013.250|
|mal-hybrid|0.789|70.572|0.000|24033.500|
|nanolog|2.344|208.913|0.000|165345.250|
|spdlog-async|5.313|1063.950|0.000|823471.250|
|mal-blocking|6.430|393.892|0.000|60707.000|
|mal-bounded|0.481|53.689|0.000|24006.250|
|g3log|9.408|196.593|0.000|68008.250|
|spdlog-sync|10.798|266.851|0.000|63681.500|
|glog|42.027|643.568|0.000|95095.000|

### threads: 16, msgs: 1M ###

#### Throughput (threads=16, msgs=1M) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.106|9512.044|0.105|1.249|800.829|1.105|0.000|
|mal-hybrid|0.095|10539.606|0.095|1.259|794.185|1.142|0.000|
|nanolog|0.574|1776.734|0.563|3.954|253.007|7.361|0.000|
|spdlog-async|0.946|1057.712|0.945|1.311|764.723|10.080|0.000|
|mal-blocking|0.926|1081.371|0.925|1.210|826.498|11.169|0.000|
|mal-bounded|0.066|4224.109|0.237|0.357|781.058|0.727|721052.893|
|g3log|1.106|905.473|1.104|0.000|0.000|14.164|0.000|
|spdlog-sync|1.375|727.978|1.374|1.376|727.064|19.834|0.000|
|glog|5.446|183.668|5.445|5.448|183.624|81.815|0.000|

#### Latency with thread clock (threads=16, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.459|0.185|0.000|487.014|
|mal-hybrid|0.471|0.261|0.000|485.797|
|nanolog|1.792|120.518|0.000|69922.390|
|spdlog-async|1.538|1.692|0.000|1433.414|
|mal-blocking|0.563|0.595|0.000|757.408|
|mal-bounded|0.288|0.215|0.000|77.137|
|g3log|3.642|37.741|0.000|28622.650|
|spdlog-sync|2.508|8.844|0.000|21796.912|
|glog|6.666|10.537|0.000|2279.296|

#### Latency with wall clock (threads=16, msgs=1M) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|1.475|138.667|0.000|36011.000|
|mal-hybrid|1.485|140.511|0.000|36012.250|
|nanolog|7.367|640.868|0.000|464987.500|
|spdlog-async|9.929|1533.480|0.000|824011.500|
|mal-blocking|12.916|645.065|0.000|264012.750|
|mal-bounded|0.881|106.345|0.000|36008.500|
|g3log|16.615|261.344|0.000|107831.750|
|spdlog-sync|21.590|458.307|0.000|99859.250|
|glog|85.751|1313.894|0.000|204089.000|

### threads: 1, msgs: 100k ###

#### Throughput (threads=1, msgs=100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6375.228|0.157|0.127|789.601|0.016|0.000|
|mal-hybrid|0.006|16009.559|0.062|0.130|771.471|0.006|0.000|
|nanolog|0.015|6561.511|0.152|0.369|270.683|0.015|0.000|
|spdlog-async|0.080|1250.099|0.800|0.140|715.535|0.080|0.000|
|mal-blocking|0.004|26903.095|0.037|0.125|797.623|0.004|0.000|
|mal-bounded|0.004|26863.993|0.037|0.125|798.509|0.004|0.000|
|g3log|0.830|120.741|8.282|0.000|0.000|0.830|0.000|
|spdlog-sync|0.094|1060.233|0.943|0.094|1059.962|0.094|0.000|
|glog|0.250|400.607|2.496|0.250|400.591|0.250|0.000|

#### Latency with thread clock (threads=1, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.237|0.065|0.000|73.584|
|mal-hybrid|0.219|0.058|0.000|23.001|
|nanolog|0.307|0.548|0.000|507.379|
|spdlog-async|0.609|0.182|0.000|77.888|
|mal-blocking|0.204|0.055|0.000|16.185|
|mal-bounded|0.204|0.055|0.000|15.487|
|g3log|3.382|1.583|0.000|578.957|
|spdlog-sync|1.127|0.726|0.000|83.000|
|glog|2.744|4.699|0.000|610.724|

#### Latency with wall clock (threads=1, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.191|29.040|0.000|18694.250|
|mal-hybrid|0.153|19.869|0.000|18198.500|
|nanolog|0.304|32.053|0.000|12054.500|
|spdlog-async|0.919|71.433|0.000|24022.250|
|mal-blocking|0.126|18.277|0.000|15800.500|
|mal-bounded|0.123|16.843|0.000|15923.500|
|g3log|8.461|178.166|0.000|35768.750|
|spdlog-sync|0.989|0.741|0.000|88.250|
|glog|2.607|5.041|0.000|1768.000|

### threads: 2, msgs: 100k ###

#### Throughput (threads=2, msgs=100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.015|6993.205|0.143|0.123|812.608|0.023|0.000|
|mal-hybrid|0.014|7475.601|0.134|0.129|777.838|0.014|0.000|
|nanolog|0.023|4393.427|0.228|0.364|275.067|0.023|0.000|
|spdlog-async|0.049|2067.928|0.484|0.126|799.563|0.084|0.000|
|mal-blocking|0.009|11348.432|0.088|0.123|811.886|0.009|0.000|
|mal-bounded|0.009|11755.607|0.085|0.125|803.583|0.009|0.000|
|g3log|0.326|307.722|3.250|0.000|0.000|0.521|0.000|
|spdlog-sync|0.122|819.163|1.221|0.122|818.546|0.196|0.000|
|glog|0.495|202.496|4.938|0.495|202.466|0.958|0.000|

#### Latency with thread clock (threads=2, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.365|0.261|0.000|74.722|
|mal-hybrid|0.345|0.245|0.000|37.425|
|nanolog|0.418|3.256|0.000|6547.597|
|spdlog-async|0.860|0.466|0.000|78.877|
|mal-blocking|0.295|0.227|0.000|40.778|
|mal-bounded|0.294|0.226|0.000|79.924|
|g3log|2.942|6.123|0.000|4248.991|
|spdlog-sync|2.056|2.058|0.000|138.149|
|glog|6.045|6.134|0.000|639.601|

#### Latency with wall clock (threads=2, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.182|0.243|0.000|58.500|
|mal-hybrid|0.176|0.241|0.000|69.000|
|nanolog|0.278|0.426|0.000|99.500|
|spdlog-async|0.942|49.640|0.000|24019.500|
|mal-blocking|0.144|0.221|0.000|54.000|
|mal-bounded|0.140|0.221|0.000|85.500|
|g3log|5.650|171.274|0.000|40647.750|
|spdlog-sync|2.037|2.601|0.000|253.000|
|glog|7.611|8.894|0.000|672.250|

### threads: 4, msgs: 100k ###

#### Throughput (threads=4, msgs=100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.017|5816.477|0.172|0.124|806.399|0.049|0.000|
|mal-hybrid|0.014|7369.058|0.136|0.127|789.789|0.038|0.000|
|nanolog|0.024|4204.168|0.238|0.360|277.912|0.061|0.000|
|spdlog-async|0.037|2714.878|0.368|0.117|851.558|0.108|0.000|
|mal-blocking|0.021|4830.749|0.207|0.125|803.084|0.060|0.000|
|mal-bounded|0.020|5089.425|0.196|0.124|807.103|0.056|0.000|
|g3log|0.181|555.494|1.800|0.000|0.000|0.506|0.000|
|spdlog-sync|0.150|667.746|1.498|0.150|667.284|0.474|0.000|
|glog|0.452|221.417|4.516|0.452|221.360|1.633|0.000|

#### Latency with thread clock (threads=4, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.530|0.368|0.000|97.979|
|mal-hybrid|0.519|0.380|0.000|100.225|
|nanolog|0.698|1.111|0.000|667.632|
|spdlog-async|1.145|1.029|0.000|2419.031|
|mal-blocking|0.487|0.339|0.000|82.305|
|mal-bounded|0.487|0.335|0.000|80.414|
|g3log|3.744|15.762|0.000|12496.331|
|spdlog-sync|4.037|4.205|0.000|130.493|
|glog|9.847|8.395|0.000|2212.302|

#### Latency with wall clock (threads=4, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.457|0.419|0.000|119.750|
|mal-hybrid|0.456|1.358|0.000|4031.500|
|nanolog|0.624|1.935|0.000|1022.250|
|spdlog-async|1.139|30.480|0.000|13164.750|
|mal-blocking|0.476|1.344|0.000|4035.750|
|mal-bounded|0.473|0.423|0.000|120.250|
|g3log|5.810|130.534|0.000|36022.000|
|spdlog-sync|4.362|6.812|0.000|296.500|
|glog|15.221|16.243|0.000|2084.750|

### threads: 8, msgs: 100k ###

#### Throughput (threads=8, msgs=100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6440.547|0.155|0.127|784.807|0.053|0.000|
|mal-hybrid|0.020|5064.253|0.197|0.126|797.569|0.078|0.000|
|nanolog|0.029|3543.887|0.282|0.369|271.005|0.134|0.000|
|spdlog-async|0.034|2964.436|0.337|0.116|860.729|0.156|0.000|
|mal-blocking|0.025|4057.172|0.246|0.130|768.689|0.106|0.000|
|mal-bounded|0.025|4095.472|0.244|0.129|774.193|0.101|0.000|
|g3log|0.148|677.489|1.476|0.000|0.000|0.779|0.000|
|spdlog-sync|0.149|675.800|1.480|0.149|675.075|0.773|0.000|
|glog|0.470|213.186|4.691|0.471|212.904|3.109|0.000|

#### Latency with thread clock (threads=8, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.555|0.361|0.000|228.484|
|mal-hybrid|0.546|0.401|0.000|106.212|
|nanolog|0.850|27.824|0.000|14481.546|
|spdlog-async|1.130|0.935|0.000|898.256|
|mal-blocking|0.541|0.347|0.000|55.388|
|mal-bounded|0.545|0.352|0.000|82.037|
|g3log|3.780|19.125|0.000|9078.383|
|spdlog-sync|3.031|4.047|0.000|182.458|
|glog|6.839|11.920|0.000|2228.419|

#### Latency with wall clock (threads=8, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.622|27.524|0.000|12054.750|
|mal-hybrid|0.740|34.288|0.000|14870.500|
|nanolog|1.225|78.700|0.000|31323.000|
|spdlog-async|1.698|71.400|0.000|15102.750|
|mal-blocking|0.842|44.671|0.000|14928.000|
|mal-bounded|0.835|43.339|0.000|14709.000|
|g3log|9.030|140.305|0.000|32022.750|
|spdlog-sync|8.448|192.520|0.000|30112.000|
|glog|33.577|553.300|0.000|70830.500|

### threads: 16, msgs: 100k ###

#### Throughput (threads=16, msgs=100k) ####

|logger|enqueue(s)|rate(Kmsg/s)|latency(us)|total(s)|disk(Kmsg/s)|thread time(s)|faults|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.016|6583.656|0.152|0.130|768.453|0.076|0.000|
|mal-hybrid|0.018|5648.697|0.177|0.131|764.880|0.114|0.000|
|nanolog|0.038|2704.247|0.370|0.383|260.937|0.353|0.000|
|spdlog-async|0.032|3122.116|0.320|0.125|801.972|0.204|0.000|
|mal-blocking|0.026|3851.225|0.260|0.133|750.795|0.195|0.000|
|mal-bounded|0.025|4079.836|0.245|0.133|754.916|0.198|0.000|
|g3log|0.125|802.922|1.245|0.000|0.000|1.337|0.000|
|spdlog-sync|0.164|613.156|1.631|0.165|608.699|1.740|0.000|
|glog|0.505|198.595|5.035|0.506|198.310|6.170|0.000|

#### Latency with thread clock (threads=16, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.546|2.715|0.000|3393.759|
|mal-hybrid|0.552|0.352|0.000|83.013|
|nanolog|1.348|68.047|0.000|21244.445|
|spdlog-async|1.128|1.525|0.000|2455.316|
|mal-blocking|0.566|0.350|0.000|92.111|
|mal-bounded|0.564|0.338|0.000|91.244|
|g3log|3.803|16.043|0.000|4505.704|
|spdlog-sync|3.240|4.327|0.000|192.859|
|glog|6.610|12.025|0.000|2161.373|

#### Latency with wall clock (threads=16, msgs=100k) ####

|logger|mean(us)|standard deviation|best(us)|worst(us)|
|:-:|:-:|:-:|:-:|:-:|
|mal-heap|0.899|62.431|0.000|16884.750|
|mal-hybrid|1.214|73.406|0.000|16083.000|
|nanolog|3.461|236.301|0.000|55229.000|
|spdlog-async|2.403|121.832|0.000|20874.750|
|mal-blocking|1.450|81.302|0.000|16016.250|
|mal-bounded|1.454|81.542|0.000|15466.250|
|g3log|15.087|190.852|0.000|29835.250|
|spdlog-sync|19.025|352.546|0.000|56399.500|
|glog|69.034|1112.627|0.000|180189.000|

> Written with [StackEdit](https://stackedit.io/).
