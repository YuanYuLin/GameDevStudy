import json
import numpy as np
import matplotlib.pyplot as plt
import os

# --- 1. 輔助函數：直接提取經緯度 (不需轉 XYZ) ---
def extract_lon_lat(coords):
    # 輸入: [[lon, lat], [lon, lat], ...] 或單個 [lon, lat]
    # 輸出: xs(經度), ys(緯度)
    xs, ys = [], []
    for pt in coords:
        xs.append(pt[0]) # Longitude (X)
        ys.append(pt[1]) # Latitude (Y)
    return xs, ys

# --- 2. 核心功能：2D 箭頭繪製 ---
def plot_directed_arrows_2d(ax, xs, ys, color='orange', density=1):
    """
    在 2D 平面上繪製箭頭
    """
    total_points = len(xs)
    
    # 準備 quiver 需要的陣列
    q_x, q_y, q_u, q_v = [], [], [], []
    
    for i in range(0, total_points - 1, density):
        start_x, start_y = xs[i], ys[i]
        end_x, end_y = xs[i+1], ys[i+1]
        
        # 計算向量 (Vector)
        u = end_x - start_x
        v = end_y - start_y
        
        q_x.append(start_x)
        q_y.append(start_y)
        q_u.append(u)
        q_v.append(v)
    
    # 畫箭頭
    # angles='xy', scale_units='xy', scale=1 是讓箭頭長度跟隨座標軸單位的關鍵設定
    if q_x:
        ax.quiver(q_x, q_y, q_u, q_v, color=color, 
                  angles='xy', scale_units='xy', scale=1, 
                  width=0.005, headwidth=4, headlength=5, alpha=0.8)

# --- 3. 主程式 ---
filename = 'train_and_track_geoV2.geojson'

# 模擬資料 (若讀不到檔案時使用)
if not os.path.exists(filename):
    print("提示：找不到檔案，生成模擬資料...")
    geojson_data = {
        "type": "FeatureCollection",
        "features": [
            {
                "type": "Feature", "properties": {"name": "西部幹線模擬路徑", "operator": "國營臺灣鐵路股份有限公司"},
                "geometry": {
                    "type": "LineString",
                    "coordinates": [
                        [121.517, 25.047], # 台北
                        [121.463, 25.014], # 板橋
                        [121.313, 24.989], # 桃園
                        [121.220, 24.950]  # 中壢
                    ]
                }
            },
            {"type": "Feature", "properties": {"name": "台北車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.517, 25.047]}},
            {"type": "Feature", "properties": {"name": "板橋車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.463, 25.014]}},
            {"type": "Feature", "properties": {"name": "桃園車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.313, 24.989]}}
        ]
    }
else:
    with open(filename, 'r', encoding='utf-8') as f:
        geojson_data = json.load(f)

# 設定繪圖 (使用標準 2D 圖表)
plt.rcParams['font.sans-serif'] = ['Microsoft JhengHei'] 
plt.rcParams['axes.unicode_minus'] = False

fig, ax = plt.subplots(figsize=(10, 10)) # 建立 2D 畫布

print("開始繪製 2D 地圖與動線...")

scatter_data = {'x': [], 'y': [], 'names': []}

for feature in geojson_data.get('features', []):
    geom = feature.get('geometry')
    props = feature.get('properties', {})
    name = props.get('name', '未命名')
    operator = props.get('operator', '未知')

    # 依照您的邏輯過濾
    if operator != '國營臺灣鐵路股份有限公司':
        continue
    
    if not geom: continue
    g_type = geom['type']
    coords = geom['coordinates']

    # === LineString (路徑 + 箭頭) ===
    if g_type == 'LineString':
        xs, ys = extract_lon_lat(coords)
        # 1. 畫線
        ax.plot(xs, ys, c='blue', linewidth=1, alpha=0.6, label='軌道' if '軌道' not in [l.get_label() for l in ax.get_lines()] else "")
        # 2. 畫箭頭
        plot_directed_arrows_2d(ax, xs, ys, color='darkorange', density=1)
        
    elif g_type == 'MultiLineString':
        for line_coords in coords:
            xs, ys = extract_lon_lat(line_coords)
            ax.plot(xs, ys, c='blue', linewidth=1, alpha=0.6)
            plot_directed_arrows_2d(ax, xs, ys, color='darkorange', density=1)

    # === Point (車站) ===
    elif g_type == 'Point':
        # Point 的 coords 是 [lon, lat]，為了 extract_lon_lat 變成 [[lon, lat]]
        xs, ys = extract_lon_lat([coords]) 
        scatter_data['x'].extend(xs)
        scatter_data['y'].extend(ys)
        scatter_data['names'].append(name)

    elif g_type == 'MultiPoint':
        xs, ys = extract_lon_lat(coords)
        scatter_data['x'].extend(xs)
        scatter_data['y'].extend(ys)
        for _ in range(len(xs)): scatter_data['names'].append(name)

    # === Polygon (建築物) ===
    elif g_type == 'Polygon':
        for ring in coords:
            xs, ys = extract_lon_lat(ring)
            ax.fill(xs, ys, c='green', alpha=0.3) # 2D 可以用 fill 填色，效果更好
            ax.plot(xs, ys, c='green', linewidth=1) # 邊框

    elif g_type == 'MultiPolygon':
        for poly in coords:
            for ring in poly:
                xs, ys = extract_lon_lat(ring)
                ax.fill(xs, ys, c='green', alpha=0.3)
                ax.plot(xs, ys, c='green', linewidth=1)

# 最後畫出所有車站點
if scatter_data['x']:
    ax.scatter(scatter_data['x'], scatter_data['y'], 
               c='red', s=50, marker='o', label='車站', zorder=5)
    
    for i, txt in enumerate(scatter_data['names']):
        # 在 2D 圖上稍微偏移文字位置，避免蓋住點
        ax.text(scatter_data['x'][i], scatter_data['y'][i], 
                txt, fontsize=9, ha='right', va='bottom')

# 設定座標軸標籤
ax.set_xlabel('經度 (Longitude)')
ax.set_ylabel('緯度 (Latitude)')
ax.set_title('台灣鐵路路網圖 (2D)')

# === 關鍵設定：讓比例尺正確 ===
# 因為經緯度 1 度在赤道跟在台灣的距離不同，不設 equal 會變形
ax.set_aspect('equal') 
ax.grid(True, linestyle='--', alpha=0.5)

# 處理圖例 (移除重複標籤)
handles, labels = ax.get_legend_handles_labels()
by_label = dict(zip(labels, handles))
ax.legend(by_label.values(), by_label.keys())

plt.show()