set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci'
set term png enhanced font 'Verdana,10'
set key left top
set output 'bn_fib_performance.png'
plot [:][:]'bn_fib.log' using 1:2 with linespoints linewidth 2 title 'iterative v2',\
            '' using 1:3 with linespoints linewidth 2 title 'fast doubling v2',\
            'bn_fib_v1' using 1:2 with linespoints linewidth 2 title 'iterative v1',\
            '' using 1:3 with linespoints linewidth 2 title 'fast doubling v1'
