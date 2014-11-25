#/ Usage: ruby fowsr-server.rb [--fowsr /path/to/fowsr] [--listen /path/to/sock]

require "socket"

def main(options)
  server = ServerInfo.new(options)

  install_signal_handlers server
  listen server

  loop do
    do_io server
    break if server.received_quit_signal?
    respawn_fowsr server
  end

  kill_fowsr server
end

def install_signal_handlers(server)
  [:INT, :TERM, :QUIT].each do |quit_signal|
    trap(quit_signal) { server.receive_quit_signal quit_signal }
  end
  [:HUP, :PIPE, :USR1, :USR2].each do |ignore_signal|
    trap(ignore_signal) { }
  end
end

def listen(server)
  begin
    UNIXSocket.new(server.socket_addr)
  rescue Errno::ECONNREFUSED
    # The socket is left over from a previous run.
    File.unlink(server.socket_addr)
  end
  server.socket = UNIXServer.new(server.socket_addr)
end

def do_io(server)
  sleep 3
  #raise "todo - select"
end

def respawn_fowsr(server)
#  raise "todo - wait on existing pid"
#  raise "spawn a new pid, if the old one has exited"

#  while ok
#  fowsr_path = options.fetch(:fowsr)
#  socket_path = options.fetch(:sock)
#
#  fowsr_io = IO.pipe
#  fowsr_pid = spawn fowsr_path, "-c", :out => fowsr_io[1]
end

class ServerInfo
  def initialize(options)
    @options = options
  end

  attr_reader :options

  # The address of the listening socket.
  def socket_addr
    options.fetch(:sock)
  end

  # The Socket (UNIXServer) where we listen for new connections.
  attr_accessor :socket

  # Has a quit_signal been received?
  def received_quit_signal?
    @quit || false
  end

  # Time to exit.
  def receive_quit_signal(signal)
    # todo - awake
    @quit = true
  end
end

def parse_args(argv)
  options = {
    :fowsr => File.expand_path("fowsr", File.dirname(__FILE__)),
    :sock  => "/var/run/fowsr.sock",
  }
  while argv.any?
    case argv.shift
    when "--fowsr"
      options[:fowsr] = argv.shift
    when "--listen"
      options[:sock] = argv.shift
    else
      puts File.read(__FILE__).lines.grep(/^#\//).map { |line| line[3..-1] }
      exit 1
    end
  end
  options.freeze
end

main(parse_args(ARGV))
