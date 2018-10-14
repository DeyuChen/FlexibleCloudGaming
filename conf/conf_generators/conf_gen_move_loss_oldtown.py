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
        
        x = 0
        y = 20
        azimuth = 0
        if m == "forward":
            x = 40
            azimuth = 90
        elif m == "backward":
            x = -40
            azimuth = 90
        elif m == "right_shift":
            x = 40
        elif m == "left_shift":
            x = -40
            
        for i in range(0, 7, 1):
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
                "model 0 models/oldtown/oldtown.ply\n" + \
                "model_pos 0 " + str(x) + " -2 " + str(y) + "\n" + \
                "azimuth " + str(azimuth) + "\n" + \
                "bandwidth_log " + os.path.join("results", d, "movement", m, str(i), "bw_log") + "\n" + \
                "timing_log " + os.path.join("results", d, "movement", m, str(i), "timing_log") + "\n" + \
                "mesh_percentage 90\n" + \
                "artificial_loss_rate 0.0" + str(i) + "\n"
            
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
