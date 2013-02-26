require 'mkmf'

# Disable warnings from ld
$LDFLAGS = "-w"
# turn on warnings from gcc
$CFLAGS = "-pedantic -Wall -Wno-long-long -Winline"

dir_config 'time-iso8601'
create_makefile 'time_iso8601'
