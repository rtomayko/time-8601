Time::iso8601
=============

A more efficient `Time::iso8601` implementation for Ruby. This doesn't implement
all of ISO8601, only the simple format supported by ruby core's `time.rb`. These
formats are accepted:

 - `2013-02-25T18:30:00` - Time in local timezone.
 - `2013-02-25T18:30:00Z` - Time in UTC.
 - `2013-02-25T18:30:00-08:00` - Time with specific UTF offset.

The library is designed to minimize overhead in constructing many Time objects
from time strings, in both CPU and object allocation.

Usage
-----

Build it:

    $ git clone https://github.com/rtomayko/time-iso8601.git
    $ cd time-iso8601
    $ rake

The `time-iso8601` extension adds a single method named `iso8601_at` to the
core `Time` class. Use it exactly like `Time::iso8601`:

    require 'time-iso8601'
    time = Time.iso8601_at("2013-02-25T18:30:00Z")

Benchmarks
----------

These are benchmark results from parsing 1000 ISO8601 time strings in each of
the supported formats and turning them into `Time` objects. The baseline results
are given for contrast.

    $ rake
    ruby 1.9.3p231-tcs-github-tcmalloc (2012-05-25, TCS patched 2012-05-27, GitHub v1.0.10) [x86_64-linux]
                                                   user     system      total        real
    Baselines (1000x)
      1 + 1                                    0.000000   0.000000   0.000000 (  0.000089)
      Time::new                                0.000000   0.000000   0.000000 (  0.000537)
      Time::new with tz offset                 0.000000   0.000000   0.000000 (  0.004182)
      Time::new without tz offset              0.000000   0.010000   0.010000 (  0.007652)
      Time::utc                                0.000000   0.000000   0.000000 (  0.004713)
      Time::mktime                             0.010000   0.000000   0.010000 (  0.007471)
      Time::at(unixtime)                       0.000000   0.000000   0.000000 (  0.000288)

    Parsing 2013-02-25T01:48:14 (1000x)
      Time::iso8601                            0.000000   0.010000   0.010000 (  0.011225)
      Time::iso8601_at                         0.000000   0.000000   0.000000 (  0.008117)
      Marshal::load                            0.000000   0.000000   0.000000 (  0.008962)

    Parsing 2013-02-25T01:48:14Z (1000x)
      Time::iso8601                            0.010000   0.000000   0.010000 (  0.016503)
      Time::iso8601_at                         0.010000   0.000000   0.010000 (  0.005368)
      Marshal::load                            0.010000   0.000000   0.010000 (  0.004921)

    Parsing 2013-02-25T01:48:14-05:00 (1000x)
      Time::iso8601                            0.010000   0.000000   0.010000 (  0.018974)
      Time::iso8601_at                         0.010000   0.000000   0.010000 (  0.004864)
      Marshal::load                            0.000000   0.000000   0.000000 (  0.004923)

`Time::at` is between 56x and 94x faster (depending on local vs utc and tz
offset) than `Time::iso8601` in these results. This is the number that led to
this project being started.

Investigation
-------------

In profiling github.com we found that it can take ~20ms to turn 1,000 ISO8601
time strings into `Time` objects via the `Time::iso8601` method. Some actions
operate on potentially thousands to tens of thousands of time strings.  In some
cases, we're spending as much as 50ms dealing with `Time` objects within a
single request.

We assumed this slowness was due to parsing and object allocation overhead since
the `Time::iso8601` implementation is written in Ruby with Regexps. In reality,
the slowness is almost entirely due to all of the concerns with moving from a
unixtime to a broken down time.

Ruby's `Time` class has some pretty serious performance issues when dealing with
"broken down times". A broken down time is when you have the individual parts of
a time like year, month, day, hour, min, sec, etc. as opposed to the unixtime
number of seconds since 1970.

Ruby's `Time` class uses a unixtime-like value internally for most operations,
so any time you're creating a `Time` object from broken down time parts, it has
to do a ton of work to figure out the unixtime first. It might not seem like
that'd require more than a little multiplication and division but timezones,
DST, leapseconds, and leap years make moving between unixtime and broken down
times a much more complex operation than you'd think.

Ruby's implementation of these conversions isn't particularly slow when compared
to various libc `mktime(3)` implementations, which turn a broken down time into
a unixtime.

Results
-------

The `Time::iso8601_at` implementation provided by this library is about 5x
faster than the stock `Time::iso8601` method. While this is a decent speed up,
it's nowhere near what we'd hoped to achieve, which was something closer to
baseline `Time::at` performance, or a 94x / nearly two orders of magnitude speed
up.

We will not be rolling this library out to github.com production. Instead we've
decided to rework various parts of the stack to transmit time information as
simple unixtime values, allowing the use of `Time::at` directly.

Status
------

This project was an experiment and is not under active development.

License
-------

COPYRIGHT (C) 2013 BY   R Y A N   T O M A Y K O   <http://tomayko.com>
