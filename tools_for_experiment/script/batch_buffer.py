import os
from subprocess import call
import threading

def worker(s1, s2):
    call([s1, s2])

dirs = ["textured", "non-textured"]
buffer_sizes = [10]
lods = [40, 80]
delays = [5, 10, 15]

for d in dirs:
    for i in lods:
        t1 = threading.Thread(target = worker, args = ("./server", os.path.join("conf", d, str(i), "server")))
        t2 = threading.Thread(target = worker, args = ("./client", os.path.join("conf", d, str(i), "client")))
        t1.start()
        t2.start()
        t1.join()
        t2.join()
        for j in delays:
            for b in buffer_sizes:
                call(["./warp_simulator", os.path.join("server_frames", d), os.path.join("results", d, str(i), "client_frames/"), str(j), str(b)])
        call("rm " + os.path.join("results", d, str(i), "*", "*.rgb"), shell = True)
        call("rm " + os.path.join("results", d, str(i), "*", "*.mvp"), shell = True)
        call("rm " + os.path.join("results", d, str(i), "*", "*.depth"), shell = True)
        
