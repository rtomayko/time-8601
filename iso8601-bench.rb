puts RUBY_DESCRIPTION if defined?(RUBY_DESCRIPTION)

GC::Profiler.enable if defined?(GC::Profiler)
GC.disable

require 'time'
require 'benchmark'
include Benchmark

def iso8601_local(date)
  year, mon, day = date[0, 4], date[5, 2], date[8, 2]
  hour, min, sec = date[11, 2], date[14, 2], date[17, 2]
  tz = date[19..-1]
  if tz != ""
    offset = (tz == 'Z' ? 0 : (tz[1, 2].to_i * 60) + tz[4, 2].to_i)
    Time.new(year.to_i, mon.to_i, day.to_i, hour.to_i, min.to_i, sec.to_i, offset)
  else
    Time.new(year.to_i, mon.to_i, day.to_i, hour.to_i, min.to_i, sec.to_i)
  end
end

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

  times.each do |string|
    time = Time.iso8601(string)
    marshaled = Marshal.dump(time)
    puts
    puts "Parsing #{string} (#{iterations}x)"

    benchmark.report "  Time::iso8601" do
      iterations.times { Time.iso8601(string) }
    end

    benchmark.report "  Time::iso8601_local" do
      iterations.times { iso8601_local(string) }
    end

    benchmark.report "  Time::strptime" do
      iterations.times { Time.strptime(string, "%FT%T") }
    end

    benchmark.report "  Marshal::load" do
      iterations.times { Marshal.load(marshaled) }
    end
  end
end

if defined?(GC::Profiler)
  puts
  puts "GC: %d runs, %0.6fs" % [GC.count, GC::Profiler.total_time]
end
