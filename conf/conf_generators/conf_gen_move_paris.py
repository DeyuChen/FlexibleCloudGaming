import os

dirs = ["textured", "non-textured"]
movements = ["forward", "backward", "left_shift", "right_shift", "left_turn", "right_turn"]

for d in dirs:
    if d == "textured":
        textured = "1"
    else:
        textured = "0"
        
    if not os.path.isdir(d):
        os.mkdir(d)
        
    if not os.path.isdir(os.path.join(d, "movement")):
        os.mkdir(os.path.join(d, "movement"))
    if not os.path.isdir(os.path.join("../results", d, "movement")):
        os.mkdir(os.path.join("../results", d, "movement"))
        
    for m in movements:
        if not os.path.isdir(os.path.join(d, "movement", m)):
            os.mkdir(os.path.join(d, "movement", m))
        if not os.path.isdir(os.path.join("../results", d, "movement", m)):
            os.mkdir(os.path.join("../results", d, "movement", m))
        
        x = -6.5
        y = -45
        if m == "forward":
            y = -85
        elif m == "backward":
            y = -5
        if m == "right_shift":
            x = 33.5
        elif m == "left_shift":
            x = -46.5
            
        for i in range(0, 101, 10):
            if not os.path.isdir(os.path.join(d, "movement", m, str(i))):
                os.mkdir(os.path.join(d, "movement", m, str(i)))
            
            f_server = open(os.path.join(d, "movement", m, str(i), "server"), "w")
            f_client = open(os.path.join(d, "movement", m, str(i), "client"), "w")

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
                "bandwidth 8000000\n" + \
                "gop 10\n" + \
                "framerate 1 30\n" + \
                "input_command_log results/" + m + ".cmd\n" + \
                "model 0 models/paris/paris.ply\n" + \
                "model_pos 0 " + str(x) + " -1.5 " + str(y) + "\n" + \
                "azimuth 0\n" + \
                "bandwidth_log " + os.path.join("results", d, "movement", m, str(i), "bw_log") + "\n" + \
                "timing_log " + os.path.join("results", d, "movement", m, str(i), "timing_log") + "\n" + \
                "mesh_percentage " + str(i) + "\n"
            
            s_server = "frame_output_path " + os.path.join("results", d, "movement", m, str(i), "server_frames") + "\n"
            s_client = "frame_output_path " + os.path.join("results", d, "movement", m, str(i), "client_frames") + "\n"
            #s_server = s_client = ""
            #s_server = ""
            
            if not os.path.isdir(os.path.join("../results", d, "movement", m, str(i))):
                os.mkdir(os.path.join("../results", d, "movement", m, str(i)))
            if not os.path.isdir(os.path.join("../results", d, "movement", m, str(i), "server_frames")):
                os.mkdir(os.path.join("../results", d, "movement", m, str(i), "server_frames"))
            if not os.path.isdir(os.path.join("../results", d, "movement", m, str(i), "client_frames")):
                os.mkdir(os.path.join("../results", d, "movement", m, str(i), "client_frames"))
            
            f_server.write(s + s_server)
            f_client.write(s + s_client)

            f_server.close();
            f_client.close();
