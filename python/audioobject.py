from typeguard import typechecked
from audioprofile import AudioProfile

__all__ = ["AudioObject"]

class AudioObject:

    @typechecked
    def __init__(self, audio_profile: AudioProfile):
        self._audio = audio_profile

    @property
    def audio(self): return self._audio

    @audio.setter
    @typechecked
    def audio(self, new_audio: AudioProfile): self._audio = new_audio