task :default => :benchmark

desc "Run benchmarks"
task :benchmark do
  ruby "iso8601-bench.rb"
end

# Extension build ===========================================================

DLEXT = Config::CONFIG['DLEXT']

directory "lib"

desc "Build the extension"
task "time-iso8601" => [ "lib/time-iso8601.#{DLEXT}" ]

file "ext/Makefile" => FileList[ "ext/*.{c,h,rb}" ] do
  Dir.chdir("ext") { ruby "extconf.rb" }
end

file "ext/time-iso8601.#{DLEXT}" => FileList["ext/*.c", "ext/Makefile"] do |f|
  Dir.chdir("ext") { sh "make" }
end

file "lib/time-iso8601.#{DLEXT}" => [ "ext/time-iso8601.#{DLEXT}" ] do |t|
  cp "ext/time-iso8601.#{DLEXT}", t.name
end

desc "Compiles all extensions"
task :compile => [ "lib/time-iso8601.#{DLEXT}" ]
