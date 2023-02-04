set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci'
set term png enhanced font 'Verdana,10'
set key left top
set output 'bn_fib_performance.png'
plot [:][:]'bn_fib_v5' using 1:2 with lines linewidth 2 title 'fast doubling v5',\
            'bn_fib_v3' using 1:2 with lines linewidth 2 title 'fast doubling v4'
