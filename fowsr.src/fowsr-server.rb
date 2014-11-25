require "socket"
require "logger"

#/ Usage: ruby fowsr-server.rb [--fowsr /path/to/fowsr] [--listen /path/to/sock]
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

# Avoid dumping stack traces when we exit in a controlled way.
def install_signal_handlers(server)
  [:INT, :TERM, :QUIT].each do |quit_signal|
    trap(quit_signal) { server.receive_quit_signal quit_signal }
  end
  [:HUP, :PIPE, :USR1, :USR2].each do |ignore_signal|
    trap(ignore_signal) { }
  end
end

# Start listening on the configured socket.
def listen(server)
  begin
    UNIXSocket.new(server.socket_addr)
  rescue Errno::ECONNREFUSED
    # The socket is left over from a previous run.
    File.unlink(server.socket_addr)
  end
  server.socket = UNIXServer.new(server.socket_addr)
  server.logger.info "Listening on #{File.expand_path(server.socket.path)}, fd = #{server.socket.to_i}"
end

# Wait for any IO activity.
# * read from fowsr
# * accept new clients
def do_io(server)
  handlers = {
    server.socket => lambda { accept_client(server) },
    server.fowsr_reader => lambda { consume_fowsr(server) },
    server.awake_reader => nil,
  }
  server.clients.each do |client|
    handlers[client] = lambda { check_client(server, client) }
  end

  server.logger.debug "Checking for input. #{server.clients.size} clients attached."
  rs, = IO.select(handlers.keys, nil, nil, 5)
  if rs
    rs.each do |read_fd|
      if handler = handlers[read_fd]
        handler.call
      end
    end
  end
end

# Try to accept a client connection to the socket.
def accept_client(server)
  client = server.socket.accept_nonblock
  server.logger.info "Accepted client on fd = #{client.to_i}"
  server.clients << client
rescue Object => e
  server.logger.error "Could not accept: #{e.class.name}: #{e}"
end

# Handle "readable" on client sockets. Also, discard any data they send us.
def check_client(server, client)
  client.read_nonblock(10000)
rescue EOFError
  server.logger.info "Client #{client.to_i} closed."
  server.clients.delete client
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

def kill_fowsr(server)
  # todo - tell fowsr to die
end

class ServerInfo
  def initialize(options)
    @options = options
  end

  attr_reader :options

  def logger
    @logger ||= Logger.new(STDERR)
  end

  # The address of the listening socket.
  def socket_addr
    options.fetch(:sock)
  end

  # The Socket (UNIXServer) where we listen for new connections.
  attr_accessor :socket

  # The Sockets of clients.
  def clients
    @clients ||= []
  end

  # The pair of IO for fowsr.
  def fowsr_pipe
    @fowsr_pipe ||= IO.pipe
  end
  def fowsr_reader ; fowsr_pipe[0] ; end
  def fowsr_writer ; fowsr_pipe[1] ; end

  # The pid of the most-recently spawned `fowsr` process.
  attr_accessor :fowsr_pid

  # Has a quit_signal been received?
  def received_quit_signal?
    @quit || false
  end

  # Time to exit.
  def receive_quit_signal(signal)
    awake_writer << "."
    @quit = true
  end

  # The pair of IO for awakening.
  def awake_pipe
    @awake_pipe ||= IO.pipe
  end
  def awake_reader ; awake_pipe[0] ; end
  def awake_writer ; awake_pipe[1] ; end
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
