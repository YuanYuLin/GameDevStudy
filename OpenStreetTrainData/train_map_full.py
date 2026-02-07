import json
import math
import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
from matplotlib.patches import Patch

# === 關鍵新套件：用於自動避讓文字 ===
try:
    from adjustText import adjust_text
    print("成功匯入 adjustText模組，將啟用文字避讓功能。")
except ImportError:
    print("警告：找不到 adjustText 模組。")
    print("文字將會重疊。請執行 'pip install adjustText' 以獲得最佳效果。")
    adjust_text = None

# ==========================================
# 1. 設定區 (Configuration)
# ==========================================
INPUT_FILENAME = 'train_and_track_geoV1.geojson'
# 改存成 PNG，放大看才不會糊掉
OUTPUT_IMAGE = 'grid_map_high_res.png'

# 網格設定：建議設小一點 (例如 100m)，這樣車站比較容易分開在不同格子
TILE_SIZE = 100

CENTER_LON = 121.5173
CENTER_LAT = 25.0479

# ==========================================
# 2. 工具類別 (與之前相同)
# ==========================================
class GridMapper:
    def __init__(self, center_lon, center_lat, tile_size):
        self.center_lon = center_lon
        self.center_lat = center_lat
        self.tile_size = tile_size
        self.R = 6371000

    def to_grid(self, lon, lat):
        lat_rad = math.radians(lat)
        lon_rad = math.radians(lon)
        c_lat_rad = math.radians(self.center_lat)
        c_lon_rad = math.radians(self.center_lon)
        meters_x = self.R * (lon_rad - c_lon_rad) * math.cos(c_lat_rad)
        meters_y = self.R * (lat_rad - c_lat_rad)
        gx = int(round(meters_x / self.tile_size))
        gy = int(round(meters_y / self.tile_size))
        return (gx, gy)

def generate_simulation_data():
    # 模擬一個密集的場景來測試文字避讓
    print("--- 提示：使用密集模擬資料測試文字重疊 ---")
    stations = [
        ("台北車站", 121.5173, 25.0479),
        ("京站", 121.5185, 25.0490), # 離台北車站很近
        ("北門站", 121.5110, 25.0495),
        ("中山站", 121.5200, 25.0530),
        ("善導寺站", 121.5230, 25.0450),
        ("板橋車站", 121.4630, 25.0140),
        ("南港車站", 121.6050, 25.0520)
    ]
    features = []
    for name, lon, lat in stations:
        features.append({
            "type": "Feature",
            "properties": {"name": name},
            "geometry": {"type": "Point", "coordinates": [lon, lat]}
        })
    # 加一條軌道串起來
    track_coords = [[s[1], s[2]] for s in stations]
    features.append({"type": "Feature", "geometry": {"type": "LineString", "coordinates": track_coords}})
    return {"type": "FeatureCollection", "features": features}

# ==========================================
# 3. 主程式流程
# ==========================================
def main():
    # A. 讀取與座標轉換
    if os.path.exists(INPUT_FILENAME):
        print(f"讀取 {INPUT_FILENAME} ...")
        with open(INPUT_FILENAME, 'r', encoding='utf-8') as f:
            geojson_data = json.load(f)
    else:
        geojson_data = generate_simulation_data()

    mapper = GridMapper(CENTER_LON, CENTER_LAT, TILE_SIZE)
    game_map = {}
    print(f"正在轉換座標 (Grid Size: {TILE_SIZE}m)...")
    
    for feature in geojson_data.get('features', []):
        geom = feature.get('geometry')
        props = feature.get('properties', {})
        operator = props.get('operator', '未知')

        if operator != '國營臺灣鐵路股份有限公司': continue
        if not geom: continue
        name = props.get('name', 'Unknown')
        g_type = geom['type']
        coords = geom['coordinates']

        if g_type == 'Point':
            gx, gy = mapper.to_grid(coords[0], coords[1])
            game_map[(gx, gy)] = {"type": 2, "name": name}
        elif g_type == 'LineString':
            for pt in coords:
                gx, gy = mapper.to_grid(pt[0], pt[1])
                if (gx, gy) not in game_map:
                    game_map[(gx, gy)] = {"type": 1, "name": "軌道"}

    if not game_map: return

    # B. 建立矩陣
    xs = [pos[0] for pos in game_map.keys()]
    ys = [pos[1] for pos in game_map.keys()]
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    width = max_x - min_x + 1
    height = max_y - min_y + 1
    
    grid_matrix = np.zeros((height, width), dtype=int)
    labels_to_draw = [] # 準備收集要畫的文字資料

    for (gx, gy), data in game_map.items():
        array_x = gx - min_x
        array_y = gy - min_y
        grid_matrix[array_y, array_x] = data['type']
        if data['type'] == 2:
            # 收集車站文字：(陣列X, 陣列Y, 站名)
            labels_to_draw.append((array_x, array_y, data['name']))

    # ==========================================
    # C. 視覺化 (改進重點)
    # ==========================================
    print("正在準備繪圖...")
    plt.rcParams['font.sans-serif'] = [
    'Microsoft JhengHei',   # Windows 優先
    'Noto Sans CJK JP', 
    'Noto Sans CJK TC',     # Linux 優先 (Google)
    'WenQuanYi Micro Hei',  # Linux 備用
    'SimHei',               # Windows 簡體備用
    'Arial Unicode MS'      # Mac 備用
    ]

    plt.rcParams['axes.unicode_minus'] = False
    
    # 1. 建立更大的畫布，讓文字有更多空間伸展
    fig, ax = plt.subplots(figsize=(16, 12))
    
    cmap = ListedColormap(['white', '#87CEEB', '#FF6347']) # 天藍、番茄紅
    im = ax.imshow(grid_matrix, origin='lower', cmap=cmap, interpolation='nearest')
    
    # 設定座標軸 (略，與之前相同，為了簡潔省略部分美化代碼，重點在文字)
    ax.set_xticks(np.arange(-0.5, width, 1), minor=True)
    ax.set_yticks(np.arange(-0.5, height, 1), minor=True)
    ax.grid(which='minor', color='gray', linestyle='-', linewidth=0.5, alpha=0.3)

    # --- 關鍵改進：繪製並自動調整文字 ---
    texts = [] # 用來收集所有的文字物件
    print(f"正在建立 {len(labels_to_draw)} 個文字標籤...")

    for lx, ly, name in labels_to_draw:
        # 先用標準方式畫出文字，並收集回傳的物件 t
        t = ax.text(lx, ly, name, color='black', ha='center', va='center', 
                    fontweight='bold', fontsize=10,
                    # 加個白色背景讓文字更清楚
                    bbox=dict(facecolor='white', alpha=0.7, edgecolor='#FF6347', boxstyle='round,pad=0.2'))
        texts.append(t)

    # 呼叫 adjust_text 開始魔法般的排版
    if adjust_text:
        print("啟動自動避讓演算法 (這可能需要幾秒鐘，視車站數量而定)...")
        # force_text: 控制推開文字的力道
        # arrowprops: 如果文字被推太遠，畫一條線連回原點
        adjust_text(texts, 
                    force_text=(0.3, 1.0), # (水平推力, 垂直推力)
                    expand_points=(1.2, 1.2), # 將資料點視為更大的範圍來避讓
                    arrowprops=dict(arrowstyle='-', color='gray', lw=1, alpha=0.8)
                   )
        print("文字排版完成！")

    # 設定標題與圖例
    ax.set_title(f"遊戲網格地圖 (Grid Size: {TILE_SIZE}m) - 自動文字避讓", fontsize=16)
    legend_elements = [Patch(facecolor='#FF6347', label='車站'), Patch(facecolor='#87CEEB', label='軌道')]
    ax.legend(handles=legend_elements, loc='upper left', bbox_to_anchor=(1, 1))
    plt.tight_layout()
    
    # --- 輸出選擇 1: 高解析度存檔 (方便放大檢視) ---
    # dpi=300: 設定超高解析度
    # format='png': 使用無損格式
    plt.savefig(OUTPUT_IMAGE, dpi=300, bbox_inches='tight', format='png')
    print(f"\n高解析度圖檔已儲存至: {os.path.abspath(OUTPUT_IMAGE)}")
    print("您可以打開該 PNG 檔案並無限放大檢視細節。")

    # --- 輸出選擇 2: 互動式視窗 (最推薦) ---
    print("\n即將開啟互動式視窗。")
    print("【操作說明】")
    print("1. 使用滑鼠滾輪可縮放地圖。")
    print("2. 點擊工具列上的「放大鏡圖示」可框選放大。")
    print("3. 點擊「十字圖示」可拖曳平移地圖。")
    plt.show()

if __name__ == "__main__":
    main()