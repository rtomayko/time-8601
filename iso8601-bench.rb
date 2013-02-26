require 'time'
require 'benchmark'
include Benchmark

class Time
  class <<self
    def iso8601_local(date)
      year, mon, day = date[0, 4], date[5, 2], date[8, 2]
      hour, min, sec = date[11, 2], date[14, 2], date[17, 2]
      tz = date[19..-1]
      if tz != ""
        offset = (tz == 'Z' ? 0 : (tz[1, 2].to_i * 60) + tz[4, 2].to_i)
        new(year.to_i, mon.to_i, day.to_i, hour.to_i, min.to_i, sec.to_i, offset)
      else
        new(year.to_i, mon.to_i, day.to_i, hour.to_i, min.to_i, sec.to_i)
      end
    end
  end
end


times = [
  "2013-02-25T01:48:14",
  "2013-02-25T01:48:14-08:00",
  "2013-02-25T01:48:14Z",
]
iterations = 1_000

times.each do |string|
  time = Time.iso8601(string)
  marshaled = Marshal.dump(time)
  puts "===> #{string} over #{iterations} iterations"

  bm(40) do |x|
    x.report "Time::new" do
      iterations.times { Time.new }
    end

    x.report "Time::new with args" do
      iterations.times { Time.new(2013, 2, 25, 1, 48, 14, 300) }
    end

    x.report "Time::iso8601" do
      iterations.times { Time.iso8601(string) }
    end

    x.report "Time::iso8601_local" do
      iterations.times { Time.iso8601_local(string) }
    end

    x.report "Time::strptime" do
      iterations.times { Time.strptime(string, "%FT%T") }
    end

    x.report "Marshal.load" do
      iterations.times { Marshal.load(marshaled) }
    end
  end
end
