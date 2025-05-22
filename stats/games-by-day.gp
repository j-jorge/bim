set terminal pngcairo size 1280, 720 enhanced font "Sans,16"
set output "games-by-day.png"

set style data histogram
set style histogram cluster gap 1
set style fill solid
set boxwidth 0.9
set xtics format ""
set grid ytics
set xtics rotate
set xtics nomirror
set ytics nomirror

set boxwidth 1.5
set key off

plot "games-by-day-hour.data" \
     using 2:xtic((int($1) == 1 || int($1) % 10 == 0)?stringcolumn(1):"") \
     linecolor rgb "#87CEEB"

