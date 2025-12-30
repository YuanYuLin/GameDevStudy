# res://Resources/TileData/TileData.gd
class_name GameTileData extends Resource

@export_group("Basic Info")
@export var tile_name: String = "未命名"
@export_multiline var description: String = "這是一個普通的格子"
@export var icon: Texture2D

# 虛擬函數，類似 C++ virtual void on_land(player)
# 雖然 Resource 主要是數據，但 Godot 允許在裡面寫輕量邏輯
func on_player_landed(player_node: Node3D):
	print("玩家到達：", tile_name)
