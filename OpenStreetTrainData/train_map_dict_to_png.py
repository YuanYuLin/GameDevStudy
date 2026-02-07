import matplotlib.pyplot as plt
import numpy as np
import os
import json
from matplotlib.colors import ListedColormap
from matplotlib.patches import Patch

# 1. 您的資料 (您可以隨時把這段換成讀取 JSON 檔)
data = {
    "-898,-785": {
        "name": "海岸線_node_3",
        "type": "Track",
        "position": {"x": -898, "y": -785}
    },
    "-897,-782": {
        "name": "大甲",
        "type": "Station",
        "position": {"x": -897, "y": -782}
    },
    "-895,-780": {
        "name": "海岸線_node_5",
        "type": "Track",
        "position": {"x": -895, "y": -780}
    }
}
INPUT_FILENAME = 'stations_track.json'
    # A. 讀取與座標轉換
if os.path.exists(INPUT_FILENAME):
    print(f"讀取 {INPUT_FILENAME} ...")
    with open(INPUT_FILENAME, 'r', encoding='utf-8') as f:
        data = json.load(f)


# 2. 解析資料並計算邊界
xs = [info['position']['x'] for info in data.values()]
ys = [info['position']['y'] for info in data.values()]

if not xs:
    print("錯誤：資料為空")
    exit()

min_x, max_x = min(xs), max(xs)
min_y, max_y = min(ys), max(ys)

# 加入緩衝區 (Padding)，讓圖看起來不會太擠
PADDING = 2
width = (max_x - min_x) + 1 + 2 * PADDING
height = (max_y - min_y) + 1 + 2 * PADDING

# 3. 建立網格矩陣
# 0:空地, 1:軌道, 2:車站
grid_matrix = np.zeros((height, width), dtype=int)
labels = []

for key, info in data.items():
    # 座標轉換公式：Array Index = (Real Coord - Min Coord) + Padding
    arr_x = (info['position']['x'] - min_x) + PADDING
    arr_y = (info['position']['y'] - min_y) + PADDING
    
    if info['type'] == 'Station':
        grid_matrix[arr_y, arr_x] = 2
        # labels.append((arr_x, arr_y, info['name']))
    else:
        grid_matrix[arr_y, arr_x] = 1
        # 如果軌道也要顯示名字，可以把下面這行註解打開
        # labels.append((arr_x, arr_y, info['name'])) 

# 4. 繪圖設定
plt.rcParams['font.sans-serif'] = [
    'Microsoft JhengHei',   # Windows 優先
    'Noto Sans CJK JP', 
    'Noto Sans CJK TC',     # Linux 優先 (Google)
    'WenQuanYi Micro Hei',  # Linux 備用
    'SimHei',               # Windows 簡體備用
    'Arial Unicode MS'      # Mac 備用
]
plt.rcParams['axes.unicode_minus'] = False # 確保負號正常顯示

fig, ax = plt.subplots(figsize=(10, 8)) # 畫布大小

# 定義顏色：白底、藍軌道、紅車站
cmap = ListedColormap(['white', '#87CEEB', '#FF6347'])

# 繪製矩陣 (origin='lower' 讓 Y 軸由下往上增長)
ax.imshow(grid_matrix, origin='lower', cmap=cmap, interpolation='nearest')

# 5. 設定座標軸標籤 (顯示真實座標)
# 產生刻度位置 (在 Array Index 上)
xticks = np.arange(0, width, 1)
yticks = np.arange(0, height, 1)

# 產生刻度文字 (轉換回 Real Coordinate)
xticklabels = [str(x - PADDING + min_x) for x in xticks]
yticklabels = [str(y - PADDING + min_y) for y in yticks]

ax.set_xticks(xticks)
ax.set_yticks(yticks)
ax.set_xticklabels(xticklabels, rotation=0) # 若 X 軸太擠可設 rotation=45
ax.set_yticklabels(yticklabels)

# 畫網格線
ax.set_xticks(np.arange(-0.5, width, 1), minor=True)
ax.set_yticks(np.arange(-0.5, height, 1), minor=True)
ax.grid(which='minor', color='white', linestyle='-', linewidth=0.5, alpha=0.3)
ax.tick_params(which='minor', size=0) # 隱藏小刻度線

# 6. 標示文字 (帶有背景框，避免看不清楚)
for lx, ly, name in labels:
    ax.text(lx, ly, name, color='black', ha='center', va='bottom',
            fontweight='bold', fontsize=12,
            bbox=dict(facecolor='white', alpha=0.8, edgecolor='red', boxstyle='round,pad=0.3'))

# 設定標題與圖例
ax.set_title(f"車站與軌道分佈圖\n範圍: X[{min_x}~{max_x}], Y[{min_y}~{max_y}]")
ax.set_xlabel("Grid X (遊戲座標)")
ax.set_ylabel("Grid Y (遊戲座標)")

legend_elements = [Patch(facecolor='#FF6347', label='車站'),
                   Patch(facecolor='#87CEEB', label='軌道'),
                   Patch(facecolor='white', edgecolor='white', label='空地')]
ax.legend(handles=legend_elements, loc='upper left', bbox_to_anchor=(1, 1))

plt.tight_layout()

# 儲存與顯示
plt.savefig('station_map_visual.png', dpi=150)
plt.show()