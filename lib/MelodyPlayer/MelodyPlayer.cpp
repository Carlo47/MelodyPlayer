/**
 * Class        MelodyPlayer.cpp
 * Author       2021-08-28 Charles Geiser (https://www.dodeka.ch)
 *
 * Purpose      Implements a nonblocking melody player with the ledc pwm subsystem of the ESP32.
 *              You have to call playMelody() in the main loop of your program. 
 * 
 * Board        ESP32 DoIt DevKit V1
 * 
 * Remarks      Uses ledcSetup(channel, frequency, resolution)   to initialize the ledc pwm subsystem
 *                   ledcAttachPin(pin, channel)                 to attach the output pin to the pwm channel
 *                   ledcWrite(channel, dutyCycle)               to set the pwm duty cycle (volume control)
 *                   ledcWriteNote(channel, note, octave)        to set the output frequency
 * 
 * References    
 */
#include "MelodyPlayer.h"

/**
 * Set the volume of the tone in the range 0..511
 * The pulse width of the speaker signal is set
 * from 0 to 50% 
 */
void MelodyPlayer::setVolume(uint32_t volume)
{
    _volume = volume;
};

/**
 * Set the tempo to a predefined tempo
 */
void MelodyPlayer::setTempo(TEMPO tempo)
{
    _tempo = tempo;
}

/**
 * Set the gap between played notes in ms (0..100)
 * 0 means no gap --> legato
 * Default value is 10 ms
 */
void MelodyPlayer::setLegato(uint32_t msNoteGap)
{

    _msNoteGap = (msNoteGap <= 100) ? msNoteGap : 100;
}

/**
 * Set the tempo to n beats per minute
 */
void MelodyPlayer::setTempo(int nBeats)
{
    _tempo = (TEMPO)nBeats;
}

/**
 * Set the melody to be played
 */
void MelodyPlayer::setMelody(musicNote m[], int len)
{
    _melody = m;
    _melodyLength = len;
}

/**
 * Turns the output signal off by
 * setting the pulse width to 0
 */
void MelodyPlayer::mute()
{
    ledcWrite(_channel, 0);
}

/**
 * Set normal playing mode so that the notes
 * of the melody are played  in order
 */
void MelodyPlayer::setNormalMode()
{
    _random = false;
}

/**
 * Set random mode so tha the notes
 * of the melody are played randomly
 */
void MelodyPlayer::setRandomMode()
{
    _random = true;
}

/**
 * Plays a single music note n with the set tempo
 * The duration is given by N_LEN.
 * A musicNote is defined like {NOTE_C, octave, N_LEN::N4}
 */
void MelodyPlayer::playNote(musicNote n)
{
    if (_notePlayed) return; // play the note only once
    if (! _started)
    {
        // Caveat: ledcWriteNote() sets the resolution to 10 bit. 
        //         That's why the volume ranges from 0..511 (0 .. 50 % duty cycle)
        
        // ledcWriteNote() returns 0 when note is a REST, so we switch off the channel
        // by setting the dyty cycle to 0, otherwise we set it to the value of volume 
        ledcWriteNote(_channel, n.note, n.octave) ? ledcWrite(_channel, _volume) : ledcWrite(_channel, 0);
        _msStart = millis();  // remember the start time
        _started = true;      // set the started flag
        return;    
    }

    if ((millis() - _msStart) > 60000 * (uint32_t)n.value / N4_LEN / (uint32_t)_tempo) // is the note length reached?
    {
        ledcWrite(_channel, 0); // stop the tone
        _started    = false;    // reset the started flag
        _notePlayed = true;     // set the played flag
        delay(_msNoteGap);      // wait some ms to separate notes (set the ms with the function setLegato())
    }
}

/**
 * Play the melody, passed as array of notes
 * Call it in the main loop
 */
void MelodyPlayer::playMelody(musicNote m[], int len, bool repeat)
{
    _notePlayed = false;
    if (_noteCounter >= len) 
    { 
        if (repeat) _noteCounter = 0; // reset the note counter to repeat the melody
        return; 
    }
    if (! _random) 
      playNote(m[_noteCounter]);
    else
      playNote(m[random(len)]);
    if (_notePlayed) _noteCounter++;  // take next note in melody
}

/**
 * Play the melody which was set with setMelody()
 * Call it in the main loop
 */
void MelodyPlayer::playMelody(bool repeat)
{
    if (_melody == nullptr) return;
    playMelody(_melody, _melodyLength, repeat);
}

/**
 * Beats the beat at the set tempo 
 * Call it in the main loop
 */
void MelodyPlayer::playBeats()
{
    if (! _started)
    {
        ledcWriteNote(0, NOTE_A, 7);
        ledcWrite(_channel, _volume);
        _started = true;
        _msStart = millis();
    }
    if ((millis() - _msStart) > 4)  mute();
    if ((millis() - _msStart) > 60000 / (uint32_t)_tempo) _started = false;
}

/**
 * Rearm player to play note again after msWait milliseconds
 * To be used after calling playNote()
 */
void MelodyPlayer::rearmNoteAfter(uint32_t msWait)
{
    if((millis() - _msPrevious >= msWait) ? (_msPrevious = millis(), true) : false) _notePlayed = false;
}