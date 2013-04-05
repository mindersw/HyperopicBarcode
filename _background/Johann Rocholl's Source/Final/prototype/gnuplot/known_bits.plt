set terminal pdf size 4, 1.5
set output 'gnuplot/known_bits.pdf'
set lmargin 3
set rmargin 2
set key off
set size ratio 0.33333333
set yrange [0.6579:0.9382]
set style line 1 lt rgb 'red' lw 3
set style line 2 lt rgb 'blue' lw 3
set style rect fc lt -1 fs solid 0.3 noborder
set obj rect from 89.500, graph 0 to 90.500, graph 1
set obj rect from 94.500, graph 0 to 95.500, graph 1
set obj rect from 100.500, graph 0 to 101.500, graph 1
set obj rect from 105.500, graph 0 to 106.500, graph 1
set obj rect from 143.500, graph 0 to 144.500, graph 1
set obj rect from 183.500, graph 0 to 184.500, graph 1
set obj rect from 225.500, graph 0 to 226.500, graph 1
set obj rect from 270.500, graph 0 to 271.500, graph 1
set obj rect from 316.500, graph 0 to 317.500, graph 1
set obj rect from 365.500, graph 0 to 366.500, graph 1
set obj rect from 373.500, graph 0 to 374.500, graph 1
set obj rect from 380.500, graph 0 to 381.500, graph 1
set obj rect from 387.500, graph 0 to 388.500, graph 1
set obj rect from 395.500, graph 0 to 396.500, graph 1
set obj rect from 402.500, graph 0 to 403.500, graph 1
set obj rect from 456.500, graph 0 to 457.500, graph 1
set obj rect from 513.500, graph 0 to 514.500, graph 1
set obj rect from 574.500, graph 0 to 575.500, graph 1
set obj rect from 638.500, graph 0 to 639.500, graph 1
set obj rect from 706.500, graph 0 to 707.500, graph 1
set obj rect from 779.500, graph 0 to 780.500, graph 1
set obj rect from 790.500, graph 0 to 791.500, graph 1
set obj rect from 801.500, graph 0 to 802.500, graph 1
set obj rect from 812.500, graph 0 to 813.500, graph 1
set obj rect from -1431655808.000, graph 0 to -1431655808.000, graph 1
plot \
'gnuplot/known_bits.dat' using 1:6 with lines ls 2 title 'guess', \
NaN notitle
