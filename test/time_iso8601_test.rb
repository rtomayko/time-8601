require 'minitest/autorun'
require 'time-iso8601'

class TimeISO8601Test < MiniTest::Unit::TestCase
  def setup
    @time_utc   = "2013-02-25T18:30:00Z"
    @time_local = "2013-02-25T18:30:00"
    @time_zoned = "2013-02-25T18:30:00-05:00"
  end

  def test_utc_time_parsing
    time = Time.iso8601_at("2013-02-25T18:30:00Z")
    assert_equal 2013, time.year
    assert_equal 2, time.month
    assert_equal 25, time.day
    assert_equal 18, time.hour
    assert_equal 30, time.min
    assert_equal 0, time.sec
    assert_equal 0, time.utc_offset
  end

  def test_utc_time_zone_info
    time = Time.iso8601_at("2013-02-25T18:30:00Z")
    assert_equal "UTC", time.zone
    assert time.utc?, "should be UTC"
  end

  def test_local_time_parsing
    time = Time.iso8601_at("2013-02-25T18:30:00")
    assert_equal 2013, time.year
    assert_equal 2, time.month
    assert_equal 25, time.day
    assert_equal 18, time.hour
    assert_equal 30, time.min
    assert_equal 00, time.sec
    assert_equal Time.new.utc_offset, time.utc_offset
  end

  def test_local_time_zone_info
    time = Time.iso8601_at("2013-02-25T18:30:00")
    assert !time.utc?, "should not be UTC"
    assert_equal Time.new.zone, time.zone
  end

  def test_explicit_zone_time_parsing
    time = Time.iso8601_at("2013-02-25T18:30:00-05:00")
    assert_equal 2013, time.year
    assert_equal 2, time.month
    assert_equal 25, time.day
    assert_equal 18, time.hour
    assert_equal 30, time.min
    assert_equal 0, time.sec
    assert_equal -5 * 60 * 60, time.utc_offset
  end

  def test_explicit_zone_time_zone_info
    time = Time.iso8601_at("2013-02-25T18:30:00-05:00")
    assert !time.utc?, "should not be UTC"
  end

  def test_explicit_zone_positive_offset
    time = Time.iso8601_at("2013-02-25T18:30:00+05:00")
    assert_equal 5 * 60 * 60, time.utc_offset
    assert !time.utc?, "should not be UTC"
  end

  def test_explicit_zone_minute_offset
    time = Time.iso8601_at("2013-02-25T18:30:00+05:27")
    assert_equal (5 * 60 * 60) + 27 * 60, time.utc_offset
    assert !time.utc?, "should not be UTC"
  end

  def test_explicit_zone_utc
    time = Time.iso8601_at("2013-02-25T18:30:00+00:00")
    assert_equal 0, time.utc_offset
    assert time.utc?, "should be UTC"
  end

  def test_nil_time
    assert_raises(TypeError) { Time.iso8601_at(nil) }
  end

  def test_bad_times
    assert_raises(ArgumentError) { Time.iso8601_at("blah") }
    assert_raises(ArgumentError) { Time.iso8601_at("2013-12-21") }
    assert_raises(ArgumentError) { Time.iso8601_at("2013-12-21 00:00:00") }
  end

  def test_times_before_epoch
    assert expect = Time.iso8601("1902-01-01T00:00:00Z")
    assert_equal expect, Time.iso8601_at("1902-01-01T00:00:00Z")
  end

  def test_really_small_times
    assert expect = Time.iso8601("0000-01-01T00:00:00Z")
    assert_equal expect, Time.iso8601_at("0000-01-01T00:00:00Z")
  end
end
