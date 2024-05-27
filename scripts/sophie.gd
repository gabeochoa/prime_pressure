extends Node2D


@export var action_label_scene: PackedScene
@export var order_queue_item_scene: PackedScene
@export var order_queue_top_spacer_scene: PackedScene

@onready var screen_container: GridContainer =  %GridContainer
@onready var order_queue_grid = %OrderQueueGrid

@onready var timer: Timer = $Timer

var screens: Array[Screen] = []

# this cant be Array[Screen] because godot deletes the inner type idk
func get_active_screens() -> Array:
	return screens.filter(func(screen): return screen.is_active())
	

func process_screen_inputs(event):
	ActionManager.instance().reset_count()
	
	InputSwitchHandler.instance().update_if_switched_input_type(event)
	
	var active_screens = get_active_screens()
	if active_screens.size() == 0: return 
	
	var screen = active_screens[0]
	if screen == null: return 
	
	var action = screen.active_action
	if action.is_active():
		if InputSwitchHandler.instance().changed_since_last_input():
			screen.is_dirty = true
		if !screen.is_active_action(action):
			return
			
		if Input.is_key_pressed(action.data.key) or Input.is_action_just_pressed(action.data.gamepad):
			print("action ", action.data.name, action.data.gamepad)
			action.action(screen)
			action.is_complete = true #TODO should this live somewhere else? 
			get_viewport().set_input_as_handled()
			return
	
func process_queue_input(event):
	var index_triggered = -1
	
	for key in range(KEY_1, KEY_9):
		var index = key - KEY_1
		if Input.is_key_pressed(key):
			index_triggered = index
			break
	
	if index_triggered == -1: return
	if queue_slots[index_triggered] == null: return
	
	var order = queue_slots[index_triggered]
	if order.last_ran_state == order.state: return
	
	match order.state:
		OrderData.State.New:
			order.last_ran_state = OrderData.State.New
			order.state = OrderData.State.Procure
			#
			var on_procure_complete = (
				func(): 
					order.last_ran_state = order.state
					order.state = OrderData.State.Pack
					#
					screens.append(
						BoxScreen.new()
						.set_on_complete(func(): 
							order.last_ran_state = order.state
							order.state = OrderData.State.Ship
							screens.append(ShipScreen.new().set_on_complete(func(): 
								order.last_ran_state = order.state
								order.state = OrderData.State.Complete
								print("lets go")))
							)
						)
					queue_dirty = true
			)
			
			var on_complete_fn = (
				func():
					print("on complete fn")
					order.procure_timer.start().on_complete(on_procure_complete)
			)
			screens.append(
				ProcureScreen
				.new(order)
				.set_on_complete(on_complete_fn)
			)
			queue_dirty = true
			pass
		OrderData.State.Procure:
			pass
		OrderData.State.Pack:
			pass
		OrderData.State.Ship:
			pass
	

func _unhandled_input(event):
	process_queue_input(event)
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
	if !active_screen.is_dirty:
		return
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


var queue_dirty: bool =  true
var queue_slots: Array[OrderData] = [
	null,
	null,
	null,
	null,
	null,
	null,
	null,
	null,
	null,
]

func gen_order_queue_item(index: int, order_in_slot: OrderData) :
	var image: Texture2D = null;
	if order_in_slot != null: 
		print("rendering slot with state ", order_in_slot.state)
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

func run_order_timers(delta):
	for order in queue_slots:
		if order == null: continue
		order._process(delta)

func _process(delta):
	run_order_timers(delta)
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
		print("No empty queue slots, you lose buddy")
		return
		
	var order = OrderData.make_new_order()
	order.queue_position = next_slot
	queue_slots[next_slot] = order
	queue_dirty = true
	print("Added order into queue at slot ", next_slot)
