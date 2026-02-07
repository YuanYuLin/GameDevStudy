import json
import os

# --- 1. 準備資料 (模擬您儲存的 JSON) ---
# 包含負數座標與中心點 (0,0)

filename = 'stations_track.json'

# 讀取檔案
if not os.path.exists(filename):
    print(f"找不到 {filename}，請確認檔案路徑。")
    # 這裡放一個模擬資料，證明 (0,0) 的邏輯是正確的
    json_data = {
    "0,0": {"name": "台北車站", "type": "Station"},
    "1,0": {"name": "軌道A", "type": "Track"},
    "2,0": {"name": "軌道B", "type": "Track"},
    "3,0": {"name": "南港車站", "type": "Station"},
    "-1,0": {"name": "軌道C", "type": "Track"},
    "-2,1": {"name": "軌道D", "type": "Track"}, # 往上拐彎
    "-2,2": {"name": "軌道E", "type": "Track"},
    "-3,2": {"name": "板橋車站", "type": "Station"},
    "0,1": {"name": "淡水線軌道", "type": "Track"}, # 台北車站往上
    "0,-1": {"name": "新店線軌道", "type": "Track"} # 台北車站往下
    }
else:
    with open(filename, 'r', encoding='utf-8') as f:
        json_data = json.load(f)

# --- 2. 設定顯示符號 ---
# 您可以隨意更改這些符號
SYMBOL_STATION = "S"  # 車站
SYMBOL_TRACK   = "+"  # 軌道
SYMBOL_EMPTY   = "."  # 空地
SYMBOL_CENTER  = "X"  # 特別標記 (0,0)

def print_ascii_map(data):
    # A. 解析座標並找出邊界
    parsed_coords = {}
    xs, ys = [], []

    for key, info in data.items():
        gx, gy = map(int, key.split(','))
        parsed_coords[(gx, gy)] = info
        xs.append(gx)
        ys.append(gy)

    # 為了讓地圖周圍留點邊框，範圍多加 1 格
    min_x, max_x = min(xs) - 1, max(xs) + 1
    min_y, max_y = min(ys) - 1, max(ys) + 1

    print(f"\n[地圖範圍]: X({min_x}~{max_x}), Y({min_y}~{max_y})\n")

    # B. 開始繪圖 (注意：Y 軸要從大到小印，因為 Terminal 是由上往下印)
    # y_range: max_y, max_y-1, ..., min_y
    for y in range(max_y, min_y - 1, -1):
        
        # 1. 印出 Y 軸座標標籤 (左側)
        # 使用 f-string {:3d} 確保對齊 (佔3格寬)
        row_str = f"{y:3d} | " 
        
        for x in range(min_x, max_x + 1):
            
            # 2. 判斷這個 (x, y) 有沒有東西
            if (x, y) in parsed_coords:
                obj = parsed_coords[(x, y)]
                if obj['type'] == 'Station':
                    # 如果是 (0,0) 且是車站，可以給特殊顏色或符號(這裡簡單處理)
                    symbol = SYMBOL_STATION
                else:
                    symbol = SYMBOL_TRACK
            else:
                # 3. 處理空地
                # 如果是 (0,0) 原點但沒東西，標示一下方便辨識
                if x == 0 and y == 0:
                    symbol = SYMBOL_CENTER 
                else:
                    symbol = SYMBOL_EMPTY
            
            # 將符號加入這一行 (加空格是為了讓長寬比例好看一點)
            row_str += f"{symbol} "
        
        # 這一行組裝完畢，印出
        print(row_str)

    # C. 印出 X 軸座標標籤 (底部)
    # 分隔線
    print("    " + "-" * ((max_x - min_x + 1) * 2))
    
    # X 軸數字 (因為數字可能很寬，我們直的印比較難，這裡簡化處理)
    # 這裡只印出 X 軸的刻度
    x_axis_str = "      " # 對齊左邊的 "  y | "
    for x in range(min_x, max_x + 1):
        # 簡單印個位數，或者每隔 5 格印一次數字，不然會擠在一起
        if abs(x) < 10:
             x_axis_str += f"{x} " # 個位數佔1格+空格
        else:
             x_axis_str += f"{x%10} " # 超過兩位數只印個位數簡化
            
    print(x_axis_str)
    
    # D. 印出圖例
    print("\n[圖例]")
    print(f"{SYMBOL_STATION} : 車站 (Station)")
    print(f"{SYMBOL_TRACK} : 軌道 (Track)")
    print(f"{SYMBOL_EMPTY} : 空地 (Empty)")
    print(f"X : 原點 (0,0) 若為空")

    # E. 印出車站列表對照
    print("\n[車站座標清單]")
    for (gx, gy), info in parsed_coords.items():
        if info['type'] == 'Station':
            print(f"{info['name']}: ({gx}, {gy})")

# 執行
print_ascii_map(json_data)