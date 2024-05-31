class_name ShipScreen extends SequentialScreen

class BoxFold extends SequentialAction:
	func _init():
		super(
			ActionData.new()
			.set_name("fold")
			.set_key(KEY_F)
		)

class BoxTape extends SequentialAction:
	func _init():
		super(
			ActionData.new()
			.set_name("tape")
			.set_key(KEY_T)
		)
		
class BoxLabel extends SequentialAction:
	func _init():
		super(
			ActionData.new()
			.set_name("label")
			.set_key(KEY_L)
		)
		
func _init():
	super("ship", ActionGroup.new().add_actions([
		BoxFold.new(),
		BoxTape.new(),
		BoxLabel.new(),
		CompleteAction.new()
	]))
	next(true)
	
