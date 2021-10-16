
/**
 * Header       MelodyPlayer.h
 * Author       2021-08-28 Charles Geiser (https://www.dodeka.ch)
 * 
 * Purpose      Declaration of the class MelodyPlayer
 * 
 * Constructor
 * arguments    pin         ESP32 pin which outputs the tone
 *              channel     ESP32 pwm channel
 */
#ifndef _MELODYPLAYER_H_
#define _MELODYPLAYER_H_
#include <Arduino.h>

#define REST NOTE_MAX

// Tempo given as number of quarter notes per minute
enum class TEMPO   { LARGO=50, LARGHETTO=63, ADAGIO=71, ANDANTE=92, MODERATO=114, ALLEGRO=144, PRESTO=184, PRESTISSIMO=204 };

// Note values (example: N4d is a dotted quarter note, N2 is a half note)
enum class N_LEN { N64=1, N32=2, N32d=3, N16=4, N16d=6, N8=8, N8d=12, N4=16, N4d=24, N2=32, N2d=48, N1=64, N1d=96 };
const uint32_t N4_LEN = 16;

// A musicNote is defined as a NOTE_x, in octave octave, 
// with duration defined as its weight in 64ths.
// Example: { NOTE_A, 4, N_LEN::N4d } is the concert pitch 440 Hz as a dotted quarter note
typedef struct { note_t note; uint8_t octave; N_LEN value; } musicNote;

class MelodyPlayer
{
    public:
        MelodyPlayer(uint8_t pin, uint8_t channel) : _pin(pin), _channel(channel)
        {
            ledcSetup(_channel, 20000, 8);
            ledcAttachPin(_pin, _channel);
            ledcWrite(_channel, _volume);
        };
        void setVolume(uint32_t volume);
        void setTempo(TEMPO tempo);
        void setTempo(int tempo);
        void setLegato(uint32_t msNoteGab);
        void setMelody(musicNote m[], int len);
        void setRandomMode();
        void setNormalMode();
        void mute();
        void playNote(musicNote n);
        void playMelody(musicNote m[], int len, bool repeat = false);
        void playMelody(bool repeat = false);
        void playBeats();
        void rearmNoteAfter(uint32_t msWait);
        
    private:
        uint8_t  _pin;
        uint8_t  _channel;
        uint32_t _volume      = 0; // 0..511
        uint32_t _msStart     = 0;
        uint32_t _msNoteGap   = 10;
        uint32_t _msPrevious  = 0;
        int      _noteCounter = 0;
        bool     _started     = false;
        bool     _notePlayed  = false;
        bool     _random      = false;
        int      _melodyLength;
        TEMPO    _tempo = TEMPO::MODERATO;
        musicNote *_melody = nullptr;    
};
#endif