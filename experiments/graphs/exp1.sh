BLOCKSIZE=32
L2_SIZE=0
L2_ASSOC=0
REPL_POL=0
INCL_POL=0

for i in $(seq 10 20); do
    L1_SIZE=$((2 ** $i))
    for j in $(seq 0 3); do
        L1_ASSOC=$((2 ** $j))
        ./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L2_SIZE $L2_ASSOC $REPL_POL $INCL_POL experiments/gcc_trace.txt \
            > experiments/graphs/logs/exp1_${L1_SIZE}_${L1_ASSOC}.txt
    done
    L1_ASSOC=$(($L1_SIZE/$BLOCKSIZE)) 
    ./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L2_SIZE $L2_ASSOC $REPL_POL $INCL_POL experiments/gcc_trace.txt \
        > experiments/graphs/logs/exp1_${L1_SIZE}_${L1_ASSOC}.txt
done
