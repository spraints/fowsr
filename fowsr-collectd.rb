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
  "wind_dir" => "gauge",
}

Interval = 10
MaxAge = 600

reported_at = (Time.now - Interval).to_i
collected_at = Time.now - MaxAge
data = nil

fowsr = UNIXSocket.new(ARGV[0])

loop do
  # Collect new data when fowsr produces a new value.
  if IO.select([fowsr], nil, nil, 1)
    raw = fowsr.recv(10000)
    if raw.size > 0
      data = JSON.load(raw)
      collected_at = Time.now
    else
      # No data? weird. try restarting the connection.
      $stderr.puts "[fowsr] got 0 bytes, reconnecting to #{ARGV[0]}"
      fowsr.close
      fowsr = UNIXSocket.new(ARGV[0])
    end
  end
  # Stop reporting data that's really old.
  if Time.now - collected_at >= MaxAge
    data = nil
  end
  # Report something at least every 10s.
  if data
    now = Time.now.to_i
    if now - reported_at >= Interval
      data.each do |k,v|
        if type = Types[k]
          $stdout.puts "PUTVAL #{hostname}/weather-kinpicka2/#{type}-#{k} interval=#{Interval} #{now}:#{v}\n"
        end
      end
      $stdout.flush # Collectd's pipe doesn't auto-flush.
      reported_at = now
    end
  end
end
