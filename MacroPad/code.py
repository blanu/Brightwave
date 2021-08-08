from adafruit_macropad import MacroPad
from rainbowio import colorwheel

version = "0.1"
cc = 23

macropad = MacroPad(rotation=0)  # create the macropad object, rotate orientation

macropad.pixels.brightness = 0.1
colors = [110, 140, 180, 65, 85, 90, 0, 15, 40, 235, 195, 185]
for index, color in enumerate(colors):
    macropad.pixels[index] = colorwheel(colors[index])

midi_values = [0, 16, 8]  # bank, cc value, pitch
# Chromatic scale starting with C3 as bottom left keyswitch (or use any notes you like)
midi_notes = [
    57, 58, 59,
    54, 55, 56,
    51, 52, 53,
    48, 49, 50
    ]

# --- Display text setup ---
text_lines = macropad.display_text("Brightwave Harp v%s" % (version))
text_lines.show()

last_knob_pos = macropad.encoder  # store knob position state
while True:
    key_event = macropad.keys.events.get()  # check for key press or release
    if key_event:
        if key_event.pressed:
            key = key_event.key_number
            macropad.midi.send(macropad.NoteOn(midi_notes[key], 120))  # send midi noteon
            macropad.pixels[key] = colorwheel(170)
            text_lines[1].text = "NoteOn:{}".format(midi_notes[key])

        if key_event.released:
            key = key_event.key_number
            macropad.midi.send(macropad.NoteOff(midi_notes[key], 0))
            macropad.pixels[key] = colorwheel(colors[key])
            text_lines[1].text = "NoteOff:{}".format(midi_notes[key])

    macropad.encoder_switch_debounced.update()  # check the knob switch for press or release
    if macropad.encoder_switch_debounced.pressed:
        text_lines[0].text = "Encoder knob pressed"
        macropad.red_led = macropad.encoder_switch
        text_lines[1].text = " "  # clear the note line

    if macropad.encoder_switch_debounced.released:
        macropad.red_led = macropad.encoder_switch

    if last_knob_pos is not macropad.encoder:  # knob has been turned
        knob_pos = macropad.encoder  # read encoder
        knob_delta = knob_pos - last_knob_pos  # compute knob_delta since last read
        last_knob_pos = knob_pos  # save new reading

        last_knob_pos = macropad.encoder
