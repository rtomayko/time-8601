Time::iso8601
=============

An efficient `Time::iso8601` implementation for Ruby. This doesn't implement all
of ISO8601, only the simple format supported by ruby core's `time.rb`. These
formats are accepted:

 - `2013-02-25T18:30:00` - Time in local timezone.
 - `2013-02-25T18:30:00Z` - Time in UTC.
 - `2013-02-25T18:30:00-08:00` - Time with specific UTF offset.
 - `2013-02-25T18:30:00.007` - Time with usec. Optional with any timezone format.

The library is designed to minimize overhead in constructing many Time objects
from time strings, both CPU use and object allocation are minimized.

Benchmarks
----------

These are time profile results from parsing 1,000 ISO8601 time strings and
turning them into Time objects. The baseline `Time.new` results are given for
contrast.

    $ time ruby iso8601-bench.rb
    ruby 1.9.3p231-tcs-github (2012-05-25, TCS patched 2012-05-27, GitHub v1.0.5) [x86_64-darwin12.2.1]
                                                   user     system      total        real
    Baselines (1000x)
      1 + 1                                    0.000000   0.000000   0.000000 (  0.000064)
      Time::new()                              0.000000   0.000000   0.000000 (  0.000491)
      Time::new with args, no tz offset        0.010000   0.010000   0.020000 (  0.010604)
      Time::new with args and tz offset        0.000000   0.000000   0.000000 (  0.003347)

    Parsing "2013-02-25T01:48:14-05:00" (1000x)
      Time::iso8601                            0.010000   0.000000   0.010000 (  0.015421)
      Time::strptime                           0.010000   0.000000   0.010000 (  0.014631)
      Marshal::load                            0.010000   0.000000   0.010000 (  0.003465)

    GC: 1 runs, 0.000000s

`Time::iso8601` is roughly 4x slower than `Time::new` with same arguments and is
30x slower than `Time::new` without arguments (current time).

Submission Status
-----------------

If a good implementation can be reached, patches will be submitted to Ruby core
to replace the existing `Time::iso8601` implementation in `time.rb`. This hasn't
happened.

License
-------

COPYRIGHT (C) 2013 BY   R Y A N   T O M A Y K O   <http://tomayko.com>
