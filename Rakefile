require 'rake/clean'

task :default => [:compile, :test]

desc "Run benchmarks"
task :benchmark do
  ruby "iso8601-bench.rb"
end

# Extension build ===========================================================

DLEXT = RbConfig::CONFIG['DLEXT']
CLEAN.include ["ext/Makefile", "ext/*.{o,bundle,so}", "lib/*.{bundle,so,dll}" ]

desc "Compiles all extensions"
task :compile => [ "lib/time_iso8601.#{DLEXT}" ]

directory "lib"

desc "Build the extension"
task "time_iso8601" => [ "lib/time_iso8601.#{DLEXT}" ]

file "ext/Makefile" => FileList[ "ext/*.{c,h,rb}" ] do
  Dir.chdir("ext") { ruby "extconf.rb" }
end

file "ext/time_iso8601.#{DLEXT}" => FileList["ext/*.c", "ext/Makefile"] do |f|
  Dir.chdir("ext") { sh "make" }
end

file "lib/time_iso8601.#{DLEXT}" => [ "ext/time_iso8601.#{DLEXT}" ] do |t|
  cp "ext/time_iso8601.#{DLEXT}", t.name
end

# Tests =====================================================================

task :test => [:compile] do
  ruby "-I", "lib", "-r", "time-iso8601", "-e", "0"
end
