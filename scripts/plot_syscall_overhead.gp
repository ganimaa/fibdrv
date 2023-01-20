set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci execution time'
set term png enhanced font 'Verdana,10'
set output 'syscall_overhead.png'
plot [:][:]'out' using 1:2 with linespoints linewidth 2 title 'kernel space',\
              '' using 1:3 with linespoints linewidth 2 title 'user space',\
              '' using 1:4 with linespoints linewidth 2 title 'system call overhead'