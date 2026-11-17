# Dark Funk Durcal Synt - Ableton Live control surface for the Arturia KeyLab Essential (mk1) 49 / 61 / 88.
# Created at Polaris Sound Lab, Merida, Yucatan, Mexico.
from __future__ import absolute_import
from .dfd_keylab import DfdKeyLab


def create_instance(c_instance):
    return DfdKeyLab(c_instance)
