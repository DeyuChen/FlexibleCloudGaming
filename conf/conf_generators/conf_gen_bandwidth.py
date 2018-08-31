import os

dirs = ["textured", "non-textured"]

for d in dirs:
    bandwidth = 2000000
    
    if d == "textured":
        textured = "1"
    else:
        textured = "0"
        
    if not os.path.isdir(d):
        os.mkdir(d)
    if not os.path.isdir(os.path.join(d, "bw")):
        os.mkdir(os.path.join(d, "bw"))
    #if not os.path.isdir(os.path.join("../results", d, "bw")):
    #    os.mkdir(os.path.join("../results", d, "bw"))

    for i in range(0, 6):
        if not os.path.isdir(os.path.join(d, "bw", str(bandwidth))):
            os.mkdir(os.path.join(d, "bw", str(bandwidth)))

        f_server = open(os.path.join(d, "bw", str(bandwidth), "server"), "w")
        f_client = open(os.path.join(d, "bw", str(bandwidth), "client"), "w")

        s = "server_ip 127.0.0.1\n" + \
            "server_port 9487\n" + \
            "simplification_algorithm QUADRICTRI\n" + \
            "width 1280\n" + \
            "height 960\n" + \
            "color 1\n" + \
            "texture " + textured + "\n" + \
            "scale 100\n" + \
            "smooth 1\n" + \
            "renderingMode 1\n" + \
            "bandwidth " + str(bandwidth) + "\n" + \
            "gop 10\n" + \
            "framerate 1 30\n" + \
            "input_command_log results/command_log\n" + \
            "//output_command_log results/command_log\n" + \
            "model 0 models/town.ply\n" + \
            "model_pos 0 0 3 0\n" + \
            "bandwidth_log " + os.path.join("results", d, "bw", str(bandwidth), "bw_log") + "\n" + \
            "timing_log " + os.path.join("results", d, "bw", str(bandwidth), "timing_log") + "\n" + \
            "mesh_percentage 0\n"
        
        #s_server = "frame_output_path " + os.path.join("results", d, "bw", str(bandwidth), "server_frames") + "\n"
        s_client = "frame_output_path " + os.path.join("results", d, "bw", str(bandwidth), "client_frames") + "\n"
        #s_server = s_client = ""
        s_server = ""
        
        f_server.write(s + s_server)
        f_client.write(s + s_client)

        f_server.close();
        f_client.close();
        
        #if not os.path.isdir(os.path.join("../results", d, "bw", str(bandwidth))):
        #    os.mkdir(os.path.join("../results", d, "bw", str(bandwidth)))
        #if not os.path.isdir(os.path.join("../results", d, "bw", str(bandwidth), "server_frames")):
        #    os.mkdir(os.path.join("../results", d, "bw", str(bandwidth), "server_frames"))
        #if not os.path.isdir(os.path.join("../results", d, "bw", str(bandwidth), "client_frames")):
        #    os.mkdir(os.path.join("../results", d, "bw", str(bandwidth), "client_frames"))
        
        bandwidth = bandwidth * 2
