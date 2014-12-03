#/ Usage: WUNDERGROUND_PASSWORD=password ruby fowsr-wunderground /var/run/fowsr.sock KINYOURID1
require "socket"
require "json"
require "open-uri"

hostname = Socket.gethostname.split(".").first
procline = $0

DynParams = {
  "time" => {:param => "dateutc", :fmt => lambda { |s| Time.at(s).utc.strftime("%Y-%m-%d+%H%%3A%M%%3A%S")}},
  "outdoor_rh" => {:param => "humidity"},
  "outdoor_f" => {:param => "tempf"},
  "pressure_inhg" => {:param => "baromin"},
  "wind_dir" => {:param => "winddir"},
}

DefaultParams = {
  "action" => "updateraw",
  "ID" => ARGV[1],
  "PASSWORD" => ENV["WUNDERGROUND_PASSWORD"],
  "softwaretype" => "fowsr+spraints",
}.freeze

fowsr = UNIXSocket.new(ARGV[0])
while line = fowsr.recv(10000)
  fowsr.close
  data = JSON.load(line)
  p data
  params = DefaultParams.dup
  data.each do |k,v|
    if spec = DynParams[k]
      params[spec[:param]] =
        if fmt = spec[:fmt]
          v = fmt.call(v)
        else
          v
        end
    end
  end
  encoded_params = params.map { |k,v| "#{k}=#{v}" }.join("&")
  url = "http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?#{encoded_params}"
  puts url
  $0 = "#{procline} -- #{url}"
  begin
    open(url) { |f| puts f.read }
  rescue Object => e
    $stderr.puts e.inspect
  end
  fowsr = UNIXSocket.new(ARGV[0])
end
