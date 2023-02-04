set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci'
set term png enhanced font 'Verdana,10'
set key left top
set output 'bn_fib_performance.png'
plot [:][:]'bn_fib.log' using 1:3 with linespoints linewidth 2 title 'fast doubling v4',\
            'bn_fib_v3' using 1:3 with linespoints linewidth 2 title 'fast doubling v3'
