import socket
import time
import json
import os
import sys

LAB_SIZE = 33

sock_out = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
MONITOR_IP = "127.0.0.1"
MONITOR_PORT = 5002
bfs_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_msg_to_monitor(text: str):
    bfs_sock.sendto(text.encode('utf-8'), (MONITOR_IP, MONITOR_PORT))

def load_data_from_monitor():
    filename = "maze_shared.json"
    if os.path.exists(filename) and os.path.getsize(filename) > 0:
        with open(filename, "r", encoding="utf-8") as f:
            content = f.read().strip()
        if content.startswith("{") and content.endswith("}"):
            data = json.loads(content)
            if isinstance(data, dict):
                rx = data.get("robot_x", 16)
                ry = data.get("robot_y", 16)
                lab = data.get("matrix", [[0 for _ in range(LAB_SIZE)] for _ in range(LAB_SIZE)])
                corn = data.get("corn", 0)
                return lab, rx, ry, corn
    return [[0 for _ in range(LAB_SIZE)] for _ in range(LAB_SIZE)], 16, 16, 0

def bfs(sent):
    global curr, path, queue, robot_x, robot_y, found
    lab, robot_x, robot_y, corn = load_data_from_monitor()
    corn = int(corn)

    if 0 <= robot_y < LAB_SIZE and 0 <= robot_x < LAB_SIZE:
        lab[robot_y][robot_x] = 1
    else:
        return

    send_msg_to_monitor("start bfs")
    found = False
    neimber = [[0, -2], [2, 0], [0, 2], [-2, 0]]
    neimber_wall = [[0, -1], [1, 0], [0, 1], [-1, 0]]

    queue = [[robot_x, robot_y]]
    path = [-1]
    curr = 0

    while curr < len(queue):
        nx, ny = queue[curr][0], queue[curr][1]
        if lab[ny][nx] == sent and (nx != robot_x or ny != robot_y):
            found = True
            break
            
        for ab in range(len(neimber)):
            tx = nx + neimber[ab][0]
            ty = ny + neimber[ab][1]
            wx = nx + neimber_wall[ab][0]
            wy = ny + neimber_wall[ab][1]
            
            if 0 <= ty < LAB_SIZE and 0 <= tx < LAB_SIZE and 0 <= wy < LAB_SIZE and 0 <= wx < LAB_SIZE:
                if lab[wy][wx] == 0 and [tx, ty] not in queue:
                    queue.append([tx, ty])
                    path.append(curr)
        curr += 1

def ride():
    global curr, path, queue, robot_x, robot_y
    target_idx = curr
    actual_path = []
    
    while target_idx != -1:
        actual_path.append(queue[target_idx])
        target_idx = path[target_idx]
    actual_path.reverse()
    
    commands = []
    curr_x, curr_y = robot_x, robot_y

    for i in range(1, len(actual_path)):
        tx, ty = actual_path[i]
        dx, dy = tx - curr_x, ty - curr_y

        if dy > 0:    commands.append('d')
        elif dy < 0:  commands.append('u')
        elif dx > 0:  commands.append('r')
        elif dx < 0:  commands.append('l')
        curr_x, curr_y = tx, ty
        
    print(",".join(commands))

if __name__ == "__main__":
    time.sleep(0.1)
    bfs(0)
    if found:
        ride()
    else:
        bfs(3)
        ride()
        send_msg_to_monitor("finish")