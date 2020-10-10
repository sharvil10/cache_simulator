import os
import glob
import pandas as pd
import numpy as np
from cycler import cycler
import  matplotlib.pyplot as plt

repl_map = {0: 'LRU',
        1: 'PLRU',
        2: 'Optimal'}

incl_map = {0: 'non-inclusive',
        1: 'inclusive'}


def get_stats(size, associativity, cacti_table):
    if(associativity == size//32):
        associativity = ' FA'
    tmp = cacti_table[cacti_table.associativity.eq(associativity) & 
            cacti_table.tsize.eq(size)]
    return tmp.iloc[0]['acc_time'], tmp.iloc[0]['en_per_acc'], tmp.iloc[0]['area']

def parse_res(file_path):
    lines = []
    with open(file_path , "r") as fl:
        lines = fl.readlines()

    idx = 0
    for i in lines:
        idx +=1
        if "results" in i:
            break
    
    l1_mr = float(lines[idx + 4].split(":")[-1].strip())
    l2_mr = float(lines[idx + 10].split(":")[-1].strip())
    mem_traffic = int(lines[-1].split(":")[-1].strip())

    return l1_mr, l2_mr, mem_traffic
    

def calc_aat(ht_l1, mr_l1, ht_l2, mr_l2):
    if mr_l2 == 0:
        return ht_l1 + (mr_l1 * 100.)
    else:
        return ht_l1 + mr_l1 * (ht_l2 + mr_l2 * 100.)

if not os.path.isdir('plots'):
    os.makedirs('plots')

cacti_table = pd.read_excel('cacti_table.xls')
cacti_table.columns = ["tsize", "size_kb", "block_size", "associativity", "acc_time", "en_per_acc", "area"]

cacti_table = cacti_table[cacti_table['block_size'] == 32]
block_size = 32

g1_res = []
g2_res = []
g3_res = []
g4_res = []

for exp in range(1, 4):
    logs = glob.glob("logs/exp%d_*"%exp)
    l1_size = 0
    l1_assoc = 0
    l2_size = 0
    l2_assoc = 0
    repl_pol = 0
    incl_pol = 0
    acc_time_l2 = 0
    en_per_acc_l2 = 0
    area_l2 = 0
    for l in logs:
        fname = os.path.basename(os.path.splitext(l)[0])
        p1 = int(fname.split("_")[1])
        p2 = int(fname.split("_")[2])
        if(exp == 1):
            l1_size = p1
            l1_assoc = p2
        elif(exp == 2):
            l1_size = p1
            l1_assoc = 4
            repl_pol = p2
        else:
            l1_size = 1024
            l1_assoc = 4
            l2_size = p1
            l2_assoc = 8
            incl_pol = p2

        acc_time_l1, en_per_acc_l1, area_l1 = get_stats(l1_size, l1_assoc, cacti_table)
        if l2_size > 0:
            acc_time_l2, en_per_acc_l2, area_l2 = get_stats(l2_size, l2_assoc, cacti_table)
        
        mr_l1, mr_l2, mem_traffic = parse_res(l)
        AAT = calc_aat(acc_time_l1, mr_l1, acc_time_l2, mr_l2)
        if exp == 1:
            if p2 == (l1_size // 32):
                p2 = 0
            g1_res.append([p1, mr_l1, p2])
            g2_res.append([p1, AAT, p2])
        elif exp == 2:
            g3_res.append([p1, AAT, p2])
        else:
            print(p1)
            g4_res.append([p1, AAT, p2])
            

#print(g2_res)
print(g4_res)

plt.rc('axes', prop_cycle=cycler('color', ['r', 'g', 'b', 'y']))

#Graph-1 plot
g1_res = np.array(g1_res)
p2s = np.unique(g1_res[:, 2])
plt.figure(1)
for i in p2s:
    curr = g1_res[g1_res[:, 2] == i]
    x_idx = np.argsort(curr[:, 0])
    x = curr[x_idx, 0] // 1024
    y = curr[x_idx, 1]
    lab = 'Full Assoc' if i == 0 else str(i)
    plt.plot(x, y, label = lab, marker = 'x')
plt.legend()
plt.xlabel('L1 Cache Size(in KB)')
plt.ylabel('L1 Miss Rate')
plt.title("L1 miss rate for various L1 cache sizes per associativity")
plt.savefig()

#Graph-2 plot
g2_res = np.array(g2_res)
p2s = np.unique(g2_res[:, 2])
plt.figure(2)
for i in p2s:
    curr = g2_res[g2_res[:, 2] == i]
    x_idx = np.argsort(curr[:, 0])
    x = curr[x_idx, 0] // 1024
    y = curr[x_idx, 1]
    lab = 'Full Assoc' if i == 0 else str(i)
    plt.plot(x, y, label = lab, marker = 'o')
plt.legend()
plt.xlabel('L1 Cache Size(in KB)')
plt.ylabel('AAT')
plt.title("AAT for various L1 cache sizes per associativity")
plt.savefig()


#Graph-3 plot
g3_res = np.array(g3_res)
p2s = np.unique(g3_res[:, 2])
plt.figure(3)
for i in p2s:
    curr = g3_res[g3_res[:, 2] == i]
    x_idx = np.argsort(curr[:, 0])
    x = curr[x_idx, 0] // 1024
    y = curr[x_idx, 1]
    lab = repl_map[i] 
    plt.plot(x, y, label = lab, marker = 'v')
plt.legend()
plt.xlabel('L1 Cache Size(in KB)')
plt.ylabel('AAT')
plt.title("AAT for various L1 cache sizes and replacement policies")
plt.savefig()

#Graph-4 plot
g4_res = np.array(g4_res)
p2s = np.unique(g4_res[:, 2])
plt.figure(4)
for i in p2s:
    curr = g4_res[g4_res[:, 2] == i]
    x_idx = np.argsort(curr[:, 0])
    x = curr[x_idx, 0] // 1024
    y = curr[x_idx, 1]
    lab = incl_map[i] 
    plt.plot(x, y, label = lab, marker = 's')
plt.legend()
plt.xlabel('L2 Cache Size(in KB)')
plt.ylabel('AAT')
plt.title("AAT for various L2 cache sizes and inclusion policies")
plt.savefig()

plt.show()
while True:
    a = 0
