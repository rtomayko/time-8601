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

static VALUE T;               /* "T" */
static VALUE Z;               /* "Z" */

static VALUE
rb_time_iso8601_strict(int argc, VALUE * argv, VALUE self)
{
  return Qnil;
}

VALUE
Init_time_iso8601() {
    /* bring in time.rb and time.c */
    rb_require("time");

    /* Time class methods */
    rb_define_singleton_method(rb_cTime, "iso8601_strict", rb_time_iso8601_strict, -1);

    /* Token Strings */
    Z = rb_str_new2("Z");
    rb_gc_register_address(&Z);

    T = rb_str_new2("T");
    rb_gc_register_address(&T);

    /* Symbol Constants */
    id_new = rb_intern("new");
    id_iso8601 = rb_intern("iso8601");
    id_iso8601_strict = rb_intern("iso8601_strict");

    return Qnil;
}
