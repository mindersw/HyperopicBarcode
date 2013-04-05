set terminal pdf size 4, 1.5
set output 'gnuplot/robust_sampling.pdf'
set lmargin 3
set rmargin 2
set key top left
set size ratio 0.33333333
set yrange [0.6351:0.9283]
set style line 1 lt rgb 'red' lw 3
set style line 2 lt rgb 'blue' lw 3
plot \
'gnuplot/robust_sampling.dat' using 1:2 with lines ls 1 title 'input', \
'gnuplot/robust_sampling.dat' using 1:6 with lines ls 2 title 'guess', \
NaN notitle
