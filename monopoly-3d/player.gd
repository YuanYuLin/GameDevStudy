extends Node3D

# 存儲當前所在的節點 (類似 C++ 的 MapNode* current_node)
var current_node: MapNode

# 移動中旗標 (Mutex/Lock 的概念，防止連點)
var is_moving: bool = false

# 移動速度 (秒/格)
@export var step_duration: float = 0.3
@export var jump_height: float = 0.5

# 初始化：把玩家直接瞬移到某個節點
func set_start_node(node: MapNode):
	current_node = node
	global_position = node.global_position

signal interaction_required(title: String, body: String, data: GameTileData)
# 核心功能：擲骰子移動
func move_steps(steps: int):
	if is_moving: return # 簡單的 Lock
	if current_node == null:
		printerr("Error: Player is not on the map!")
		return

	is_moving = true
	print("Player moving %d steps..." % steps)

	# 迴圈執行每一步
	for i in range(steps):
		# 1. 檢查是否有下一格 (Null Check)
		if current_node.next_node == null:
			break
		
		# 2. 更新邏輯指針
		current_node = current_node.next_node
		
		# 3. 執行「跳躍」動畫 (這是 Coroutine)
		await _animate_jump_to(current_node.global_position)
	
	# 移動結束
	is_moving = false
	print("Movement finished. Landed on: ", current_node.name)
	_handle_landing()

func _handle_landing():
	if current_node.data:
		# 1. 執行 Resource 內定義的 Log 邏輯
		current_node.data.on_player_landed(self)
		
		# 2. 發送訊號給 UI 層 (解耦：Player 不直接操作 UI)
		# 我們根據資料類型組裝訊息
		var title = current_node.data.tile_name
		var body = current_node.data.description
		
		# 如果是土地，額外顯示價格
		if current_node.data is GameLandData:
			var land = current_node.data as GameLandData
			body += "\n\n價格: $%d\n過路費: $%d" % [land.base_price, land.base_toll]
			
		emit_signal("interaction_required", title, body, current_node.data)
	else:
		print("到達了一個沒有資料的荒地")
		
# 私有函數：處理單步動畫
func _animate_jump_to(target_pos: Vector3) -> void:
	# 建立 Tween (類似建立一個暫時的 Animation Task)
	var tween = create_tween()
	
	# 設定並行 (Parallel): 我們希望 XZ 平移和 Y 跳躍同時發生
	tween.set_parallel(true)
	
	# 1. 水平移動 (XZ Plane)
	# tween_property(物件, 屬性名, 目標值, 時間)
	tween.tween_property(self, "global_position:x", target_pos.x, step_duration)
	tween.tween_property(self, "global_position:z", target_pos.z, step_duration)
	
	# 2. 垂直跳躍 (Y Axis) - 使用拋物線效果
	# 先上
	tween.tween_property(self, "global_position:y", target_pos.y + jump_height, step_duration / 2.0)\
		.set_ease(Tween.EASE_OUT).set_trans(Tween.TRANS_QUAD)
	
	# 後下 (chain() 會等待上面的 tween 跑完才跑這個，破壞 parallel 規則，但我們用 delay 來模擬)
	# 更好的寫法是分兩個 tween 或者用 Sequence，這裡為了簡單展示：
	# 我們用一個 trick: 直接對 Y 軸做兩個連續的 tween 比較麻煩
	# 讓我們用更簡單的 "Sequence" 方式重寫 Y 軸邏輯:
	
	# ---- 修正後的 Tween 邏輯 ----
	# 為了讓跳躍看起來自然，我們其實可以建立兩個 Tween，或者利用 set_parallel(false) 
	# 但最簡單的方法是用 Godot 的 "Sub-tweening"。
	# 這裡我們換個寫法，只 tween 位置，不手動搞拋物線，改用 Easing 來模擬 "滑動" 
	# (如果你堅持要跳躍，下面的代碼會稍微複雜一點，我們暫時先做 "滑順滑動")
	
	# 覆蓋上面的 Y 軸邏輯，改為滑順移動:
	tween.tween_property(self, "global_position:y", target_pos.y, step_duration)
	
	# ---- 關鍵指令 ----
	# await 等待 tween 的 "finished" 信號
	# 這行代碼會讓這個函數暫停執行，直到動畫跑完，將控制權交還給 Main Loop
	await tween.finished
