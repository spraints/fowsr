require "socket"
require "logger"
require "json"

#/ Usage: ruby fowsr-server.rb [--fowsr /path/to/fowsr] [--listen /path/to/sock]
def main(options)
  server = ServerInfo.new(options)

  install_signal_handlers server
  listen server
  respawn_fowsr server

  loop do
    do_io server
    break if server.received_quit_signal?
    respawn_fowsr server
  end

  kill_fowsr server
end

# Avoid dumping stack traces when we exit in a controlled way.
def install_signal_handlers(server)
  trap(:CHLD) { server.awake }
  [:INT, :TERM, :QUIT].each do |quit_signal|
    trap(quit_signal) { server.receive_quit_signal quit_signal }
  end
  [:HUP, :PIPE, :USR1, :USR2].each do |ignore_signal|
    trap(ignore_signal) { }
  end
end

# Start listening on the configured socket.
def listen(server)
  if File.exist?(server.socket_addr) && File.socket?(server.socket_addr)
    begin
      UNIXSocket.new(server.socket_addr)
    rescue Errno::ECONNREFUSED
      # The socket is left over from a previous run.
      File.unlink(server.socket_addr)
    end
  end
  server.socket = UNIXServer.new(server.socket_addr)
  File.chmod(0777, server.socket_addr)
  server.logger.info "Listening on #{File.expand_path(server.socket.path)}, fd = #{server.socket.to_i}"
end

# Wait for any IO activity.
# * read from fowsr
# * accept new clients
def do_io(server)
  handlers = {
    server.socket => lambda { accept_client(server) },
    server.fowsr_reader => lambda { consume_fowsr(server) },
    server.awake_reader => lambda { server.awake_reader.read_nonblock(10000) },
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

# Interpret the output of fowsr and send it along to all clients.
def consume_fowsr(server)
  data = server.fowsr_reader.read_nonblock(10000)
  server.logger.info "Got #{data.size} bytes from fowsr."
  if data = convert(server, data)
    server.clients.each do |client|
      client.write(data)
    end
  end
end

# Given fowsr data, build the format required for clients.
#
# Conditions:
#   Outdoor temp: 33F
#   Outdoor humidity: 72%
#   Indoor temp: 61F
#   Indoor humidity: 52%
#   pressure: 28inHg
#
# Input:
#   DTime 25-11-2014 00:03:00
#   ETime 1416898363
#   RHi 52.0
#   Ti 16.3
#   RHo 72.0
#   To 0.7
#   RP 1004.3
#   WS 0.0
#   WG 0.0
#   DIR 270.0
#   Rtot 0.3
#   state 00
#
# Output: (note this only includes data that's valid on my weather station)
#   {"time": 1416898363, "indoor_rh": 52, "indoor_c": 16.3, "indoor_f": 61.2, "outdoor_rh": 72, "outdoor_c": 0.7, "outdoor_f": 33.3, "pressure_mbar": 1004.3, "pressure_inhg": 29.656}
def convert(server, data)
  converted = {}

  data.lines.each do |line|
    case line
    when /^ETime (\d+)/
      converted["time"] = $1.to_i
    when /^RH([io]) ([0-9.]+)/
      converted["#{Location.fetch($1)}_rh"] = $2.to_f
    when /^T([io]) (-?[0-9.]+)/
      loc = Location.fetch($1)
      converted["#{loc}_c"] = c = $2.to_f
      converted["#{loc}_f"] = f = (c * 9 / 5) + 32
    when /^RP ([0-9.]+)/
      converted["pressure_mbar"] = mbar = $1.to_f
      converted["pressure_inhg"] = inhg = mbar * 0.0295299833
    when /^DIR ([0-9]+)/
      converted["wind_dir"] = $1.to_i
    end
  end

  if converted.any?
    JSON.dump(converted)
  else
    nil
  end
end

Location = {"i" => "indoor", "o" => "outdoor"}

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
  client.close
end

def respawn_fowsr(server)
  if server.fowsr_pid
    if Process.waitpid(server.fowsr_pid, Process::WNOHANG)
      server.logger.info "fowsr[#{server.fowsr_pid}] #{$?}"
      server.fowsr_pid = nil
      server.next_start = Time.now + 15
    end
  end

  if server.fowsr_pid.nil? && server.next_start < Time.now
    server.fowsr_pid = spawn(server.fowsr_path, "-c", :out => server.fowsr_writer)
    server.logger.info "fowsr[#{server.fowsr_pid}] spawned."
  end
end

def kill_fowsr(server)
  if server.fowsr_pid
    Process.kill :QUIT, server.fowsr_pid
  end
end

class ServerInfo
  def initialize(options)
    @options = options
    @next_start = Time.now - 1
  end

  attr_reader :options
  # The path to the fowsr executable.
  def fowsr_path ; options.fetch(:fowsr) ; end
  # The address of the listening socket.
  def socket_addr ; options.fetch(:sock) ; end

  def logger
    @logger ||= Logger.new(STDERR)
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

  # The time to start another `fowsr` process.
  attr_accessor :next_start

  # Has a quit_signal been received?
  def received_quit_signal?
    @quit || false
  end

  # Time to exit.
  def receive_quit_signal(signal)
    @quit = true
    awake
  end

  # Time to pop out of IO.select.
  def awake
    awake_writer << "."
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
