set xrange [0:1850]
set yrange [0:600]
plot "plot.dat" using 1:2 with lines
pause 1
reread
