# res://Resources/TileData/LandData.gd
class_name GameLandData extends GameTileData

@export_group("Economy")
@export var base_price: int = 1000
@export var base_toll: int = 200 # 過路費
@export var owner_id: int = -1 # -1 代表銀行，0, 1, 2 代表玩家 ID

func on_player_landed(player_node: Node3D):
	super.on_player_landed(player_node) # 呼叫父類別
	print("這是一塊土地，價格：$%d，過路費：$%d" % [base_price, base_toll])
	# 這裡未來會接上 UI 訊號
