class_name BoxScreen extends SequentialScreen

class BoxUnfold extends SequentialAction:
	func _init():
		super(
			ActionData.new()
			.set_name("unfold")
			.set_key(KEY_U)
		)

class BoxBottom extends SequentialAction:
	func _init():
		super(
			ActionData.new()
			.set_name("bottom")
			.set_key(KEY_B)
		)

class BoxTape extends SequentialAction:
	func _init():
		super(
			ActionData.new()
			.set_name("tape")
			.set_key(KEY_T)
		)
		
func _init():
	super("box", ActionGroup.new().add_actions([
		BoxUnfold.new(),
		BoxBottom.new(),
		BoxTape.new(),
		CompleteAction.new()
	]))
	next(true)
	
