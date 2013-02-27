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

static int
_strzone(const char * pz, int * utc_offset)
{
	int sec;
	const char * pe;
	int mul;
	int offset = 0;

	if (pz[0] == 'Z') {
		*utc_offset = 0;
		return 1;
	}

	if (*pz == '+')
	{
		mul = 1;
		pz++;
	}
	else if (*pz == '-')
	{
		mul = -1;
		pz++;
	}

	offset = _strtol(pz, &pe);
	if (pz == pe)
		return 0;

	if (*pe == ':')
	{
		offset *= 60 * 60;
		sec = _strtol(pe + 1, &pe) * 60;
		offset += sec;
	}

	offset *= mul;
	*utc_offset = offset;

	return 1;
}

static VALUE
_strtime(const char * str, struct tm * tdata, int * utc_offset)
{
	const char * ps = str;
	const char * pe;

	tdata->tm_isdst = -1;

	tdata->tm_year = _strtol(ps, &pe) - 1900;
	if (pe == ps || *pe != '-')
		return 0;

	ps = pe + 1;
	tdata->tm_mon = _strtol(ps, &pe) - 1;
	if (pe == ps || *pe != '-' || tdata->tm_mon < 1 || tdata->tm_mon > 12)
		return 0;

	ps = pe + 1;
	tdata->tm_mday = _strtol(ps, &pe);
	if (pe == ps || *pe != 'T' || tdata->tm_mday < 1 || tdata->tm_mday > 31)
		return 0;

	ps = pe + 1;
	tdata->tm_hour = _strtol(ps, &pe);
	if (pe == ps || *pe != ':' || tdata->tm_hour < 0 || tdata->tm_hour > 24)
		return 0;

	ps = pe + 1;
	tdata->tm_min = _strtol(ps, &pe);
	if (pe == ps || *pe != ':' || tdata->tm_min < 0 || tdata->tm_min > 61)
		return 0;

	ps = pe + 1;
	tdata->tm_sec = _strtol(ps, &pe);
	if (pe == ps || tdata->tm_sec < 0 || tdata->tm_sec > 61)
		return 0;

	ps = pe;
	if (*ps == '\0') {
		/* no zone, use local time */
		*utc_offset = timezone;
		return 1;
	}
	else
	{
		/* need to parse zone info */
		return _strzone(ps, utc_offset);
	}
}

static VALUE
time_iso8601_parse(const char * str)
{
	VALUE time;
	time_t utc_time;
	int utc_offset;
	struct tm tdata;
	memset(&tdata, 0x0, sizeof(struct tm));

	if (_strtime(str, &tdata, &utc_offset))
	{
		utc_time = timegm(&tdata);
		if (utc_offset == 0) {
			time = rb_time_new(utc_time, 0);
			rb_funcall(time, id_utc, 0);
		}
		else if (utc_offset == timezone)
		{
			utc_time += timezone;
			time = rb_time_new(utc_time, 0);
		}
		else
		{
			utc_time -= utc_offset;
			time = rb_time_new(utc_time, 0);
			rb_funcall(time, id_localtime, 1, INT2FIX(utc_offset));
		}
	}
	else
	{
		rb_raise(rb_eArgError, "invalid date: %s", (void*)str);
	}

	return time;
}

static VALUE
rb_time_iso8601_at(VALUE self, VALUE str)
{
	VALUE time;

	Check_Type(str, T_STRING);

	/* minumum possible ISO8601 strict time value */
	if (RSTRING_LEN(str) < 16)
		rb_raise(rb_eArgError, "invalid date: %s (too short)", (void*)StringValueCStr(str));

	time = time_iso8601_parse(StringValueCStr(str));

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
