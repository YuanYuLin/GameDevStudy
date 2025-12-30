class_name MapNode extends Node3D

enum Type { START, LAND, CHANCE, JAIL }
@export var node_type: Type = Type.LAND

# 這裡持有 Resource 的指標
@export var data: GameTileData

# 這是你的 Linked List 指標
# @export 讓它可以在編輯器 UI 中被賦值
@export var next_node: MapNode

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
