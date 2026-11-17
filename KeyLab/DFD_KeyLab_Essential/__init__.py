# Dark Funk Durcal Synt - Ableton Live control surface for the Arturia KeyLab Essential 49 / 61 / 88.
# Created at Polaris Sound Lab, Merida, Yucatan, Mexico.
#
# get_capabilities: Live's auto-detection. When the KeyLab Essential is plugged in, Live can pick this script for it by
# itself (Settings > Link, Tempo & MIDI then shows DFD_KeyLab_Essential with the KeyLab's ports, Track and Remote on).
from __future__ import absolute_import
from .dfd_keylab import DfdKeyLab

try:
    from ableton.v2.control_surface.capabilities import (CONTROLLER_ID_KEY, NOTES_CC, PORTS_KEY, REMOTE, SCRIPT,
                                                         controller_id, inport, outport)

    def get_capabilities():
        return {CONTROLLER_ID_KEY: controller_id(vendor_id=7285, product_ids=[588, 589, 590],
                                                 model_name=['KeyLab Essential 49', 'KeyLab Essential 61',
                                                             'KeyLab Essential 88']),
                PORTS_KEY: [inport(props=[NOTES_CC, SCRIPT, REMOTE]), outport(props=[SCRIPT])]}
except ImportError:   # (an older Live: no auto-detection, choose it by hand)
    pass


def create_instance(c_instance):
    return DfdKeyLab(c_instance)
