import json
import numpy as np
import matplotlib.pyplot as plt
import os

# --- 1. 輔助函數 ---
def extract_lon_lat(coords):
    xs, ys = [], []
    for pt in coords:
        xs.append(pt[0])
        ys.append(pt[1])
    return xs, ys

# --- 2. 新增功能：繪製「站到站」的直接箭頭 ---
def plot_station_connections(ax, station_map, ordered_list, color='magenta'):
    """
    station_map: 字典 {'站名': (經度, 緯度)}
    ordered_list: 串列 ['站名A', '站名B', '站名C'...] (定義順序)
    """
    print(f"開始繪製前後站關係箭頭 (共 {len(ordered_list)-1} 段)...")
    
    q_x, q_y, q_u, q_v = [], [], [], []

    for i in range(len(ordered_list) - 1):
        start_name = ordered_list[i]
        end_name = ordered_list[i+1]
        
        # 確保站名在我們的地圖資料中找得到座標
        if start_name in station_map and end_name in station_map:
            start_coords = station_map[start_name]
            end_coords = station_map[end_name]
            
            # 起點座標
            q_x.append(start_coords[0])
            q_y.append(start_coords[1])
            
            # 計算向量 (終點 - 起點)
            q_u.append(end_coords[0] - start_coords[0])
            q_v.append(end_coords[1] - start_coords[1])
        else:
            print(f"警告：找不到 {start_name} 或 {end_name} 的座標資料，跳過繪製。")

    # 統一繪製箭頭
    # 為了讓這種「邏輯箭頭」更明顯，我們把它畫得比軌道粗一點，顏色鮮豔一點
    if q_x:
        # zorder 設定高一點，確保畫在軌道和建築物上面
        ax.quiver(q_x, q_y, q_u, q_v, color=color, 
                  angles='xy', scale_units='xy', scale=1, 
                  width=0.008, headwidth=5, headlength=6, alpha=0.9, zorder=10, label='前後站關係')

# --- 3. 主程式 ---
filename = 'train_and_track_geoV2.geojson'

# 模擬完整一點的資料
if not os.path.exists(filename):
    print("提示：找不到檔案，生成模擬資料...")
    geojson_data = {
        "type": "FeatureCollection",
        "features": [
            # 模擬一條彎曲的軌道 (LineString)
            {
                "type": "Feature", "properties": {"name": "西部幹線軌道", "operator": "國營臺灣鐵路股份有限公司"},
                "geometry": {"type": "LineString", "coordinates": [
                    [121.740, 25.132], [121.720, 25.110], [121.680, 25.080], # 基隆往南
                    [121.605, 25.052], [121.550, 25.050], [121.517, 25.047], # 南港到台北
                    [121.480, 25.030], [121.463, 25.014], [121.400, 25.000], # 台北到板橋
                    [121.313, 24.989] # 桃園
                ]}
            },
            # 車站點 (Point)
            {"type": "Feature", "properties": {"name": "基隆車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.740, 25.132]}},
            {"type": "Feature", "properties": {"name": "南港車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.605, 25.052]}},
            {"type": "Feature", "properties": {"name": "台北車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.517, 25.047]}},
            {"type": "Feature", "properties": {"name": "板橋車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.463, 25.014]}},
            {"type": "Feature", "properties": {"name": "桃園車站", "operator": "國營臺灣鐵路股份有限公司"}, "geometry": {"type": "Point", "coordinates": [121.313, 24.989]}}
        ]
    }
else:
    with open(filename, 'r', encoding='utf-8') as f:
        geojson_data = json.load(f)

# 設定繪圖
# 建立一個字體優先順序清單
# Matplotlib 會依序尋找，找到第一個有的就用
plt.rcParams['font.sans-serif'] = [
    'Microsoft JhengHei',   # Windows 優先
    'Noto Sans CJK JP', 
    'Noto Sans CJK TC',     # Linux 優先 (Google)
    'WenQuanYi Micro Hei',  # Linux 備用
    'SimHei',               # Windows 簡體備用
    'Arial Unicode MS'      # Mac 備用
]
plt.rcParams['axes.unicode_minus'] = False
fig, ax = plt.subplots(figsize=(12, 12))

# --- 關鍵步驟：定義我們想要呈現的順序 ---
# 在真實應用中，這個順序可能來自資料庫或 OSM 的 Relation
my_route_order = ["基隆車站", "南港車站", "台北車站", "板橋車站", "桃園車站"]

# 用來儲存「站名 -> 座標」的對照表
station_coords_map = {}

print("開始解析 GeoJSON 並繪製底圖...")
scatter_data = {'x': [], 'y': [], 'names': []}

for feature in geojson_data.get('features', []):
    geom = feature.get('geometry')
    props = feature.get('properties', {})
    name = props.get('name', '未命名')
    operator = props.get('operator', '未知')

    if operator != '國營臺灣鐵路股份有限公司': continue
    if not geom: continue
    g_type = geom['type']
    coords = geom['coordinates']

    # 繪製軌道 (LineString) - 這次只畫線，不畫密集的箭頭了，以免混淆
    if g_type in ['LineString', 'MultiLineString']:
        coords_list = coords if g_type == 'LineString' else [c for lines in coords for c in lines]
        for line_c in (coords if g_type == 'MultiLineString' else [coords]):
            xs, ys = extract_lon_lat(line_c)
            # zorder 設低一點，當作背景
            ax.plot(xs, ys, c='royalblue', linewidth=2, alpha=0.4, zorder=1, label='實體軌道')

    # 處理車站 (Point) - 收集座標並建立對照表
    elif g_type == 'Point':
        lon, lat = coords[0], coords[1]
        scatter_data['x'].append(lon)
        scatter_data['y'].append(lat)
        scatter_data['names'].append(name)
        # 重要：將站名與座標存入字典
        station_coords_map[name] = (lon, lat)

    # 處理 Polygon... (略，與之前相同)

# --- 繪製車站點 ---
if scatter_data['x']:
    ax.scatter(scatter_data['x'], scatter_data['y'], 
               c='red', s=80, marker='o', label='車站位置', zorder=5, edgecolor='white')
    for i, txt in enumerate(scatter_data['names']):
        ax.text(scatter_data['x'][i], scatter_data['y'][i], 
                txt, fontsize=10, ha='right', va='bottom', zorder=6, fontweight='bold')

# --- 核心呼叫：繪製前後站關係箭頭 ---
# 這會在圖上畫出又粗又明顯的箭頭，直接連接我們指定的車站順序
plot_station_connections(ax, station_coords_map, my_route_order, color='magenta')

# --- 最終設定 ---
ax.set_xlabel('經度 (Longitude)')
ax.set_ylabel('緯度 (Latitude)')
ax.set_title('台灣鐵路：前後站關係示意圖 (2D)')
ax.set_aspect('equal') 
ax.grid(True, linestyle='--', alpha=0.5)

# 處理圖例
handles, labels = ax.get_legend_handles_labels()
by_label = dict(zip(labels, handles))
ax.legend(by_label.values(), by_label.keys(), loc='best')

plt.show()