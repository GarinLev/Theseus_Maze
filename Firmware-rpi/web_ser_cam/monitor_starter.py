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
state["matrix"][16][16] = 3

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

        vx, vy = state["robot_x"], state["robot_y"]

        if cmd == 'pos':
            coords = parts[1].split(',')
            state["robot_x"] = int(coords[0]) * 2
            state["robot_y"] = int(coords[1]) * 2
            state["corn"] = int(parts[2])
            if state["matrix"][state["robot_y"]][state["robot_x"]] == 0:
                state["matrix"][state["robot_y"]][state["robot_x"]] = 1

        elif cmd == 'wall':
            coords = parts[1].split(',')
            wx_base = int(coords[0]) * 2
            wy_base = int(coords[1]) * 2
            walls = parts[2].split(',')
            
            for w in walls:
                wx, wy = wx_base, wy_base
                if w == 'up':    wy -= 1
                elif w == 'down':  wy += 1
                elif w == 'left':  wx -= 1
                elif w == 'right': wx += 1

                if 0 <= wx < LAB_SIZE and 0 <= wy < LAB_SIZE:
                    if state["matrix"][wy][wx] == 3:
                        continue 
                    if state["matrix"][wy][wx] == 4:
                        continue
                    state["matrix"][wy][wx] = 2

        elif cmd == 'victim':
            tx, ty = vx, vy
            if state["corn"] == 0:
                if parts[1] == 'l': tx -= 1
                elif parts[1] == 'r': tx += 1
            elif state["corn"] == 90:
                if parts[1] == 'l': ty -= 1
                elif parts[1] == 'r': ty += 1
            elif state["corn"] == 180:
                if parts[1] == 'l': tx += 1
                elif parts[1] == 'r': tx -= 1
            elif state["corn"] == 270:
                if parts[1] == 'l': ty += 1
                elif parts[1] == 'r': ty -= 1

            if 0 <= tx < LAB_SIZE and 0 <= ty < LAB_SIZE:
                state["matrix"][ty][tx] = 4
                state["victims"].append({"pos": f"{tx//2},{ty//2}", "label": parts[2]})
            
            # ИСПРАВЛЕНО: Если это жертва, которой положен 1 или 2 набора, 
            # помечаем саму КЛЕТКУ робота как 4, чтобы сохранить статус в общем JSON
            if parts[2] in ['1', '2']:
                if 0 <= vx < LAB_SIZE and 0 <= vy < LAB_SIZE:
                    state["matrix"][vy][vx] = 4

        try:
            with open("maze_shared.json", "w", encoding="utf-8") as f:
                json.dump(state, f)
        except Exception as e:
            print(f"Error writing to JSON: {e}")

@app.route('/api/state')
def get_state():
    return jsonify(state)

@app.route('/')
def index():
    return render_template_string('''
    <!DOCTYPE html>
    <html>
    <head>
        <title>Maze Monitor</title>
        <style>
            body { background: #141419; color: #e1e1e6; font-family: 'Segoe UI', sans-serif; margin: 30px; }
            h2 { color: #50fa7b; margin-bottom: 20px; font-weight: 500; font-family: monospace; }
            #layout { display: flex; gap: 25px; }
            canvas { background: #0b0b0e; border: 2px solid #222227; border-radius: 8px; box-shadow: 0 4px 20px rgba(0,0,0,0.6); }
            #panel { width: 340px; display: flex; flex-direction: column; gap: 15px; }
            .box { background: #1a1a24; padding: 18px; border-radius: 8px; border: 1px solid #2d2d3d; }
            .box h3 { margin-top: 0; margin-bottom: 12px; font-size: 13px; text-transform: uppercase; letter-spacing: 1px; color: #8f8f9d; }
            #pos { color: #ff79c6; font-weight: bold; font-family: monospace; font-size: 20px; }
            #logs { font-size: 11px; height: 220px; overflow-y: auto; background: #070709; padding: 8px; font-family: monospace; border-radius: 4px; border: 1px solid #21212b; }
            #victims { font-size: 12px; font-family: monospace; display: flex; flex-direction: column; gap: 6px; }
            .vic-item { background: rgba(80, 250, 123, 0.1); border-left: 3px solid #50fa7b; padding: 6px 10px; border-radius: 4px; color: #50fa7b; }
        </style>
    </head>
    <body>
        <h2>Rescue Maze PRO-Monitor</h2>
        <div id="layout">
            <canvas id="mazeCanvas" width="540" height="540"></canvas>
            <div id="panel">
                <div class="box"><h3>Robot Position</h3><div id="pos"></div></div>
                <div class="box"><h3>Victims Detected</h3><div id="victims"></div></div>
                <div class="box"><h3>UDP Stream Logs</h3><div id="logs"></div></div>
            </div>
        </div>
        <script>
            const canvas = document.getElementById('mazeCanvas');
            const ctx = canvas.getContext('2d');
            
            const NODE_SIZE = 24; 
            const WALL_THICK = 6; 
            const PADDING = 10;   
            
            function getCoords(i) {
                let pos = PADDING;
                for (let c = 0; c < i; c++) {
                    pos += (c % 2 === 0) ? NODE_SIZE : WALL_THICK;
                }
                let size = (i % 2 === 0) ? NODE_SIZE : WALL_THICK;
                return { pos, size };
            }

            function draw(state) {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                
                for (let y = 0; y < 33; y++) {
                    let yData = getCoords(y);
                    for (let x = 0; x < 33; x++) {
                        let xData = getCoords(x);
                        let val = state.matrix[y][x];
                        
                        if (x % 2 === 0 && y % 2 === 0) {
                            if (val === 3) ctx.fillStyle = '#662299'; 
                            else if (val === 4) ctx.fillStyle = '#50fa7b'; // ИСПРАВЛЕНО: Клетка-жертва теперь тоже рисуется зеленым!
                            else if (val === 1) ctx.fillStyle = '#2d2d3d'; 
                            else ctx.fillStyle = '#111116'; 
                            
                            ctx.fillRect(xData.pos, yData.pos, xData.size, yData.size);
                            ctx.strokeStyle = '#1c1c24';
                            ctx.lineWidth = 1;
                            ctx.strokeRect(xData.pos, yData.pos, xData.size, yData.size);
                        } else {
                            if (val === 2) {
                                ctx.fillStyle = '#ff5555'; 
                                ctx.fillRect(xData.pos, yData.pos, xData.size, yData.size);
                            } else if (val === 4) {
                                ctx.fillStyle = '#50fa7b'; 
                                ctx.fillRect(xData.pos, yData.pos, xData.size, yData.size);
                            } else if (val === 1 || val === 3) {
                                ctx.fillStyle = '#22222f'; 
                                ctx.fillRect(xData.pos, yData.pos, xData.size, yData.size);
                            }
                        }
                    }
                }
                
                let rxData = getCoords(state.robot_x);
                let ryData = getCoords(state.robot_y);
                
                ctx.fillStyle = '#8be9fd'; 
                ctx.fillRect(rxData.pos, ryData.pos, rxData.size, ryData.size);
                
                ctx.fillStyle = '#141419';
                ctx.font = 'bold 16px monospace';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                let arrow = '^';
                if (state.corn === 90) arrow = '>';
                else if (state.corn === 180) arrow = 'v';
                else if (state.corn === 270) arrow = '<';
                ctx.fillText(arrow, rxData.pos + rxData.size / 2, ryData.pos + ryData.size / 2);

                document.getElementById('pos').innerText = `X: ${Math.floor(state.robot_x/2)}, Y: ${Math.floor(state.robot_y/2)} @ ${state.corn}°`;
                
                let logDiv = document.getElementById('logs');
                logDiv.innerHTML = state.logs.slice().reverse().map(l => {
                    if (l.includes('wall')) return `<div style="color: #ff5555;">${l}</div>`;
                    if (l.includes('victim')) return `<div style="color: #50fa7b;">${l}</div>`;
                    return `<div style="color: #8f8f9d;">${l}</div>`;
                }).join('');

                let vicDiv = document.getElementById('victims');
                if (state.victims.length === 0) {
                    vicDiv.innerHTML = '<div style="color: #44475a;">No victims detected yet</div>';
                } else {
                    vicDiv.innerHTML = state.victims.slice().reverse().map(v => 
                        `<div class="vic-item">🚨 Type <strong>${v.label}</strong> at cell [${v.pos}]</div>`
                    ).join('');
                }
            }
            
            setInterval(() => fetch('/api/state').then(r => r.json()).then(draw), 300);
        </script>
    </body>
    </html>
    ''')

if __name__ == '__main__':
    t = threading.Thread(target=udp_listener, daemon=True)
    t.start()
    app.run(host='0.0.0.0', port=5001)