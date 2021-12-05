set terminal png truecolor size 800,600
set output "graph.png"
set title "Evolution de l'épidémie"
set xlabel "Nombres de tours"
set ylabel "Nombres de personnes"
plot "evolution.txt" using 1:2 with line title "Personnes en bonne santé", \
     "evolution.txt" using 1:3 with line title "Personnes malade", \
     "evolution.txt" using 1:4 with line title "Personnes décédées", \
     "evolution.txt" using 1:5 with line title "Cadavres brûlés" 