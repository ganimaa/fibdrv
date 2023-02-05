set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci'
set term png enhanced font 'Verdana,10'
set output 'bn_fib_kernel-v1.png'
plot [:][:]'fib-fast.log' using 1:2 with linespoints linewidth 3 title 'fast doubling',\
            '' using 1:3 with linespoints linewidth 3 title 'iterative'
