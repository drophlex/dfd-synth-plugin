# Dark Funk Durcal Synt - KeyLab Essential (mk1) control surface for Ableton Live 11 / 12.
#
# With the Dark Funk Durcal Synt device selected (blue hand):
#   9 knobs (encoders 1-8)  -> the plugin's current SOUND page (8 parameters); PART < > (bank buttons) changes page
#   9 faders                -> Drums, Acid, Bass, Lead levels, DJ Filter, Echo, Reverb, Drive; master fader = Master
#   8 pads                  -> the plugin's 8 performance pads (mutes, fill, stutter, build/drop, next phrase)
#   LCD (2 x 16)            -> the page name, then the name and value of whatever you move
# With any other device: knobs -> that device's parameters (8 per page), faders -> track volumes, pads -> clip launch.
# Transport, loop, metronome, record, undo and the track arrows work everywhere.
#
# KeyLab Essential DAW protocol (from Live's own KeyLab_Essential script): encoders CC 16-23 relative on channel 1,
# faders = pitch bend on channels 1-8 (master on 9), pads = notes 36-43 on channel 11, buttons = notes on channel 1.
from __future__ import absolute_import, print_function, unicode_literals

import Live
from _Framework.ControlSurface import ControlSurface
from _Framework.InputControlElement import MIDI_CC_TYPE, MIDI_NOTE_TYPE, MIDI_PB_TYPE
from _Framework.EncoderElement import EncoderElement
from _Framework.SliderElement import SliderElement
from _Framework.ButtonElement import ButtonElement
from _Framework.DeviceComponent import DeviceComponent
from _Framework.MixerComponent import MixerComponent
from _Framework.TransportComponent import TransportComponent

HEADER = (0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42)
LIVE_MODE_ON = HEADER + (0x02, 0x00, 0x40, 0x11, 0x7F, 0xF7)
LIVE_MODE_OFF = HEADER + (0x02, 0x00, 0x40, 0x11, 0x00, 0xF7)
DAW_PRESET = HEADER + (0x05, 0x02, 0xF7)
LCD_WIDTH = 16

DFD_NAME = 'Dark Funk Durcal'
DFD_BANKS = ('PERFORM', 'ACID', 'DRUMS', 'BASS', 'LEAD', 'FX', 'GROOVE', 'MIX & PLAY')
# parameter names (as the plugin reports them) for faders 1-8 and the master fader
DFD_FADERS = ('Drums Level', 'Acid Level', 'Bass Level', 'Lead Level', 'DJ Filter', 'Echo', 'Reverb', 'Drive')
DFD_MASTER = 'Master'
PAD_RGB = ((100, 90, 70), (100, 90, 70), (100, 90, 70), (100, 90, 70), (127, 20, 10), (127, 40, 10), (127, 10, 0), (127, 60, 30))


def _ascii(text):
    out = []
    for ch in (text or ''):
        o = ord(ch)
        if 32 <= o < 127:
            out.append(o)
        else:  # accents etc: strip to the closest plain letter
            out.append({'á': 97, 'é': 101, 'í': 105, 'ó': 111, 'ú': 117, 'ñ': 110,
                        'Á': 65, 'É': 69, 'Í': 73, 'Ó': 79, 'Ú': 85, 'Ñ': 78}.get(ch, 32))
    return out[:LCD_WIDTH]


class DfdKeyLab(ControlSurface):

    def __init__(self, c_instance):
        ControlSurface.__init__(self, c_instance)
        self._dfd_mode = False
        self._bound_faders = []
        with self.component_guard():
            self._create_controls()
            self._create_components()
        self.song().view.add_selected_track_listener(self._on_selection_changed)
        self._on_selection_changed()
        self.schedule_message(2, self._hello)

    # ------------------------------------------------------------------ hardware
    def _hello(self):
        self._send_midi(LIVE_MODE_ON)
        self._send_midi(DAW_PRESET)
        self._lcd('DARK FUNK DURCAL', 'SYNT  READY')

    def disconnect(self):
        try:
            self._lcd('', '')
            self._send_midi(LIVE_MODE_OFF)
        except Exception:
            pass
        try:
            self.song().view.remove_selected_track_listener(self._on_selection_changed)
        except Exception:
            pass
        ControlSurface.disconnect(self)

    def _lcd(self, line1, line2):
        msg = HEADER + (0x04, 0x00, 0x60, 0x01) + tuple(_ascii(line1)) + (0x00, 0x02) + tuple(_ascii(line2)) + (0x00, 0xF7)
        self._send_midi(msg)

    def _pads_light(self, on):
        for i in range(8):
            rgb = PAD_RGB[i] if on else (0, 0, 0)
            self._send_midi(HEADER + (0x02, 0x00, 0x16, 112 + i) + rgb + (0xF7,))

    def _create_controls(self):
        self._encoders = [EncoderElement(MIDI_CC_TYPE, 0, 16 + i, Live.MidiMap.MapMode.relative_signed_bit) for i in range(8)]
        for i, enc in enumerate(self._encoders):
            enc.add_value_listener(self._on_encoder, identify_sender=True)
        self._faders = [SliderElement(MIDI_PB_TYPE, i, 0) for i in range(8)]
        for f in self._faders:
            f.add_value_listener(self._on_fader, identify_sender=True)
        self._master = SliderElement(MIDI_PB_TYPE, 8, 0)
        self._master.add_value_listener(self._on_fader, identify_sender=True)

        def btn(note):
            return ButtonElement(True, MIDI_NOTE_TYPE, 0, note)
        self._play, self._stop, self._rec = btn(94), btn(93), btn(95)
        self._loop, self._metro, self._undo = btn(86), btn(89), btn(81)
        self._rwd, self._ffw = btn(91), btn(92)
        self._bank_left, self._bank_right = btn(46), btn(47)
        self._prev_track, self._next_track = btn(98), btn(99)
        self._undo.add_value_listener(self._on_undo)
        self._prev_track.add_value_listener(self._on_prev_track)
        self._next_track.add_value_listener(self._on_next_track)
        self._bank_left.add_value_listener(self._on_bank_button)
        self._bank_right.add_value_listener(self._on_bank_button)

    def _create_components(self):
        self._transport = TransportComponent()
        self._transport.set_play_button(self._play)
        self._transport.set_stop_button(self._stop)
        self._transport.set_record_button(self._rec)
        self._transport.set_loop_button(self._loop)
        self._transport.set_metronome_button(self._metro)
        self._transport.set_seek_buttons(self._ffw, self._rwd)

        self._mixer = MixerComponent(8)
        self._mixer.master_strip().set_volume_control(self._master)
        for i in range(8):
            self._mixer.channel_strip(i).set_volume_control(self._faders[i])

        self._device = DeviceComponent()
        self._device.set_parameter_controls(tuple(self._encoders))
        self._device.set_bank_nav_buttons(self._bank_left, self._bank_right)
        self.set_device_component(self._device)

    # ------------------------------------------------------------------ the selected device decides the mode
    def _on_selection_changed(self):
        self.schedule_message(1, self._update_mode)

    def _selected_device(self):
        track = self.song().view.selected_track
        dev = track.view.selected_device if track else None
        if dev is None and track and len(track.devices):
            dev = track.devices[0]
        return dev

    def _is_dfd(self, dev):
        if dev is None:
            return False
        try:
            if DFD_NAME.lower() in (dev.name or '').lower() or DFD_NAME.lower() in (getattr(dev, 'class_display_name', '') or '').lower():
                return True
            names = set(p.name for p in dev.parameters)
            return 'Acid Cutoff' in names and 'DJ Filter' in names
        except Exception:
            return False

    def _update_mode(self):
        dev = self._selected_device()
        dfd = self._is_dfd(dev)
        with self.component_guard():
            self._release_faders()
            if dfd:
                # faders and the master fader drive the plugin; the mixer lets go of them
                for i in range(8):
                    self._mixer.channel_strip(i).set_volume_control(None)
                self._mixer.master_strip().set_volume_control(None)
                by_name = dict((p.name, p) for p in dev.parameters)
                for i, name in enumerate(DFD_FADERS):
                    p = by_name.get(name)
                    if p is not None:
                        self._faders[i].connect_to(p)
                        self._bound_faders.append(self._faders[i])
                p = by_name.get(DFD_MASTER)
                if p is not None:
                    self._master.connect_to(p)
                    self._bound_faders.append(self._master)
            else:
                for i in range(8):
                    self._mixer.channel_strip(i).set_volume_control(self._faders[i])
                self._mixer.master_strip().set_volume_control(self._master)
        if dfd != self._dfd_mode:
            self._dfd_mode = dfd
            self._pads_light(dfd)
        if dfd:
            self._lcd('DARK FUNK DURCAL', self._bank_title())
        elif dev is not None:
            self._lcd(dev.name, 'KNOBS = DEVICE')

    def _release_faders(self):
        for f in self._bound_faders:
            try:
                f.release_parameter()
            except Exception:
                pass
        self._bound_faders = []

    def _bank_title(self):
        try:
            b = self._device._bank_index
        except Exception:
            b = 0
        name = DFD_BANKS[b] if 0 <= b < len(DFD_BANKS) else 'PAGE %d' % (b + 1)
        return '%d %s' % (b + 1, name)

    # ------------------------------------------------------------------ feedback on the LCD
    def _show_param(self, param):
        if param is None:
            return
        try:
            value = param.str_for_value(param.value)
        except Exception:
            value = str(param)
        self._lcd(param.name, value)

    def _on_encoder(self, value, sender):
        self.schedule_message(1, lambda: self._show_param(sender.mapped_parameter()))

    def _on_fader(self, value, sender):
        self.schedule_message(1, lambda: self._show_param(sender.mapped_parameter()))

    def _on_bank_button(self, value):
        if value:
            self.schedule_message(1, lambda: self._lcd('DARK FUNK DURCAL' if self._dfd_mode else 'DEVICE PAGE', self._bank_title()))

    # ------------------------------------------------------------------ buttons
    def _on_undo(self, value):
        if value and self.song().can_undo:
            self.song().undo()

    def _step_track(self, delta):
        tracks = list(self.song().visible_tracks)
        cur = self.song().view.selected_track
        if cur in tracks:
            i = max(0, min(len(tracks) - 1, tracks.index(cur) + delta))
            self.song().view.selected_track = tracks[i]

    def _on_prev_track(self, value):
        if value:
            self._step_track(-1)

    def _on_next_track(self, value):
        if value:
            self._step_track(1)
