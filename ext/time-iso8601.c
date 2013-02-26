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

static VALUE
time_iso8601_strict(char * str, int len)
{
	VALUE time = Qnil;
	char * ps = str;
	char * pe;
	int year, mon, day, hour, min, sec;

	year = strtol(ps, &pe, 10);
	if (pe > ps && *pe == '-')
	{
		ps = pe + 1;
		mon = strtol(ps, &pe, 10);
		if (pe > ps && *pe == '-' && mon >= 1 && mon <= 12)
		{
			ps = pe + 1;
			day = strtol(ps, &pe, 10);
			if (pe > ps && *pe == 'T' && day >= 1 && day <= 31)
			{
				ps = pe + 1;
				hour = strtol(ps, &pe, 10);
				if (pe > ps && *pe == ':' && hour >= 0 && hour <= 24)
				{
					ps = pe + 1;
					min = strtol(ps, &pe, 10);
					if (pe > ps && *pe == ':' && min >= 0 && min <= 61)
					{
						ps = pe + 1;
						sec = strtol(ps, &pe, 10);
						if (pe > ps && sec >= 0 && sec <= 61)
						{
							if (pe >= (str + len))
							{
								/* no timezone, use local time */
								time = rb_funcall(rb_cTime, id_mktime, 6,
								                  INT2FIX(year), INT2FIX(mon), INT2FIX(day),
								                  INT2FIX(hour), INT2FIX(min), INT2FIX(sec));
							}
							else
							{
								ps = pe;
								switch(*ps)
								{
									case 'Z': /* utc timezone */
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

						}
					}
				}
			}
		}
	}

	return time;
}

static VALUE
rb_time_iso8601_strict(VALUE self, VALUE str)
{
	VALUE time;

	/* minumum possible ISO8601 strict time value */
	if (RSTRING_LEN(str) < 16)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	time = time_iso8601_strict(StringValueCStr(str), RSTRING_LEN(str));

	if (NIL_P(time))
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	return time;
}

static time_t
time_iso8601_strptime(const char * str, int len)
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

	/* minumum possible ISO8601 strict time value */
	if (RSTRING_LEN(str) < 16)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	t = time_iso8601_strptime(StringValueCStr(str), RSTRING_LEN(str));

	if (t == -1)
		rb_raise(rb_eArgError, "invalid date: %p", (void*)str);

	time = rb_funcall(rb_cTime, id_at, 1, INT2FIX((int)t));
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
	id_at = rb_intern("at");
	id_new = rb_intern("new");
	id_mktime = rb_intern("mktime");
	id_utc = rb_intern("utc");
	id_iso8601 = rb_intern("iso8601");
	id_iso8601_strict = rb_intern("iso8601_strict");

	return Qnil;
}

/* vim: set noexpandtab: */
