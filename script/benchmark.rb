puts "==> ISO8601 performance"
puts RUBY_DESCRIPTION if defined?(RUBY_DESCRIPTION)

GC::Profiler.enable if defined?(GC::Profiler)
GC.disable

require 'time'
require 'benchmark'
include Benchmark

require 'time-iso8601'

iterations = 1_000
times = [
  "2013-02-25T01:48:14",
  "2013-02-25T01:48:14Z",
  "2013-02-25T01:48:14-05:00"
]

bm(40) do |benchmark|
  puts "Baselines (#{iterations}x)"
  benchmark.report "  1 + 1" do
    iterations.times { 1 + 1 }
  end

  benchmark.report "  Time::new" do
    iterations.times { Time.new }
  end

  benchmark.report "  Time::new without tz offset" do
    iterations.times { Time.new(2013, 2, 25, 1, 48, 14) }
  end

  benchmark.report "  Time::new with tz offset" do
    iterations.times { Time.new(2013, 2, 25, 1, 48, 14, 300) }
  end

  benchmark.report "  Time::utc" do
    iterations.times { Time.utc(2013, 2, 25, 1, 48, 14) }
  end

  benchmark.report "  Time::mktime" do
    iterations.times { Time.mktime(2013, 2, 25, 1, 48, 14) }
  end

  benchmark.report "  Time::at with int" do
    iterations.times { Time.at(1361875523) }
  end

  benchmark.report "  Time::at with int and utc convert" do
    iterations.times { Time.at(1361875523).utc }
  end

  benchmark.report "  Time::at with int and local convert" do
    iterations.times { Time.at(1361875523).localtime(-5 * 60 * 60) }
  end

  benchmark.report "  Time::at with usecs" do
    iterations.times { Time.at(1361875523.6666) }
  end

  times.each do |string|
    time = Time.iso8601(string)
    marshaled = Marshal.dump(time)
    puts
    puts "Parsing #{string} (#{iterations}x)"

    benchmark.report "  Time::iso8601_strict" do
      iterations.times { Time.iso8601_strict(string) }
    end

    benchmark.report "  Time::iso8601" do
      iterations.times { Time.iso8601(string) }
    end

    # benchmark.report "  Time::strptime" do
    #   iterations.times { Time.strptime(string, "%FT%T") }
    # end

    benchmark.report "  Marshal::load" do
      iterations.times { Marshal.load(marshaled) }
    end
  end
end

if defined?(GC::Profiler)
  puts
  puts "GC: %d runs, %0.6fs" % [GC.count, GC::Profiler.total_time]
end
