import socket
import subprocess
import time
import sys
import os
import json

stat_bfs = 0 

def load_data_from_monitor():
    global stat_bfs
    filename = "maze_shared.json"
    if os.path.exists(filename):
        with open(filename, "r") as f:
            data = json.load(f)
            if isinstance(data, dict):
                return data.get("matrix")           
            else:
                return data  
    return [[0 for _ in range(33)] for _ in range(33)]

current_dir = os.path.dirname(os.path.abspath(__file__))
bfs_path = os.path.join(current_dir, "bfs.py")
TARGET_IP = "127.0.0.1"
TARGET_PORT = 5002
sender_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_msg(text):
    sender_sock.sendto(text.encode('utf-8'), (TARGET_IP, TARGET_PORT))

def get_global_walls(corn, wall_front, wall_right, wall_back, wall_left):
    global_walls = []
    if corn == 0 or corn == 360:
        if wall_front: global_walls.append('up')
        if wall_right: global_walls.append('right')
        if wall_back:  global_walls.append('down')
        if wall_left:  global_walls.append('left')
    elif corn == 180:
        if wall_front: global_walls.append('down')
        if wall_right: global_walls.append('left')
        if wall_back:  global_walls.append('up')
        if wall_left:  global_walls.append('right')
    elif corn == 270:
        if wall_front: global_walls.append('left')
        if wall_right: global_walls.append('up')
        if wall_back:  global_walls.append('right')
        if wall_left:  global_walls.append('down')
    elif corn == 90:
        if wall_front: global_walls.append('right')
        if wall_right: global_walls.append('down')
        if wall_back:  global_walls.append('left')
        if wall_left:  global_walls.append('up')
    return ",".join(global_walls)

def main():
    rx, ry = 8, 8  
    corn = 0       
    test_photos = ['test_h.jpg', 'test_s.jpg', 'test_red.jpg']
    photo_idx = 0
    
    send_msg(f"pos:{rx},{ry}:{corn}")
    dir_names = {0: "UP", 360: "UP", 90: "RIGHT", 180: "DOWN", 270: "LEFT"}

    while True:
        current_dir_name = dir_names.get(corn, f"{corn}°")
        print(f"\n[{rx}, {ry}] {current_dir_name} ({corn}°)")
        
        cmd = input("cmd: ").strip().lower()
        
        if cmd == 'move':
            move_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            move_sock.bind(('127.0.0.1', 5003))
            move_sock.settimeout(0.5) 

            try:
                while True:
                    try:
                        data, _ = move_sock.recvfrom(1024)
                        rcv = data.decode('utf-8')

                        if rcv == "finish":
                            send_msg("finish") 
                            time.sleep(0.1)
                            break
                        
                        elif rcv.startswith("cmd:"):
                            action = rcv.split(":")[1]
                            if action == 'w':
                                if corn == 0 or corn == 360: ry -= 1
                                elif corn == 180: ry += 1
                                elif corn == 270: rx -= 1
                                elif corn == 90: rx += 1
                            elif action == 's':
                                if corn == 0 or corn == 360: ry += 1
                                elif corn == 180: ry -= 1
                                elif corn == 270: rx += 1
                                elif corn == 90: rx -= 1
                                corn = (corn - 180) % 360
                            elif action == 'a':
                                corn = (corn - 90) % 360
                                if corn == 0 or corn == 360:  ry -= 1
                                elif corn == 180:             ry += 1
                                elif corn == 270:             rx -= 1
                                elif corn == 90:              rx += 1
                                send_msg(f"pos:{rx},{ry}:{corn}")
                            elif action == 'd':
                                corn = (corn + 90) % 360
                                if corn == 0 or corn == 360:  ry -= 1
                                elif corn == 180:             ry += 1
                                elif corn == 270:             rx -= 1
                                elif corn == 90:              rx += 1
                                send_msg(f"pos:{rx},{ry}:{corn}")
                            send_msg(f"pos:{rx},{ry}:{corn}") 
                            
                    except socket.timeout:
                        pass 
                        
            except KeyboardInterrupt:
                move_sock.close()
                continue

        elif cmd == 'vl' or cmd == 'vr':
            send_msg(f"{cmd}:")
            continue

        elif cmd == 'v':
            label = input("(F/nedoF/OM)").strip().upper() or "F"
            filename = test_photos[photo_idx % len(test_photos)]
            photo_idx += 1
            send_msg(f"victim:{rx},{ry}:{label}:{filename}")
            continue

        if len(cmd) != 4 or not cmd.isdigit():
            continue
        
        n_walls = 0
        if cmd[0] == "1": n_walls += 1
        if cmd[1] == "1": n_walls += 1
        if cmd[2] == "1": n_walls += 1
        if cmd[3] == "1": n_walls += 1
        sf = cmd[0] == '1'
        sr = cmd[1] == '1'
        sb = cmd[2] == '1'
        sl = cmd[3] == '1'

        calculated_walls = get_global_walls(corn, sf, sr, sb, sl)
        send_msg(f"wall:{rx},{ry}:{calculated_walls}")
        lab = load_data_from_monitor()

        stat_bfs = 0
        if 0 <= ry * 2 < 33 and 0 <= rx * 2 < 33:
            if lab[ry * 2][rx * 2] in [1, 3]:
                stat_bfs = 1

        if n_walls == 3 or stat_bfs == 1:
            subprocess.run(["python", bfs_path, str(corn)])
            with open("maze_shared.json", "r") as f:
                data = json.load(f)
                if isinstance(data, dict):
                    rx = data.get("robot_x", rx * 2) // 2
                    ry = data.get("robot_y", ry * 2) // 2
                    corn = data.get("corn", corn)
        else:
            if not sr:
                corn = (corn + 90) % 360
                if corn == 0 or corn == 360:  ry -= 1
                elif corn == 180:             ry += 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
                send_msg(f"pos:{rx},{ry}:{corn}")
            elif not sf:
                if corn == 0 or corn == 360:  ry -= 1
                elif corn == 180:             ry += 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
                send_msg(f"pos:{rx},{ry}:{corn}")
            elif not sl:
                corn = (corn - 90) % 360
                if corn == 0 or corn == 360:  ry -= 1
                elif corn == 180:             ry += 1
                elif corn == 270:             rx -= 1
                elif corn == 90:              rx += 1
                send_msg(f"pos:{rx},{ry}:{corn}")

        if not (0 <= rx < 16 and 0 <= ry < 16):
            rx, ry = 8, 8
            send_msg(f"pos:{rx},{ry}:{corn}")

if __name__ == "__main__":
    main()