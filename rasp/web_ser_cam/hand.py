import socket
import subprocess
import time
import sys
import os
import json
import select
import serial

stat_bfs = 0 

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=0.5)

def send_to_arduino(cmd):
    if ser:
        encoded_data = cmd.encode('ascii') 
        ser.write(encoded_data)

def read_from_arduino():
    if not ser:
        time.sleep(0.5)
        return ""
    
    if ser.in_waiting > 0:
        byte = ser.read(1)
        s = byte.decode('ascii', errors='ignore')
        rest = ser.readline().decode('ascii', errors='ignore').strip()
        full_str = s + rest
        return full_str
    
    return ""

def load_data_from_monitor():
    global stat_bfs
    filename = "maze_shared.json"
    if os.path.exists(filename) and os.path.getsize(filename) > 0:
        with open(filename, "r") as f:
            data = json.load(f)
            if isinstance(data, dict):
                return data.get("matrix")
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
    
    move_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    move_sock.bind(('127.0.0.1', 5003))

    while True:
        ready = select.select([move_sock], [], [], 0.05)
        if ready[0]:
            data, _ = move_sock.recvfrom(1024)
            rcv = data.decode('utf-8')

            if rcv == "finish":
                send_msg("finish")
                break
            
            elif rcv.startswith("cmd:"):
                action = rcv.split(":")[1]
                send_to_arduino(action)
                
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
                elif action == 'd':
                    corn = (corn + 90) % 360
                    if corn == 0 or corn == 360:  ry -= 1
                    elif corn == 180:             ry += 1
                    elif corn == 270:             rx -= 1
                    elif corn == 90:              rx += 1
                
                send_msg(f"pos:{rx},{ry}:{corn}") 

        cmd = read_from_arduino().strip().lower()
        if not cmd:
            continue

        if cmd == 'vl' or cmd == 'vr':
            send_msg(f"{cmd}:")
            continue

        elif cmd.startswith('v'):
            label = cmd[1:].upper() if len(cmd) > 1 else "F"
            filename = test_photos[photo_idx % len(test_photos)]
            photo_idx += 1
            send_msg(f"victim:{rx},{ry}:{label}:{filename}")
            continue

        if len(cmd) >= 4 and cmd[:4].isdigit():
            walls_str = cmd[:4]
            
            n_walls = 0
            if walls_str[0] == "1": n_walls += 1
            if walls_str[1] == "1": n_walls += 1
            if walls_str[2] == "1": n_walls += 1
            if walls_str[3] == "1": n_walls += 1
            
            sf = walls_str[0] == '1'
            sr = walls_str[1] == '1'
            sb = walls_str[2] == '1'
            sl = walls_str[3] == '1'

            calculated_walls = get_global_walls(corn, sf, sr, sb, sl)
            send_msg(f"wall:{rx},{ry}:{calculated_walls}")
            lab = load_data_from_monitor()

            stat_bfs = 0
            if 0 <= ry * 2 < 33 and 0 <= rx * 2 < 33:
                if lab[ry * 2][rx * 2] in [1, 3]:
                    stat_bfs = 1

            if n_walls == 3 or stat_bfs == 1:
                subprocess.run([sys.executable, bfs_path, str(corn)])
                if os.path.exists("maze_shared.json") and os.path.getsize("maze_shared.json") > 0:
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
                    send_to_arduino('d')
                elif not sf:
                    if corn == 0 or corn == 360:  ry -= 1
                    elif corn == 180:             ry += 1
                    elif corn == 270:             rx -= 1
                    elif corn == 90:              rx += 1
                    send_msg(f"pos:{rx},{ry}:{corn}")
                    send_to_arduino('w') 
                elif not sl:
                    corn = (corn - 90) % 360
                    if corn == 0 or corn == 360:  ry -= 1
                    elif corn == 180:             ry += 1
                    elif corn == 270:             rx -= 1
                    elif corn == 90:              rx += 1
                    send_msg(f"pos:{rx},{ry}:{corn}")
                    send_to_arduino('a')

            if not (0 <= rx < 16 and 0 <= ry < 16):
                rx, ry = 8, 8
                send_msg(f"pos:{rx},{ry}:{corn}")

    move_sock.close()

if __name__ == "__main__":
    main()