import json
from collections import defaultdict

def geojson_to_tree(input_file, output_file):
    # 1. 讀取 GeoJSON 檔案
    with open(input_file, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # 2. 定義樹狀結構容器
    # 結構目標: tree['營運者']['車站名'] = {經緯度, 其他屬性}
    tree_structure = defaultdict(dict)

    # 3. 遍歷每一個 Feature (車站)
    for feature in data['features']:
        props = feature['properties']
        geometry = feature['geometry']
        
        # 取得關鍵欄位，若沒有則標示為 'Unknown'
        # OSM 的 operator 標籤有時會混亂，這裡做簡單正規化
        raw_operator = props.get('operator', 'Unknown')
        name = props.get('name', 'Unnamed Station')
        
        # 簡單清理 operator 名稱 (例如將 TR, TRA 統一)
        operator = clean_operator_name(raw_operator)

        if operator != '國營臺灣鐵路股份有限公司' :
            continue

        # 4. 建構節點資料
        station_data = {
            "name_en": props.get('name:en', ''),
            "location": {
                "lat": geometry['coordinates'][1], # GeoJSON 是 [lon, lat]
                "lon": geometry['coordinates'][0]
            },
            # 您可以加入更多 OSM 屬性，例如輪椅是否有障礙
            "wheelchair": props.get('wheelchair', 'unknown')
        }

        # 5. 插入樹狀結構
        # 這裡示範兩層結構: Root -> Operator -> Station Name -> Data
        tree_structure[operator][name] = station_data

    # 6. 輸出成 JSON 檔案
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(tree_structure, f, ensure_ascii=False, indent=4)
    
    print(f"轉換完成！已依據營運單位分類，輸出至 {output_file}")

def clean_operator_name(op_name):
    """簡單的名稱正規化輔助函式"""
    op_name = op_name.lower()
    if 'taiwan railways' in op_name or 'tra' in op_name:
        return 'Taiwan Railways (台鐵)'
    elif 'high speed' in op_name or 'thsr' in op_name:
        return 'THSR (高鐵)'
    elif 'metro' in op_name or 'mrt' in op_name:
        return 'MRT (捷運)'
    else:
        return op_name.title() # 保持原樣但首字大寫

# 執行函式 (請確保您的檔案名稱正確)
# geojson_to_tree('stations.geojson', 'stations_tree.json')
geojson_to_tree('train_and_track_geoV2.geojson', 'stations_tree.json')