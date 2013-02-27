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
static ID id_at;              /* :at */
static ID id_new;             /* :new */
static ID id_utc;             /* :utc */
static ID id_mktime;          /* :mktime */

/*
static VALUE time_iso8601_mktime(tm * time)
{
}
else
{
	ps = pe;
	switch(*ps)
	{
		case 'Z':
			time = rb_funcall(rb_cTime, id_utc, 6,
			                  INT2FIX(year), INT2FIX(mon), INT2FIX(day),
			                  INT2FIX(hour), INT2FIX(min), INT2FIX(sec));
			break;

		case '+':
		case '-':
			time = rb_funcall(rb_cTime, id_new, 7,
			                  INT2FIX(year), INT2FIX(mon), INT2FIX(day),
			                  INT2FIX(hour), INT2FIX(min), INT2FIX(sec),
			                  rb_str_new(ps, len - (ps - str)));
			break;

		default:
			return Qnil;
	}
}
*/

/* This is the Linux+v3.8 mktime. Does not account for leap seconds.
 *
 * http://lxr.linux.no/linux+v3.8/kernel/time.c#L313
 *
 * Copyright Linux people.
 */
time_t
time_iso8601_mktime(struct tm * time)
{
	unsigned int mon = time->tm_mon, year = time->tm_year;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;      /* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
	            (year/4 - year/100 + year/400 + 367*mon/12 + time->tm_mday) +
	            year*365 - 719499
	                  )*24 + time->tm_hour   /* now have hours */
	                  )*60 + time->tm_min    /* now have minutes */
	                  )*60 + time->tm_sec;   /* finally seconds */
}

static VALUE
time_iso8601_strptime(const char * str, int len)
{
	struct tm time;
	char * pe;

	if ((pe = strptime(str, "%FT%T", &time)))
	{
		/* need to parse zone info */
		if (pe - str < len)

		time.tm_isdst = -1;
		return time_iso8601_mktime(&time);
	}
	else
	{
		return -1;
	}
}

static VALUE
rb_time_iso8601_at(VALUE self, VALUE str)
{
	VALUE time;
	time_t t;

	/* minumum possible ISO8601 strict time value */
	if (RSTRING_LEN(str) < 16)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	t = time_iso8601_strptime(StringValueCStr(str), RSTRING_LEN(str));

	if (t == -1)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	time = rb_funcall(rb_cTime, id_at, 1, INT2FIX((int)t));
	/* rb_funcall(time, id_utc, 0); */

	return Qnil;
}

VALUE
Init_time_iso8601() {
	/* bring in time.rb and time.c */
	rb_require("time");

	/* Time class methods */
	rb_define_singleton_method(rb_cTime, "iso8601_at", rb_time_iso8601_at, 1);

	/* symbols */
	id_at = rb_intern("at");
	id_new = rb_intern("new");
	id_mktime = rb_intern("mktime");
	id_utc = rb_intern("utc");
	id_iso8601 = rb_intern("iso8601");
	id_iso8601_strict = rb_intern("iso8601_strict");

	return Qnil;
}

/* vim: set noexpandtab: */
