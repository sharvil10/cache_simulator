BLOCKSIZE=32
L2_ASSOC=8
L1_SIZE=1024
L1_ASSOC=4
REPL_POL=0

for i in $(seq 11 16); do
    L2_SIZE=$((2 ** $i))
    for j in $(seq 0 1); do
        INCL_POL=$j
        ./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L2_SIZE $L2_ASSOC $REPL_POL $INCL_POL experiments/gcc_trace.txt \
            > experiments/graphs/logs/exp3_${L2_SIZE}_${INCL_POL}.txt
    done
done
