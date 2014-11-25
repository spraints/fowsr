require "socket"
require "json"
require "open-uri"

hostname = Socket.gethostname.split(".").first

DynParams = {
  "time" => {:param => "dateutc", :fmt => lambda { |s| Time.at(s).utc.strftime("%Y-%m-%d+%H%%3A%M%%3A%S")}},
  "outdoor_rh" => {:param => "humidity"},
  "outdoor_f" => {:param => "tempf"},
  "pressure_inhg" => {:param => "baromin"},
}

DefaultParams = {
  "action" => "updateraw",
  "ID" => "KINPICKA2",
  "PASSWORD" => ENV["WUNDERGROUND_PASSWORD"],
  "softwaretype" => "fowsr+spraints",
}.freeze

fowsr = UNIXSocket.new(ARGV[0])
while line = fowsr.recv(10000)
  data = JSON.load(line)
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
  open(url) { |f| f.read }
end
