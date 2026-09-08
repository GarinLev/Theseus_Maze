import socket
import subprocess
import time
import sys
import os
import json
import select
import serial

stat_bfs = 0 
stat_black = 0
stat_cam = 1

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=0.5)

def send_to_arduino(cmd):
    if ser:
        encoded_data = cmd.encode('ascii') 
        ser.write(encoded_data)

def read_from_arduino():
    if not ser:
        time.sleep(0.5)
        print("noardsend")
        return ""
    
    if ser.in_waiting > 0:
        byte = ser.read(1)
        s = byte.decode('ascii', errors='ignore')
        rest = ser.readline().decode('ascii', errors='ignore').strip()
        full_str = s + rest
        # print(full_str)
        return full_str
    
    return ""

camera_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def set_camera_pause(paused: bool):
    msg = "pause" if paused else "resume"
    try:
        camera_sock.sendto(msg.encode('utf-8'), ('127.0.0.1', 5004))
    except Exception as e:
        print(f"Ошибка отправки UDP камере: {e}")

def load_data_from_monitor():
    global stat_bfs
    filename = "maze_shared.json"
    while True:
        if os.path.exists(filename) and os.path.getsize(filename) > 0:
            try:
                with open(filename, "r") as f:
                    data = json.load(f)
                    if isinstance(data, dict):
                        return data.get("matrix")
                    return data
            except (json.JSONDecodeError, PermissionError):
                time.sleep(0.05)
        else:
            time.sleep(0.05)

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

def run_bfs_route(corn, rx, ry):
    global stat_cam
    result = subprocess.run(
        [sys.executable, bfs_path, str(rx), str(ry), str(corn)], 
        capture_output=True, 
        text=True
    )
    if result.stderr:
        print("\n[ДЕБАГ ИЗ BFS.PY]:", result.stderr.strip())
    bfs_output = result.stdout.strip()
    if bfs_output:
        route_commands = bfs_output.split(',')
        num_commands = len(route_commands)
        set_camera_pause(True)
        
        for i, bfs_action in enumerate(route_commands):
            target_angles = {'u': 0, 'r': 90, 'd': 180, 'l': 270}
            target_corn = target_angles.get(bfs_action, corn)
            angle_diff = (target_corn - corn) % 360
            
            if angle_diff == 0:    arduino_cmd = 'u'
            elif angle_diff == 90:  arduino_cmd = 'r'
            elif angle_diff == 270: arduino_cmd = 'l'
            elif angle_diff == 180: arduino_cmd = 'd' 
            
            ser.reset_input_buffer()
            
            send_to_arduino(arduino_cmd)
            print(arduino_cmd)
            if bfs_action == 'u':    ry -= 1
            elif bfs_action == 'd':  ry += 1
            elif bfs_action == 'l':  rx -= 1
            elif bfs_action == 'r':  rx += 1
            corn = target_corn
            
            send_msg(f"pos:{rx},{ry}:{corn}")
            
            if i < num_commands - 1:
                serial_buffer = ""
                while len(serial_buffer) < 4:
                    if ser.in_waiting > 0:
                        serial_buffer += ser.read(ser.in_waiting).decode('ascii', errors='ignore')
                    time.sleep(0.01)
            else:
                pass
        print("bfstop4ik")
        if rx == 8 and ry == 8:
            send_msg("finish")
                
    return corn, rx, ry

def main():
    global stat_bfs, stat_black, stat_cam
    ser.write(b"s")
    rx, ry = 8, 8
    corn = 0
    chk_rx, chk_ry, chk_corn = 8, 8, 0
    prev_rx, prev_ry, prev_corn = 8, 8, 0
    
    send_msg(f"pos:{rx},{ry}:{corn}")
    
    move_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    move_sock.bind(('127.0.0.1', 5003))

    while True:
        ready = select.select([move_sock], [], [], 0.05)
        n_walls = 0
        if ready[0]:
            data, _ = move_sock.recvfrom(1024)
            rcv = data.decode('utf-8')

            if rcv == "finish":
                send_msg("finish")
                break
            
            elif rcv.startswith("cmd:"):
                bfs_action = rcv.split(":")[1] 
                target_angles = {'u': 0, 'r': 90, 'd': 180, 'l': 270}
                target_corn = target_angles.get(bfs_action, corn)
                angle_diff = (target_corn - corn) % 360

                if angle_diff == 0:     arduino_cmd = 'u'
                elif angle_diff == 90:  arduino_cmd = 'r'
                elif angle_diff == 270: arduino_cmd = 'l'
                elif angle_diff == 180: arduino_cmd = 's' 

                send_to_arduino(arduino_cmd)
                time.sleep(1.5)
                
                if bfs_action == 'u': ry -= 1
                elif bfs_action == 'd': ry += 1
                elif bfs_action == 'l': rx -= 1
                elif bfs_action == 'r': rx += 1
                
                corn = target_corn
                send_msg(f"pos:{rx},{ry}:{corn}")

        cmd = read_from_arduino().strip().lower()
        cmd_pc = 0

        if cmd_pc == "move":
            while cmd_pc != "stop":
                cmd_pc = input()
                if cmd_pc == "d":
                    send_to_arduino('r')
                    time.sleep(1.2)  # Ждем физический поворот
                    corn = (corn + 90) % 360
                    send_msg(f"pos:{rx},{ry}:{corn}")  # Координаты rx, ry НЕ МЕНЯЕМ при повороте
                elif cmd_pc == "a":
                    send_to_arduino('l')
                    time.sleep(1.2)  # Ждем физический поворот
                    corn = (corn - 90) % 360
                    send_msg(f"pos:{rx},{ry}:{corn}")  # Координаты rx, ry НЕ МЕНЯЕМ при повороте
                elif cmd_pc == "w":
                    send_to_arduino('u')
                    time.sleep(1.5)  # Ждем физический доезд в следующую клетку
                    if corn == 0 or corn == 360:  ry -= 1
                    elif corn == 180:             ry += 1
                    elif corn == 270:             rx -= 1
                    elif corn == 90:              rx += 1
                    send_msg(f"pos:{rx},{ry}:{corn}")
                elif cmd_pc == "s":
                    send_to_arduino('d')
                    time.sleep(1.5)  # Ждем физический доезд назад
                    corn = (corn - 180) % 360
                    if corn == 0 or corn == 360:  ry += 1
                    elif corn == 180:             ry -= 1
                    elif corn == 270:             rx += 1
                    elif corn == 90:              rx -= 1
                    send_msg(f"pos:{rx},{ry}:{corn}")

        if not cmd:
            continue

        if cmd.startswith('s') and len(cmd) > 1:
            cmd = cmd[1:]
        
        print("i_see:", cmd)
        if cmd == 'p':
            while True:
                rcmd = read_from_arduino().strip().lower()
                if rcmd == 'o':
                    rx, ry, corn = chk_rx, chk_ry, chk_corn
                    send_msg(f"pos:{rx},{ry}:{corn}")
                    break
                time.sleep(0.1)
            continue

        elif cmd == 's':
            ser.write(b"s")
            time.sleep(0.05)
            continue

        if len(cmd) >= 4 and cmd[:4].isdigit():
            set_camera_pause(False)
            print("yes")
            print("walls:", cmd)
            send_msg(f"pos:{rx},{ry}:{corn}")
            walls_str = cmd[:4]
            stat_pl = cmd[4] if len(cmd) >= 5 else '0'

            if stat_pl == '1':
                chk_rx, chk_ry, chk_corn = rx, ry, corn
                send_msg(f"tile:{rx},{ry}:silver")

            elif stat_pl == '2':
                send_msg(f"tile:{rx},{ry}:black")
                send_msg(f"wall:{rx},{ry}:up,down,left,right")
                rx, ry = prev_rx, prev_ry
                send_msg(f"pos:{rx},{ry}:{corn}")
                stat_bfs = 1
                stat_black = 1

            sf = walls_str[0] == '1'
            sr = walls_str[1] == '1'
            sb = walls_str[2] == '1'
            sl = walls_str[3] == '1'
            if sf == True: n_walls += 1
            if sr == True: n_walls += 1
            if sl == True: n_walls += 1
            if sb == True: n_walls += 1

            if stat_black != 1:
                calculated_walls = get_global_walls(corn, sf, sr, sb, sl)
                send_msg(f"wall:{rx},{ry}:{calculated_walls}")
            else:
                stat_black = 0
            lab = load_data_from_monitor()

            if n_walls >= 3 or stat_bfs == 1:
                stat_bfs = 0
                corn, rx, ry = run_bfs_route(corn, rx, ry)
                prev_rx, prev_ry, prev_corn = rx, ry, corn
            else:
                prev_rx, prev_ry, prev_corn = rx, ry, corn
                next_x, next_y = rx, ry
                if not sr:
                    time.sleep(1.5)
                    send_to_arduino('r')
                    corn = (corn + 90) % 360
                    if corn == 0 or corn == 360:  next_y -= 1
                    elif corn == 180:             next_y += 1
                    elif corn == 270:             next_x -= 1
                    elif corn == 90:              next_x += 1
                elif not sf:
                    time.sleep(1.5)
                    send_to_arduino('u')
                    if corn == 0 or corn == 360:  next_y -= 1
                    elif corn == 180:             next_y += 1
                    elif corn == 270:             next_x -= 1
                    elif corn == 90:              next_x += 1
                    stat_cam = 0
                elif not sl:
                    time.sleep(1.5)
                    send_to_arduino('l')
                    corn = (corn - 90) % 360
                    if corn == 0 or corn == 360:  next_y -= 1
                    elif corn == 180:             next_y += 1
                    elif corn == 270:             next_x -= 1
                    elif corn == 90:              next_x += 1

                if 0 <= next_y * 2 < 33 and 0 <= next_x * 2 < 33:
                    if lab[next_y * 2][next_x * 2] == 1:
                        stat_bfs = 1
                
                rx, ry = next_x, next_y
            if stat_cam == 0:
                set_camera_pause(True)

            if not (0 <= rx < 16 and 0 <= ry < 16):
                rx, ry = 8, 8
                send_msg(f"pos:{rx},{ry}:{corn}")

    move_sock.close()

if __name__ == "__main__":
    main()