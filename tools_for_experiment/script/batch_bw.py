import os
from subprocess import call
import threading

def worker(s1, s2):
    call([s1, s2])

dirs = ["textured", "non-textured"]

bandwidth = 2000000

for d in dirs:
    for i in range(0, 6):
        t1 = threading.Thread(target = worker, args = ("./server", os.path.join("conf", d, "bw", str(bandwidth), "server")))
        t2 = threading.Thread(target = worker, args = ("./client", os.path.join("conf", d, "bw", str(bandwidth), "client")))
        t1.start()
        t2.start()
        t1.join()
        t2.join()
        call(["./warp_simulator", os.path.join("server_frames", d), os.path.join("results", d, "bw", str(bandwidth), "client_frames/"), "0"])
        call("rm " + os.path.join("results", d, "bw", str(bandwidth), "*", "*.rgb"), shell = True)
        call("rm " + os.path.join("results", d, "bw", str(bandwidth), "*", "*.mvp"), shell = True)
        call("rm " + os.path.join("results", d, "bw", str(bandwidth), "*", "*.depth"), shell = True)
        
        bandwidth = bandwidth * 2
        
