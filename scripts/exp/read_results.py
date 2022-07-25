import os
from pprint import pprint
import sys
import os.path
import json
import re

import time
import datetime

node_id = 0
client_node_id = 1
client_thread_id = 1
results_dir = sys.argv[1]
data_type = sys.argv[2]

def cal_hit_rate():
    hits = []
    misses = []
    count = int(sys.argv[3]) * 6
    try:
        file = open("{}/server-node-{}-out".format(results_dir, node_id), 'r')
    except Exception as e:
        print(e)
        return False
    lines = file.readlines()
    if len(lines) == 0 or len(lines) <= 2:
		return False
    for i in range(len(lines)):
        line = lines[i]
        if "cache hits in 10s" in line:
            hits.append(int(line.split(",")[-2]))
            print(line)
        if "cache misses in 10s" in line:
            misses.append(int(line.split(",")[-2]))
            print(line)
    total_hits = sum(hits[count:])
    total_misses = sum(misses[count:])
    hit_rate = float(total_hits) / float(total_hits + total_misses)
    print(total_hits)
    print(total_misses)
    print(hit_rate)
    print(len(hits[count:]))
    print(len(misses[count:]))
    return True

def cal_cpu():
    peak = 0
    cpus = []
    count = int(sys.argv[3]) * 60
    tmp = count
    try:
        file = open("{}/node-{}-cpu.txt".format(results_dir, node_id), 'r')
    except Exception as e:
        print(e)
        return False
    lines = file.readlines()
    if len(lines) == 0 or len(lines) <= 2:
		return False
    for i in range(len(lines)):
        line = lines[i]
        if "all" in line:
            cpu = 100.0 - float(line.split(" ")[-1])
            cpus.append(cpu)
            if tmp == 0:
                if peak < cpu:
                    peak = cpu
            else:
                tmp -= 1
            print(line)
    avg_cpus = sum(cpus[count:]) / len(cpus[count:])
    print(len(cpus[count:]))
    print(avg_cpus)
    print(peak)

def cal_disk():
    disks = []
    count = int(sys.argv[3]) * 60
    try:
        file = open("{}/node-{}-disk.txt".format(results_dir, node_id), 'r')
    except:
        print(e)
        return False
    lines = file.readlines()
    if len(lines) == 0 or len(lines) <= 2:
		return False
    for i in range(len(lines)):
        line = lines[i]
        if "dev8-0" in line:
            disks.append(float(line.split(" ")[-1]))
            print(float(line.split(" ")[-1]))
            print(line)
    avg_disks = sum(disks[count:]) / len(disks[count:])
    print(len(disks[count:]))
    print(avg_disks)

def cal_net():
    nets_txkb = []
    nets_rxkb = []
    count = int(sys.argv[3]) * 60
    try:
        file = open("{}/node-{}-net.txt".format(results_dir, node_id), 'r')
    except:
        print(e)
        return False
    lines = file.readlines()
    if len(lines) == 0 or len(lines) <= 2:
		return False
    for i in range(len(lines)):
        line = lines[i]
        if "enp3s0f0" in line:
            pattern = r' +'
            nets_rxkb.append(float(re.split(pattern, line)[5]))
            nets_txkb.append(float(re.split(pattern, line)[6]))
            print(re.split(pattern, line)[5])
            print(re.split(pattern, line)[6])
            print(line)
    avg_nets_rxkb = sum(nets_rxkb[count:]) / len(nets_rxkb[count:])
    avg_nets_txkb = sum(nets_txkb[count:]) / len(nets_txkb[count:])
    print(len(nets_rxkb[count:]))
    print(len(nets_txkb[count:]))
    print(avg_nets_rxkb)
    print(avg_nets_txkb)
    
def cal_throughput():
    throughputs = []
    count = int(sys.argv[3]) * 60
    try:
        file = open("{}/client-node-{}-{}-out".format(results_dir, client_node_id, client_node_id), 'r')
    except Exception as e:
        print(e)
        return False
    lines = file.readlines()
    for i in range(len(lines)):
        line = lines[i]
        if "[READ: Count=" in line:
            if(count == 0):
                print(line.split(" ")[6])
                print(line)
                throughputs.append(float(line.split(" ")[6]))
            else:
                count -= 1
    avg_throughput = sum(throughputs[count:]) / len(throughputs[count:])
    print(avg_throughput)
    print(len(throughputs[count:]))
    print(len(throughputs))







if data_type == "cache_hit_rate":
    cal_hit_rate()
elif data_type == "cpu":
    cal_cpu()
elif data_type == "disk":
    cal_disk()
elif data_type == "net":
    cal_net()
elif data_type == "throughput":
    cal_throughput() 