extends Node3D

var rotate_speed = 2.0

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
# 每一幀 Godot 都會呼叫這個函數
	# Input.get_axis 會回傳 -1, 0, 或 1
	# 預設 ui_left 是左鍵/A鍵，ui_right 是右鍵/D鍵
	var input = Input.get_axis("ui_left", "ui_right")
	
	if input != 0:
		# 繞著 Y 軸旋轉
		# 這裡用的是 Euler Angles
		rotation.y += input * rotate_speed * delta
