extends Node2D

@export var action_label_scene: PackedScene
@export var order_queue_item_scene: PackedScene
@export var order_queue_top_spacer_scene: PackedScene

@onready var screen_container: GridContainer =  %GridContainer
@onready var order_queue_grid = %OrderQueueGrid

@onready var timer: Timer = $Timer

var screens: Array[Screen] = []
var queue_dirty: bool =  true
var queue_slots: Array[OrderData] = [
	null,
	# null,
	# null,
	# null,
	# null,
	# null,
	# null,
	# null,
	# null,
]


# this cant be Array[Screen] because godot deletes the inner type idk
func get_active_screens() -> Array:
	return screens.filter(func(screen): return screen.is_active())

func remove_completed_screens():
	screens = screens.filter(func(screen): return !screen.is_complete())

func mark_screens_if_dirty():
	var active_screens = get_active_screens() 
	if active_screens.size() == 0: return 
	
	# mark all dirty if the user switched from keyboard to gamepad (or vice versa) 
	var input_switched = InputSwitchHandler.instance().changed_since_last_input()
	if !input_switched: return
	for screen in active_screens: screen.is_dirty = input_switched

func process_screen_inputs(_event):
	var active_screens = get_active_screens() 
	if active_screens.size() == 0: return 
	
	var screen = active_screens[0]
	if screen == null: return 
	
	var action = screen.active_action
	if !action.is_active(): return
	if !screen.is_active_action(action): return
		
	if (Input.is_key_pressed(action.data.key) 
	or Input.is_action_just_pressed(action.data.gamepad)):
		print("action ", action.data.name, action.data.gamepad)
		action.action(screen)
		action.is_complete = true #TODO should this live somewhere else? 
		get_viewport().set_input_as_handled() # consume the input 

func process_queue_input(_event):
	var index_triggered = -1
	
	# TODO fix for controller
	for key in range(KEY_1, KEY_9):
		var index = key - KEY_1
		if !Input.is_key_pressed(key): continue
		index_triggered = index
		break
	
	if index_triggered == -1: return
	if queue_slots[index_triggered] == null: return
	
	var order = queue_slots[index_triggered]
	if order.last_ran_state == order.state: return

	# TODO show clock
	if order.timeAtState < order.get_ready_time_for_state(): return

	screens.append(order.open_next_screen())
	queue_dirty = true

	return 

func _unhandled_input(event):
	mark_screens_if_dirty()
	process_queue_input(event)
	ActionManager.instance().reset_count()
	InputSwitchHandler.instance().update_if_switched_input_type(event)
	process_screen_inputs(event)
	
func create_action_button(screen: Screen, action: ActionListener) -> Control :
	ActionManager.instance().assign_input_if_missing(action)

	var label = action_label_scene.instantiate().with_data(
		action.data
		.set_active(screen.is_active_action(action))
	)
	
	var wrapper = MarginContainer.new()
	wrapper.add_child(label)
	wrapper.add_theme_constant_override("theme_override_constants/margin_left", 100)
	return wrapper
		
func render_actions():
	var children = screen_container.get_children();
	
	var actives = get_active_screens()
	if actives.size() == 0:
		for child in children:
			child.queue_free()
		screen_container.get_parent().get_parent().hide()
		return
	screen_container.get_parent().get_parent().show()
		
	var active_screen = actives[0]
	if !active_screen.is_dirty: return

	active_screen.is_dirty = false
	
	for child in children:
		child.queue_free()
	
	screen_container.columns = active_screen.num_items_per_row

	active_screen.config.for_all(
		func (action): 
			screen_container.add_child(
				create_action_button(active_screen, action)
			),
	)


func gen_order_queue_item(index: int, order_in_slot: OrderData) :
	var image: Texture2D = null;
	if order_in_slot != null: 
		print("rendering slot %s with state %s" % [order_in_slot.queue_position, order_in_slot.state])
		match order_in_slot.state:
			OrderData.State.New:
				image = load("res://graphics/order_new.png")
			OrderData.State.Procure:
				image = load("res://graphics/order_procure.png")
			OrderData.State.Pack:
				image = load("res://graphics/order_pack.png")
			OrderData.State.Ship:
				image = load("res://graphics/order_ship.png")
				
	var order_ui_item = order_queue_item_scene.instantiate().with_data(
		"order_queue_item_%s" % [index],
		str(index+1),
		image,
	)
	return order_ui_item

func render_order_queue():
	var children = order_queue_grid.get_children()
	
	if !queue_dirty: return
	queue_dirty = false
	print("Rendering Order Queue")
	
	for child in children:
		child.queue_free()
	
	var new_children = []
	# TODO make global for max orders
	for i in range(0, queue_slots.size()):
		var slot = queue_slots[i]
		new_children.append(gen_order_queue_item(i, slot))
		
	order_queue_grid.add_child(order_queue_top_spacer_scene.instantiate())
	
	for child in new_children:
		order_queue_grid.add_child(child)
		
func _ready():
	timer.start()

func run_order_processing(delta):
	for order in queue_slots:
		if order == null: continue
		if order.should_clear_slot(): 
			queue_slots[order.queue_position] = null
			queue_dirty = true
		order._process(delta)

func _process(delta):
	remove_completed_screens()
	run_order_processing(delta)
	render_order_queue()
	render_actions()
	pass

func find_empty_queue_slot() -> int:
	for i in range(0, queue_slots.size()):
		if queue_slots[i] == null:
			return i
	return -1
	
func _on_timer_timeout():
	var next_slot = find_empty_queue_slot()
	if next_slot == -1:
		# print("No empty queue slots, you lose buddy")
		return
		
	var order = OrderData.make_new_order()
	order.queue_position = next_slot
	queue_slots[next_slot] = order
	queue_dirty = true

	print("Added order into queue at slot ", next_slot)
	
	if find_empty_queue_slot() != -1:
		timer.start()
