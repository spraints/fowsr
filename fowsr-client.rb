require "socket"
require "logger"

logger = Logger.new(STDOUT)
sock = UNIXSocket.new(ARGV[0])
logger.info "Connected to #{sock.path}."

while IO.select([sock])
  puts sock.read_nonblock(1000)
end
