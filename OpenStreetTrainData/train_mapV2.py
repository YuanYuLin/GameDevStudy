import json
import numpy as np
import matplotlib.pyplot as plt
import os

# --- 1. 定義座標轉換函數 ---
def geo_to_cartesian(lon, lat, alt=0):
    R = 6371  # 地球半徑 (km)
    lat_rad = np.radians(lat)
    lon_rad = np.radians(lon)
    r_total = R + (alt / 1000)
    
    x = r_total * np.cos(lat_rad) * np.cos(lon_rad)
    y = r_total * np.cos(lat_rad) * np.sin(lon_rad)
    z = r_total * np.sin(lat_rad)
    return x, y, z

# --- 2. 輔助函數：將座標列表轉為 XYZ ---
def convert_coords_list(coords):
    xs, ys, zs = [], [], []
    for pt in coords:
        # 確保格式正確 (有些資料可能只有 [lon, lat])
        alt = pt[2] if len(pt) > 2 else 0
        x, y, z = geo_to_cartesian(pt[0], pt[1], alt)
        xs.append(x)
        ys.append(y)
        zs.append(z)
    return xs, ys, zs

# --- 3. 核心功能：畫出帶有方向的箭頭 (Quiver) ---
def plot_directed_arrows(ax, xs, ys, zs, color='orange', density=1):
    """
    在路徑上繪製箭頭
    density: 控制箭頭密度，每幾個點畫一個箭頭 (避免箭頭太密變成海膽)
    """
    total_points = len(xs)
    
    for i in range(0, total_points - 1, density):
        # 起點
        start_x, start_y, start_z = xs[i], ys[i], zs[i]
        # 終點 (下一個點)
        end_x, end_y, end_z = xs[i+1], ys[i+1], zs[i+1]
        
        # 計算向量 (Vector)
        u = end_x - start_x
        v = end_y - start_y
        w = end_z - start_z
        
        # 畫箭頭
        # length: 控制箭頭長短, normalize: 讓箭頭大小一致
        ax.quiver(start_x, start_y, start_z, u, v, w, 
                  color=color, length=0.5, normalize=True, arrow_length_ratio=0.4)

# --- 4. 主程式 ---
filename = 'train_and_track_geoV2.geojson'

# 模擬資料 (若讀不到檔案時使用)
if not os.path.exists(filename):
    print("提示：找不到檔案，生成模擬資料...")
    geojson_data = {
        "type": "FeatureCollection",
        "features": [
            # 模擬一條軌道 (LineString)，這代表了前後順序
            {
                "type": "Feature", "properties": {"name": "西部幹線模擬路徑"},
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
            # 模擬車站 (Point)
            {"type": "Feature", "properties": {"name": "台北車站"}, "geometry": {"type": "Point", "coordinates": [121.517, 25.047]}},
            {"type": "Feature", "properties": {"name": "板橋車站"}, "geometry": {"type": "Point", "coordinates": [121.463, 25.014]}},
            {"type": "Feature", "properties": {"name": "桃園車站"}, "geometry": {"type": "Point", "coordinates": [121.313, 24.989]}}
        ]
    }
else:
    with open(filename, 'r', encoding='utf-8') as f:
        geojson_data = json.load(f)

# 設定繪圖
plt.rcParams['font.sans-serif'] = ['Microsoft JhengHei'] 
plt.rcParams['axes.unicode_minus'] = False
fig = plt.figure(figsize=(12, 10))
ax = fig.add_subplot(111, projection='3d')

print("開始繪製地圖與動線...")

# 收集散佈點以便最後一起畫
scatter_data = {'x': [], 'y': [], 'z': [], 'names': []}

for feature in geojson_data.get('features', []):
    geom = feature.get('geometry')
    props = feature.get('properties', {})
    name = props.get('name', '未命名')
    operator = props.get('operator', '未知')

    if operator != '國營臺灣鐵路股份有限公司' :
        continue
    
    if not geom: continue
    g_type = geom['type']
    coords = geom['coordinates']

    # === 處理線條 (LineString) -> 視為路徑，畫箭頭 ===
    if g_type == 'LineString':
        xs, ys, zs = convert_coords_list(coords)
        # 1. 畫線 (軌道)
        ax.plot(xs, ys, zs, c='blue', linewidth=1, alpha=0.6, label='軌道')
        # 2. 畫箭頭 (方向) - 這裡設定每隔 1 個點畫一個箭頭
        plot_directed_arrows(ax, xs, ys, zs, color='darkorange', density=1)
        
    elif g_type == 'MultiLineString':
        for line_coords in coords:
            xs, ys, zs = convert_coords_list(line_coords)
            ax.plot(xs, ys, zs, c='blue', linewidth=1, alpha=0.6)
            plot_directed_arrows(ax, xs, ys, zs, color='darkorange', density=1)

    # === 處理點 (Point) -> 視為車站 ===
    elif g_type == 'Point':
        xs, ys, zs = convert_coords_list([coords]) # 轉成 list 方便統一處理
        scatter_data['x'].extend(xs)
        scatter_data['y'].extend(ys)
        scatter_data['z'].extend(zs)
        scatter_data['names'].append(name)

    elif g_type == 'MultiPoint':
        xs, ys, zs = convert_coords_list(coords)
        scatter_data['x'].extend(xs)
        scatter_data['y'].extend(ys)
        scatter_data['z'].extend(zs)
        for _ in range(len(xs)): scatter_data['names'].append(name) # 簡單處理名稱

    # === 處理面 (Polygon) -> 視為建築物 ===
    elif g_type == 'Polygon':
        for ring in coords:
            xs, ys, zs = convert_coords_list(ring)
            ax.plot(xs, ys, zs, c='green', linewidth=1, alpha=0.5)

    elif g_type == 'MultiPolygon':
        for poly in coords:
            for ring in poly:
                xs, ys, zs = convert_coords_list(ring)
                ax.plot(xs, ys, zs, c='green', linewidth=1, alpha=0.5)

# 最後畫出所有車站點
if scatter_data['x']:
    ax.scatter(scatter_data['x'], scatter_data['y'], scatter_data['z'], 
               c='red', s=80, marker='o', label='車站/地點')
    
    for i, txt in enumerate(scatter_data['names']):
        ax.text(scatter_data['x'][i], scatter_data['y'][i], scatter_data['z'][i], 
                txt, fontsize=9, zorder=10)

# 設定視圖
ax.set_xlabel('X (km)')
ax.set_ylabel('Y (km)')
ax.set_zlabel('Z (km)')
ax.set_title(f'3D 軌道地圖與方向 (箭頭代表行進順序)')

# 為了避免 legend 重複，手動處理一下
handles, labels = plt.gca().get_legend_handles_labels()
by_label = dict(zip(labels, handles))
plt.legend(by_label.values(), by_label.keys())

plt.show()