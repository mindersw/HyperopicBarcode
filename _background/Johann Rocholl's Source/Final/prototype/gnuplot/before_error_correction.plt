set terminal pdf size 4, 1.5
set output 'gnuplot/before_error_correction.pdf'
set lmargin 3
set rmargin 2
set key top left
set size ratio 0.33333333
set yrange [0.5573:1.0237]
set style line 1 lt rgb 'red' lw 3
set style line 2 lt rgb 'blue' lw 3
set style rect fc lt -1 fs solid 0.3 noborder
set obj rect from 93.500, graph 0 to 94.500, graph 1
set obj rect from 98.500, graph 0 to 99.500, graph 1
set obj rect from 104.500, graph 0 to 105.500, graph 1
set obj rect from 109.500, graph 0 to 110.500, graph 1
set obj rect from 147.500, graph 0 to 148.500, graph 1
set obj rect from 187.500, graph 0 to 188.500, graph 1
set obj rect from 229.500, graph 0 to 230.500, graph 1
set obj rect from 274.500, graph 0 to 275.500, graph 1
set obj rect from 321.500, graph 0 to 322.500, graph 1
set obj rect from 370.500, graph 0 to 371.500, graph 1
set obj rect from 377.500, graph 0 to 378.500, graph 1
set obj rect from 384.500, graph 0 to 385.500, graph 1
set obj rect from 392.500, graph 0 to 393.500, graph 1
set obj rect from 399.500, graph 0 to 400.500, graph 1
set obj rect from 407.500, graph 0 to 408.500, graph 1
set obj rect from 461.500, graph 0 to 462.500, graph 1
set obj rect from 518.500, graph 0 to 519.500, graph 1
set obj rect from 578.500, graph 0 to 579.500, graph 1
set obj rect from 643.500, graph 0 to 644.500, graph 1
set obj rect from 711.500, graph 0 to 712.500, graph 1
set obj rect from 784.500, graph 0 to 785.500, graph 1
set obj rect from 795.500, graph 0 to 796.500, graph 1
set obj rect from 806.500, graph 0 to 807.500, graph 1
set obj rect from 817.500, graph 0 to 818.500, graph 1
set obj rect from 1431655808.000, graph 0 to 1431655808.000, graph 1
plot \
'gnuplot/before_error_correction.dat' using 1:2 with lines ls 1 title 'input', \
'gnuplot/before_error_correction.dat' using 1:7 with lines ls 2 title 'simulated', \
NaN notitle
