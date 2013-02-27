/*
 * Efficient Ruby Time::iso8601 implementation.
 *
 * https://github.com/rtomayko/time-iso8601
 */

#include <ruby.h>
#include <time.h>

#define FLOOR(a) lrintf(floorf(a))
#define FLOAT(a) (float)a

static ID id_iso8601;         /* :iso8601 */
static ID id_iso8601_strict;  /* :iso8601_strict */
static ID id_new;             /* :new */
static ID id_utc;             /* :utc */
static ID id_mktime;          /* :mktime */

static inline int
_isdigit(char c)
{
	return (c >= '0' && c <= '9');
}

static inline int
_strtol(const char *str, const char **out)
{
	int n = 0;

	while (_isdigit(*str))
	{
		n *= 10;
		n += *str++ - '0';
	}

	*out = str;
	return n;
}

static time_t
time_iso8601_mktime(struct tm *time)
{
	unsigned int mon = time->tm_mon, year = time->tm_year;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;      /* Puts Feb last since it has leap day */
		year -= 1;
	}
	year += 1900;
	mon += 1;

	return ((((unsigned long)
	            (year/4 - year/100 + year/400 + 367*mon/12 + time->tm_mday) +
	            year*365 - 719499
	                  )*24 + time->tm_hour   /* now have hours */
	                  )*60 + time->tm_min    /* now have minutes */
	                  )*60 + time->tm_sec;   /* finally seconds */
}

static VALUE
time_iso8601_strict(const char * str)
{
	const char * ps = str;
	const char * pe;
	struct tm tdata;
	time_t utc_time;

	memset(&tdata, 0x0, sizeof(struct tm));
	tdata.tm_isdst = -1;

	tdata.tm_year = _strtol(ps, &pe) - 1900;
	if (pe == ps || *pe != '-')
		return Qnil;
	
	ps = pe + 1;
	tdata.tm_mon = _strtol(ps, &pe) - 1;
	if (pe == ps || *pe != '-' || tdata.tm_mon < 1 || tdata.tm_mon > 12)
		return Qnil;

	ps = pe + 1;
	tdata.tm_mday = _strtol(ps, &pe);
	if (pe == ps || *pe != 'T' || tdata.tm_mday < 1 || tdata.tm_mday > 31)
		return Qnil;

	ps = pe + 1;
	tdata.tm_hour = _strtol(ps, &pe);
	if (pe == ps || *pe != ':' || tdata.tm_hour < 0 || tdata.tm_hour > 24)
		return Qnil;

	ps = pe + 1;
	tdata.tm_min = _strtol(ps, &pe);
	if (pe == ps || *pe != ':' || tdata.tm_min < 0 || tdata.tm_min > 61)
		return Qnil;

	ps = pe + 1;
	tdata.tm_sec = _strtol(ps, &pe);
	if (pe == ps || tdata.tm_sec < 0 || tdata.tm_sec > 61)
		return Qnil;

	utc_time = time_iso8601_mktime(&tdata);

	if (*pe == '\0')
	{
		return rb_time_new(utc_time + timezone, 0);
	}
	else
	{
		VALUE rb_time;
		int hours, mins, offset;
		int negative = 0; 

		switch(*pe)
		{
			case 'Z': /* utc timezone */
				rb_time = rb_time_new(utc_time, 0);
				rb_funcall(rb_time, id_utc, 0);
				return rb_time;

			case '-':
				negative = 1;
				/* fall through */

			case '+':
				ps = pe + 1;
				hours = _strtol(ps, &pe);
				if (pe == ps || hours < 0 || hours >= 14)
					return Qnil;

				ps = pe + 1;
				mins = _strtol(ps, &pe);
				if (pe == ps || mins < 0 || mins > 59)
					return Qnil;

				offset = (mins * 60) + (hours * 60 * 60);
				if (negative)
					offset = -offset;

				rb_time = rb_time_new(utc_time - offset, 0);
				rb_funcall(rb_time, rb_intern("localtime"), 1, INT2FIX(offset));
				return rb_time;
		}
	}

	return Qnil;
}

static VALUE
rb_time_iso8601_strict(VALUE self, VALUE str)
{
	VALUE time;

	Check_Type(str, T_STRING);

	/* minumum possible ISO8601 strict time value */
	if (RSTRING_LEN(str) < 16)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	time = time_iso8601_strict(StringValueCStr(str));

	if (NIL_P(time))
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	return time;
}

static time_t
time_iso8601_strptime(const char * str)
{
	struct tm time;
	char * pe;

	if ((pe = strptime(str, "%FT%T", &time)))
	{
		/* need to parse zone info */
		/* if (pe - str < len) */

		return mktime(&time);
	}
	else
	{
		return -1;
	}
}

static VALUE
rb_time_iso8601_strptime(VALUE self, VALUE str)
{
	VALUE time;
	time_t t;

	Check_Type(str, T_STRING);

	/* minumum possible ISO8601 strict time value */
	if (RSTRING_LEN(str) < 16)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	t = time_iso8601_strptime(StringValueCStr(str));

	if (t == -1)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	time = rb_time_new(t, 0);
	rb_funcall(time, id_utc, 0);

	return time;
}

VALUE
Init_time_iso8601() {
	/* bring in time.rb and time.c */
	rb_require("time");

	/* Time class methods */
	rb_define_singleton_method(rb_cTime, "iso8601_strict",   rb_time_iso8601_strict, 1);
	rb_define_singleton_method(rb_cTime, "iso8601_strptime", rb_time_iso8601_strptime, 1);

	/* symbols */
	id_new = rb_intern("new");
	id_mktime = rb_intern("mktime");
	id_utc = rb_intern("utc");
	id_iso8601 = rb_intern("iso8601");
	id_iso8601_strict = rb_intern("iso8601_strict");

	return Qnil;
}

/* vim: set noexpandtab: */
