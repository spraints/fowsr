require "socket"
require "json"

hostname = Socket.gethostname.split(".").first

Types = {
  "outdoor_c" => "temperature",
  "outdoor_f" => "temperature",
  "outdoor_rh" => "percent",
  "indoor_c" => "temperature",
  "indoor_f" => "temperature",
  "indoor_rh" => "percent",
  "pressure_mbar" => "gauge",
  "pressure_inhg" => "gauge",
}

fowsr = UNIXSocket.new(ARGV[0])
while line = fowsr.recv(10000)
  data = JSON.load(line)
  data.each do |k,v|
    if type = Types[k]
      puts "PUTVAL #{hostname}/weather-kinpicka2/#{type}-#{k} #{data["time"]}:#{v}\n"
    end
  end
end
