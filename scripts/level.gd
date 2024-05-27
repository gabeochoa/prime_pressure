extends Node2D

func _unhandled_input(event):
	if event.is_action_pressed("ui_cancel") or Input.is_action_pressed("menu_back"): 
		get_tree().quit()
