./sim_cache 16 1024 2 0 0 0 0 ../MachineProblem1_Fall2020/traces/gcc_trace.txt > validation0.txt
./sim_cache 16 1024 1 0 0 0 0 ../MachineProblem1_Fall2020/traces/perl_trace.txt > validation1.txt
./sim_cache 16 1024 2 0 0 1 0 ../MachineProblem1_Fall2020/traces/gcc_trace.txt > validation2.txt
./sim_cache 16 1024 2 0 0 2 0 ../MachineProblem1_Fall2020/traces/vortex_trace.txt > validation3.txt
./sim_cache 16 1024 2 8192 4 0 0 ../MachineProblem1_Fall2020/traces/gcc_trace.txt > validation4.txt
./sim_cache 16 1024 1 8192 4 0 0 ../MachineProblem1_Fall2020/traces/go_trace.txt > validation5.txt
./sim_cache 16 1024 2 8192 4 0 1 ../MachineProblem1_Fall2020/traces/gcc_trace.txt > validation6.txt
./sim_cache 16 1024 1 8192 4 0 1 ../MachineProblem1_Fall2020/traces/compress_trace.txt > validation7.txt

for i in $(seq 0 7); do
    diff -i -w ../MachineProblem1_Fall2020/validation_runs/validation$i.txt validation$i.txt
    echo "No diff in validation$i.txt"
done
