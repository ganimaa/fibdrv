set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci'
set term png enhanced font 'Verdana,10'
set output 'fib-fast.png'
plot [:][:]'fib-fast.log' using 1:2 with linespoints linewidth 2 title 'fast w/o clz',\
            '' using 1:3 with linespoints linewidth 2 title 'fast w/ clz'
