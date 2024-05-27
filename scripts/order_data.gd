class_name OrderData 

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

static var items: Dictionary = {
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

class ProcureTimer:
	var started = false
	var totalReset = 1 # number of seconds
	var total = 0
	var on_complete_cb: Callable
	
	func start() -> ProcureTimer: 
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

var procure_timer = ProcureTimer.new()

func _process(delta):
	if procure_timer.started:
		procure_timer.pass_time(delta)

func update_state(new_state: State):
	state = new_state
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
