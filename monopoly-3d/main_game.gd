extends Node3D

# 透過 @export 讓我們可以在編輯器裡把場景中的物件拖進來
@export var player: Node3D # 這裡類型是 Node3D，實際上是 Player
@export var start_node: MapNode

@onready var game_dialog: AcceptDialog = $CanvasLayer/GameDialog

func _ready():
	# 遊戲開始，初始化玩家位置
	# 為了安全，我們檢查一下是否設定了變數
	if player and start_node:
		# 呼叫 Player 腳本裡的函數 (Duck Typing)
		# 如果你想要強型別，可以在上面寫 @export var player: Player
		player.set_start_node(start_node)
		# 連接訊號：當 Player 發出 interaction_required 時，執行 _on_player_interaction
		player.interaction_required.connect(_on_player_interaction)

func _on_player_interaction(title: String, body: String, data: GameTileData):
	# 設定 UI 文字
	game_dialog.title = title
	game_dialog.dialog_text = body
	
	# 根據資料類型，我們可以改變按鈕文字
	# 比如：如果是土地，將 "OK" 改成 "購買" (這裡先做最簡單的展示)
	game_dialog.ok_button_text = "確定"
	
	# 彈出視窗
	game_dialog.popup_centered()
	
func _input(event):
	# 監聽空白鍵
	if event.is_action_pressed("ui_accept"): # 預設是 Space 或 Enter
		if player and !player.is_moving:
			# 隨機擲骰子 1-6
			var steps = randi_range(1, 6)
			print("Dice rolled: ", steps)
			player.move_steps(steps)
