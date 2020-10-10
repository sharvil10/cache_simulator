BLOCKSIZE=32
L2_SIZE=0
L2_ASSOC=0
L1_ASSOC=4
INCL_POL=0

for i in $(seq 10 18); do
    L1_SIZE=$((2 ** $i))
    for j in $(seq 0 2); do
        REPL_POL=$j
        ./sim_cache $BLOCKSIZE $L1_SIZE $L1_ASSOC $L2_SIZE $L2_ASSOC $REPL_POL $INCL_POL experiments/gcc_trace.txt \
            > experiments/graphs/logs/exp2_${L1_SIZE}_${REPL_POL}.txt
    done
done
