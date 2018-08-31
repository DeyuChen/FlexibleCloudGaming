import os
from subprocess import call
import threading

def worker(s1, s2):
    call([s1, s2])

dirs = ["textured"]
movements = ["forward", "backward", "left_shift", "right_shift", "left_turn", "right_turn"]
loss = [0, 1, 2, 3, 4, 5]

for d in dirs:
    for m in movements:
        for i in loss:
            t1 = threading.Thread(target = worker, args = ("./server", os.path.join("conf", d, "movement", m, str(i), "server")))
            t2 = threading.Thread(target = worker, args = ("./client", os.path.join("conf", d, "movement", m, str(i), "client")))
            t1.start()
            t2.start()
            t1.join()
            t2.join()
            call(["./warp_simulator", os.path.join("results", d, "movement", m, str(i), "server_frames/"), os.path.join("results", d, "movement", m, str(i), "client_frames/"), "0"])
            call("rm " + os.path.join("results", d, "movement", m, str(i), "*", "*.rgb"), shell = True)
            call("rm " + os.path.join("results", d, "movement", m, str(i), "*", "*.mvp"), shell = True)
            call("rm " + os.path.join("results", d, "movement", m, str(i), "*", "*.depth"), shell = True)
        
