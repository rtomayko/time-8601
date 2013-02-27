/*
 * Efficient Ruby Time::iso8601 implementation.
 *
 * https://github.com/rtomayko/time-iso8601
 */

#include <ruby.h>
#include <time.h>

static ID id_iso8601;         /* :iso8601 */
static ID id_iso8601_strict;  /* :iso8601_strict */
static ID id_at;              /* :at */
static ID id_new;             /* :new */
static ID id_utc;             /* :utc */
static ID id_localtime;       /* :localtime */
static ID id_mktime;          /* :mktime */

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
	year += 1900;
	mon += 1;

	return ((((unsigned long)
	            (year/4 - year/100 + year/400 + 367*mon/12 + time->tm_mday) +
	            year*365 - 719499
	                  )*24 + time->tm_hour   /* now have hours */
	                  )*60 + time->tm_min    /* now have minutes */
	                  )*60 + time->tm_sec;   /* finally seconds */
}

static int
time_iso8601_strzone(const char * pz, int len)
{
	int offset = 0;
	int sec;
	char * pe;

	if (pz[0] == 'Z')
		return 0;

	offset = strtol(pz, &pe, 10);
	if (*pe == ':') {
		offset *= 60 * 60;
		sec = strtol(pe + 1, &pe, 10) * 60;
		if (offset < 0) sec *= -1;
		offset += sec;
	}

	return offset;
}

static VALUE
time_iso8601_strptime(const char * str, int len)
{
	struct tm tm;
	char * pz;

	if ((pz = strptime(str, "%FT%T", &tm)))
	{
		time_t t = time_iso8601_mktime(&tm);
		VALUE time;

		/* need to parse zone info */
		if (pz - str < len) {
			int utc_offset = time_iso8601_strzone(pz, len - (pz - str));

			if (utc_offset == 0) {
				/* utc time */
				t -= utc_offset;
				time = rb_funcall(rb_cTime, id_at, 1, INT2FIX(t));

				struct time_object *tobj;
				GetTimeval(time, tobj);
				TIME_SET_UTC(tobj);
			} else {
				/* explicit zone */
				t -= utc_offset;
				time = rb_funcall(rb_cTime, id_at, 1, INT2FIX(t));
				rb_funcall(time, id_localtime, 1, INT2FIX(utc_offset));
			}
		} else {
			/* no zone, use local time */
			t += timezone;
			time = rb_funcall(rb_cTime, id_at, 1, INT2FIX(t));
		}

		return time;
	}
	else
	{
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);
	}
}

static VALUE
rb_time_iso8601_at(VALUE self, VALUE str)
{
	VALUE time;

	/* minumum possible ISO8601 strict time value */
	if (RSTRING_LEN(str) < 16)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	time = time_iso8601_strptime(StringValueCStr(str), RSTRING_LEN(str));

	/* rb_funcall(time, id_utc, 0); */

	return time;
}

VALUE
Init_time_iso8601() {
	tzset();

	/* bring in time.rb and time.c */
	rb_require("time");

	/* Time class methods */
	rb_define_singleton_method(rb_cTime, "iso8601_at", rb_time_iso8601_at, 1);

	/* symbols */
	id_at = rb_intern("at");
	id_new = rb_intern("new");
	id_mktime = rb_intern("mktime");
	id_utc = rb_intern("utc");
	id_localtime = rb_intern("localtime");
	id_iso8601 = rb_intern("iso8601");
	id_iso8601_strict = rb_intern("iso8601_strict");

	return Qnil;
}

/* vim: set noexpandtab: */
