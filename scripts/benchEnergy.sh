#!/usr/bin/env bash

set -e
set -x

declare -a CASES=(imstat imstat_fread)

host=$1
image=$2
mkdir -p stat/$host/energy

AWK=awk

function pattern() {
  id=$1
  label=$2
  PATTERN="
  /energy-pkg/ {
    energy = \$1;
  }
  /seconds time elapsed/ {
    time = \$1;
    printf(\"%s %s %s %s\n\", $id, \"$label\", energy, time);
  }
  "
  echo $PATTERN
}

i=0

rm -f stat/$host/energy/${image}-stats.csv
for case in ${CASES[@]}
do  
      rm -f stat/$host/energy/${image}-${case}-result.txt
      
      echo "Running ${case}" >> stat/$host/energy/${image}-${case}-result.txt 

      perf stat -e power/energy-pkg/ ./build/${case} ${image} &>> stat/$host/energy/${image}-${case}-result.txt 

      PATTERN=`pattern $i $case`
      ((i+=1))
      cat stat/$host/energy/${image}-${case}-result.txt | $AWK "$PATTERN" >> stat/$host/energy/${image}-stats.csv
done


case=imstat_fread_le
rm -f stat/$host/energy/${image}-${case}-result.txt

echo "Running ${case}" >> stat/$host/energy/${image}-${case}-result.txt 

perf stat -e power/energy-pkg/ ./build/${case} ${image}.le &>> stat/$host/energy/${image}-${case}-result.txt 

PATTERN=`pattern $i $case`

cat stat/$host/energy/${image}-${case}-result.txt | $AWK "$PATTERN" >> stat/$host/energy/${image}-stats.csv


  echo "                                            \
    reset;                                          \
    set terminal png enhanced large; \
                                                          \
    set title \"$bench Benchmark\";                        \
    set xlabel \"Matrix Dim\";                             \
    set ylabel \"Joules\";                     \
    set yrange [0:500];                 \
    unset key;                                      \
    set boxwidth 0.5;                                       \
    set style fill solid;                                \
                                                          \
    plot \"stat/$host/energy/${image}-stats.csv\" using 1:3:xtic(2) with boxes; \
" | gnuplot > stat/$host/energy/${image}-performance.png
