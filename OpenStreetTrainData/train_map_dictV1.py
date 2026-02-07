import json
import math
import os

# --- 1. 設定遊戲參數 ---
# 設定一格代表多少公尺 (決定地圖的精細度)
TILE_SIZE = 100  # 100公尺 = 1格

# 設定地圖中心 (台北車站)
CENTER_LON = 121.5173
CENTER_LAT = 25.0479

# --- 2. 座標轉換工具類別 ---
class GridMapper:
    def __init__(self, center_lon, center_lat, tile_size):
        self.center_lon = center_lon
        self.center_lat = center_lat
        self.tile_size = tile_size
        self.R = 6371000 # 地球半徑 (m)

    def to_grid(self, lon, lat):
        """
        將 (經度, 緯度) -> 轉換為 -> (Grid_X, Grid_Y)
        回傳值為整數 tuple，例如 (0, 0) 或 (-5, 10)
        """
        lat_rad = math.radians(lat)
        lon_rad = math.radians(lon)
        c_lat_rad = math.radians(self.center_lat)
        c_lon_rad = math.radians(self.center_lon)

        # 計算與中心的距離 (公尺)
        meters_x = self.R * (lon_rad - c_lon_rad) * math.cos(c_lat_rad)
        meters_y = self.R * (lat_rad - c_lat_rad)

        # 轉為網格座標 (四捨五入取整)
        gx = int(round(meters_x / self.tile_size))
        gy = int(round(meters_y / self.tile_size))
        
        return (gx, gy)

# --- 3. 主程式 ---
filename = 'train_and_track_geoV2.geojson'

# 讀取檔案
if not os.path.exists(filename):
    print(f"找不到 {filename}，請確認檔案路徑。")
    # 這裡放一個模擬資料，證明 (0,0) 的邏輯是正確的
    geojson_data = {
        "features": [
            {"properties": {"name": "台北車站"}, "geometry": {"type": "Point", "coordinates": [121.5173, 25.0479]}}, # 中心
            {"properties": {"name": "板橋車站"}, "geometry": {"type": "Point", "coordinates": [121.4630, 25.0140]}}, # 西南 (負數)
        ]
    }
else:
    with open(filename, 'r', encoding='utf-8') as f:
        geojson_data = json.load(f)

# 初始化轉換器
mapper = GridMapper(CENTER_LON, CENTER_LAT, TILE_SIZE)

# ==========================================
# 核心結構：使用 Dictionary 存地圖
# Key: (x, y) tuple -> 支援負數
# Value: 您定義的結構 data
# ==========================================
game_map = {}

print("開始轉換地圖資料...\n")

for feature in geojson_data.get('features', []):
    geom = feature.get('geometry')
    props = feature.get('properties', {})
    operator = props.get('operator', '未知')

    if operator != '國營臺灣鐵路股份有限公司': continue
    if not geom: continue
    
    name = props.get('name', 'Unknown')
    g_type = geom['type']
    coords = geom['coordinates']

    # --- 處理 Point (車站) ---
    if g_type == 'Point':
        # 取得網格座標 (Tuple Key)
        grid_pos = mapper.to_grid(coords[0], coords[1]) # e.g., (0, 0)
        
        # 建立資料物件
        obj_data = {
            "name": name,
            "type": "Station",
            "position": {"x": grid_pos[0], "y": grid_pos[1]} # 雖然 Key 已經有座標，但物件內存一份方便取用
        }
        
        # 存入字典
        game_map[grid_pos] = obj_data

    # --- 處理 LineString (軌道) ---
    elif g_type == 'LineString':
        # 軌道是一連串的點，我們把每個轉折點都存進去
        for idx, pt in enumerate(coords):
            grid_pos = mapper.to_grid(pt[0], pt[1])
            
            # 如果這個位置已經有車站了，我們不要覆蓋它 (車站優先於軌道)
            if grid_pos in game_map and game_map[grid_pos]['type'] == 'Station':
                continue
            
            obj_data = {
                "name": f"{name}_node_{idx}",
                "type": "Track",
                "position": {"x": grid_pos[0], "y": grid_pos[1]}
            }
            game_map[grid_pos] = obj_data

# --- 4. 驗證結果 ---

print(f"轉換完成！地圖中共有 {len(game_map)} 個物件。\n")

# A. 測試存取中心點 (台北車站)
center_key = (0, 0)
if center_key in game_map:
    print(f"座標 {center_key} 發現物件: {game_map[center_key]['name']}")
else:
    print(f"座標 {center_key} 是空的 (可能是座標有些微誤差，被歸到 (0,1) 之類的)")

# B. 測試存取負數座標 (板橋車站)
# 透過程式反查板橋在哪裡，來證明可以用負數 Key
'''
banqiao_pos = None
for pos, data in game_map.items():
    if data['name'] == '板橋車站':
        banqiao_pos = pos
        break

if banqiao_pos:
    print(f"座標 {banqiao_pos} 發現物件: {game_map[banqiao_pos]['name']}")
    print(f"-> 驗證 X 軸是否為負數: {banqiao_pos[0] < 0}") 
'''
# --- 5. 遊戲邏輯示範：查詢周圍 ---
print("\n--- 遊戲邏輯測試: 查詢 (0,0) 的右邊一格 ---")
target_pos = (1, 0) # 往右一格
if target_pos in game_map:
    print(f"右邊一格有東西: {game_map[target_pos]['name']}")
else:
    print("右邊一格是空的 (空地)")

# --- 6. 轉存 JSON (注意) ---
# JSON 的 Key 必須是字串，不能是 Tuple，所以輸出時要轉一下
json_output = {}
for coords_tuple, data in game_map.items():
    # 將 (-5, 10) 轉成字串 "-5,10"
    str_key = f"{coords_tuple[0]},{coords_tuple[1]}"
    json_output[str_key] = data

# print(json.dumps(json_output, indent=2, ensure_ascii=False))

# 輸出成 JSON 檔案
with open('stations_track.json', 'w', encoding='utf-8') as f:
    json.dump(json_output, f, ensure_ascii=False, indent=4)