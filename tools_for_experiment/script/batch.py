import os
from subprocess import call
import threading

def worker(s1, s2):
    call([s1, s2])

dirs = ["textured", "non-textured"]

for d in dirs:
    for i in range(0, 91, 10):
        t1 = threading.Thread(target = worker, args = ("./server", os.path.join("conf", d, str(i), "server")))
        t2 = threading.Thread(target = worker, args = ("./client", os.path.join("conf", d, str(i), "client")))
        t1.start()
        t2.start()
        t1.join()
        t2.join()
        call(["./warp_simulator", os.path.join("server_frames", d), os.path.join("results", d, str(i), "client_frames/"), "0"])
        call("rm " + os.path.join("results", d, str(i), "*", "*.rgb"), shell = True)
        call("rm " + os.path.join("results", d, str(i), "*", "*.mvp"), shell = True)
        call("rm " + os.path.join("results", d, str(i), "*", "*.depth"), shell = True)
        
