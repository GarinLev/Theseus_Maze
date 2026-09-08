import socket
import json
import threading
from flask import Flask, jsonify, render_template_string

app = Flask(__name__)
LAB_SIZE = 33

state = {
    "matrix": [[0]*LAB_SIZE for _ in range(LAB_SIZE)],
    "robot_x": 16,
    "robot_y": 16,
    "corn": 0,
    "logs": ["Web Monitor 33x33 started."],
    "victims": []
}

def udp_listener():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('127.0.0.1', 5002))
    
    while True:
        data, _ = sock.recvfrom(1024)
        text = data.decode('utf-8')
        parts = text.split(':')
        cmd = parts[0]
        
        state["logs"].append(f"[UDP]: {text}")
        if len(state["logs"]) > 20:
            state["logs"].pop(0)

        vx, vy = state["robot_x"] // 2, state["robot_y"] // 2

        if cmd == 'pos':
            coords = parts[1].split(',')
            state["robot_x"] = int(coords[0]) * 2
            state["robot_y"] = int(coords[1]) * 2
            state["corn"] = int(parts[2])
            
        elif cmd == 'wall':
            coords = parts[1].split(',')
            cx, cy = int(coords[0]) * 2, int(coords[1]) * 2
            wall_list = parts[2].split(',')
            
            if 0 <= cx < LAB_SIZE and 0 <= cy < LAB_SIZE:
                if 'up' in wall_list and cy - 1 >= 0:
                    if state["matrix"][cy - 1][cx] != 4: state["matrix"][cy - 1][cx] = 2
                if 'down' in wall_list and cy + 1 < LAB_SIZE:
                    if state["matrix"][cy + 1][cx] != 4: state["matrix"][cy + 1][cx] = 2
                if 'left' in wall_list and cx - 1 >= 0:
                    if state["matrix"][cy][cx - 1] != 4: state["matrix"][cy][cx - 1] = 2
                if 'right' in wall_list and cx + 1 < LAB_SIZE:
                    if state["matrix"][cy][cx + 1] != 4: state["matrix"][cy][cx + 1] = 2

        elif cmd == 'victim':
            state["victims"].append({'pos': f"{vx},{vy}", 'label': parts[2]})
            if state["corn"] in [0, 360] and vy*2 - 1 >= 0: state["matrix"][vy*2 - 1][vx*2] = 4
            elif state["corn"] == 180 and vy*2 + 1 < LAB_SIZE: state["matrix"][vy*2 + 1][vx*2] = 4
            elif state["corn"] == 270 and vx*2 - 1 >= 0: state["matrix"][vy*2][vx*2 - 1] = 4
            elif state["corn"] == 90 and vx*2 + 1 < LAB_SIZE: state["matrix"][vy*2][vx*2 + 1] = 4

        data_to_save = {
            "matrix": state["matrix"],
            "robot_x": state["robot_x"],  
            "robot_y": state["robot_y"],  
            "corn": state["corn"]
        }
        with open("maze_shared.json", "w") as f:
            json.dump(data_to_save, f)

@app.route('/api/state')
def get_state():
    return jsonify(state)

HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <title>Maze Web Monitor</title>
    <style>
        body { background-color: #1a1a1a; color: #fff; font-family: monospace; display: flex; }
        canvas { border: 1px solid #333; margin: 20px; }
        #logs { margin-top: 20px; max-height: 400px; overflow-y: auto; color: #00ff00; }
    </style>
</head>
<body>
    <div>
        <canvas id="mazeCanvas" width="660" height="660"></canvas>
    </div>
    <div>
        <h2>Rescue Maze Terminal</h2>
        <p>Robot Pos: <span id="pos"></span></p>
        <div id="logs"></div>
    </div>
    <script>
        const canvas = document.getElementById('mazeCanvas');
        const ctx = canvas.getContext('2d');
        const CELL_SIZE = 20;

        function draw(state) {
            ctx.fillStyle = '#141419';
            ctx.fillRect(0, 0, canvas.width, canvas.height);

            for(let y = 0; y < 33; y++) {
                for(let x = 0; x < 33; x++) {
                    let val = state.matrix[y][x];
                    let px = x * CELL_SIZE;
                    let py = y * CELL_SIZE;

                    if (x % 2 === 0 && y % 2 === 0) {
                        ctx.fillStyle = val === 3 ? '#662299' : (val === 1 ? '#383847' : '#141419');
                        ctx.fillRect(px, py, CELL_SIZE, CELL_SIZE);
                        ctx.strokeStyle = '#222';
                        ctx.strokeRect(px, py, CELL_SIZE, CELL_SIZE);
                    } else if (val === 2 || val === 4) {
                        ctx.fillStyle = val === 4 ? '#00ff4c' : '#ff6600';
                        ctx.fillRect(px, py, CELL_SIZE, CELL_SIZE);
                    }
                }
            }

            ctx.fillStyle = '#0073e6';
            ctx.fillRect(state.robot_x * CELL_SIZE, state.robot_y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
            
            document.getElementById('pos').innerText = `[${state.robot_x/2}, ${state.robot_y/2}] @ ${state.corn}°`;
            document.getElementById('logs').innerHTML = state.logs.map(l => l + "<br>").join('');
        }

        setInterval(() => {
            fetch('/api/state').then(r => r.json()).then(data => draw(data));
        }, 300);
    </script>
</body>
</html>
"""

@app.route('/')
def index():
    return render_template_string(HTML_TEMPLATE)

if __name__ == '__main__':
    threading.Thread(target=udp_listener, daemon=True).start()
    app.run(host='0.0.0.0', port=5000)