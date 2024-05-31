class_name OrderData 

class OrderTimer:
	var started = false
	var totalReset = 1 # number of seconds
	var total = 0
	var on_complete_cb: Callable
	
	func start() -> OrderTimer: 
		started = true
		return self
	
	func on_complete(cb: Callable):
		on_complete_cb = cb
	
	func pass_time(delta):
		total += delta
		#print("Procure timer at %s" % [total / totalReset ])
		if total > totalReset:
			on_complete_cb.call()
			started = false
			total = 0

enum State {
	None,
	New,
	Procure,
	Pack,
	Ship,
	Complete,
}

enum ItemType {
	Shampoo,
	Chips, 
	ToiletPaper,
	Sheets,
	Slippers,
	Popcorn,
	Leggings,
	Soda
} 

static var all_items: Dictionary = {
	ItemType.Shampoo: { "en_name": "Shampoo", "keys": "SHAM" },
	ItemType.Chips: { "en_name": "Chips", "keys": "CHIP" },
	ItemType.ToiletPaper: { "en_name": "Toilet Paper", "keys": "TOIL" },
	ItemType.Sheets: { "en_name": "Sheets", "keys": "SHEE" },
	ItemType.Slippers: { "en_name": "Slippers", "keys": "SLIP" },
	ItemType.Popcorn: { "en_name": "Popcorn", "keys": "CORN" },
	ItemType.Leggings: { "en_name": "Leggings", "keys": "LEGG" },
	ItemType.Soda: { "en_name": "Soda", "keys": "SODA" },
}

class Item:
	var type: ItemType
	var amount: int
	
	func set_type(t: ItemType):
		type = t
		return self
	
	func set_amount(amt: int):
		amount = amt
		return self
	
var queue_position: int = -1
var state: State = State.New
var last_ran_state: State = State.None
var order_items: Array[Item]
var timeAtState: float = 0
var order_timer = OrderTimer.new()

var next_screen = null

func should_clear_slot() -> bool: 
	return state == State.Complete

func get_ready_time_for_state(): 
	match state:
		OrderData.State.New:
			return 0
		OrderData.State.Procure:
			return 3
		OrderData.State.Pack:
			return 3
		OrderData.State.Ship:
			return 0
	# TODO log_warn
	return 0

func get_percentage() -> float :
	print(" time at ", timeAtState)
	if get_ready_time_for_state() == 0: return 0
	return timeAtState / get_ready_time_for_state()

func fetch_next_screen():
	if next_screen != null: return
	match state:
		OrderData.State.New:
			next_screen = (
				ProcureScreen
				.new(self)
				.set_on_complete(func():
					print("procure complete")
					next_screen = null
					)
				)
		OrderData.State.Procure:
			next_screen = (
				BoxScreen.new()
				.set_on_complete(func(): 
					print("box complete")
					next_screen = null
					)	
			)
			pass
		OrderData.State.Pack:
			next_screen = (
				ShipScreen.new()
				.set_on_complete(func(): 
					print("lets go")
					next_screen = null
					)
			)
			pass
		OrderData.State.Ship:
			next_screen = (
				CompleteScreen.new()
				.set_on_complete(func(): 
					next_screen = null
					)
			)
			pass
		OrderData.State.Complete:
			pass
	pass

func _process(delta):
	timeAtState += delta
	fetch_next_screen()
	# print(" time at state ", state, " is " , timeAtState, " delta was ", delta)

func open_next_screen() -> Screen: 
	assert(next_screen != null, "Missing next screen")

	match state: 
		OrderData.State.New:
			update_state(OrderData.State.Procure)
			pass
		OrderData.State.Procure:
			update_state(OrderData.State.Pack)
			pass
		OrderData.State.Pack:
			update_state(OrderData.State.Ship)
			pass
		OrderData.State.Ship:
			update_state(OrderData.State.Complete)
			pass
		OrderData.State.Complete:
			pass

	return next_screen

func update_state(new_state: State):
	# print("update state from ", state, " to " , new_state)
	if new_state != state:  timeAtState = 0
	state = new_state
	timeAtState = 0
	return self


func get_items():
	return order_items
	
func set_items(it: Array[Item]) -> OrderData:
	order_items = it
	return self

static func make_example_order() -> Array[Item]:
	var items = ItemType.keys()
	items.shuffle()
	
	var example_order: Array[Item] = []
	for i in range(0, 3):
		var item_type  = items[i]
		example_order.append(
			Item.new().set_type(ItemType.get(item_type))
		)
	return example_order

static func make_new_order() -> OrderData: 
	return (
		OrderData.new()
		.set_items(OrderData.make_example_order())
		.update_state(State.New)
	)
