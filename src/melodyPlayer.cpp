/**
 * Program      melodyPlayer.cpp
 * Author       2021-08-21 Charles Geiser (https://www.dodeka.ch)
 * 
 * Purpose      Demonstrates the usage of the class MelodyPlayer. A CLI menu is used to select 
 *              different melodies, set tempo or volume and to play the notes of a melody in 
 *              normal order or randomly. You may also choose to beat the beat in the set tempo.
 *              The volume at the speaker output can be changed in the range 0..511, which 
 *              corresponds to a duty cycle of 0..50%.   
 * 
 * Board        ESP32 DoIt DevKit V1
 * 
 *                           .--------------.
 * Wiring                    |      Vin(5V) |---->   5V o--------------+------o
 *                           |              |                          |      |
 *                           |              |                         _|_     |   _/|
 *                           |    ESP32     |                         / \     '--|  |
 *                           |              |                         ¨|¨     .--|_ | Speaker
 *                           |              |                          |      |    \|
 *                           |              |                          +------o
 *                           |              |                          |
 *                           |              |                      |¦--' N-CH MOSFET
 *                           |              |                      |¦<-. T40N03G
 *                           |       GPIO25 |---->  Vin o-----+----|¦--|
 *                           |              |                 |        |
 *                           |              |                |¨|       |
 *                           |              |                |_| 10k   |
 *                           |              |                 |        |
 *                           |          GND |---->  GND o-----+--------'
 *                           '--------------'
 * 
 * Remarks      Instead of the speaker with driver you can also connect a piezo buzzer directly
 *              from GPIO25 to GND
 *
 * References   https://makeabilitylab.github.io/physcomp/esp32/tone.html#the-esp32-tone-problem
 *              
 */

#include <Arduino.h>
#include "MelodyPlayer.h"

//#define CLR_LINE "\r                                                                      \r"
#define CLR_LINE "\r%*c\r", 128, ' '
const int channel  = 0;
const int PIN_SPKR = GPIO_NUM_25;
int volume         = 1; // 0..511 for duty cycle 0..50%
bool beatTheBeat   = false;

typedef struct { const char key; const char *txt; void (&action)(char ch); } MenuItem;

// Forward declaration of menu actions
void playMelody(char ch);
void playBeats(char ch);
void setTempo(char ch);
void setTempo1(char ch);
void setLegato(char ch);
void setVolume(char ch);
void setNormal(char ch);
void setRandom(char ch);
void showMenu(char ch);

MenuItem menu[] = 
{
  { 'a', "[a] Play Am Louenesee",                        playMelody },
  { 'c', "[c] Play Chum Bueb",                           playMelody },
  { 'e', "[e] Play Entertainer",                         playMelody },
  { 'o', "[o] Play Old Mac Donald",                      playMelody },
  { 'm', "[m] Play Martinshorn",                         playMelody },
  { 'p', "[p] Play Postauto",                            playMelody },
  { 'C', "[C] Play Chromatic Scale",                     playMelody },
  { 'P', "[P] Play Pentatonic Scale",                    playMelody },
  { 'B', "[B] Beat the beat",                            playBeats },
  { 't', "[t] Set Tempo [1..8]",                         setTempo },
  { 'b', "[b] Set Tempo [beats per minute]",             setTempo1 },
  { 'l', "[l] Set Legato (gap between notes)[0..100ms]", setLegato },
  { 'v', "[v] Set Volume [0..511]",                      setVolume },
  { 'n', "[n] Set normal mode",                          setNormal },
  { 'r', "[r] Set random mode",                          setRandom },
  { 'S', "[S] Show Menu",                                showMenu },
};
constexpr int nbrMenuItems = sizeof(menu) / sizeof(menu[0]);

// A melody is defined as an array of musicNotes
musicNote oldMacDonald[] =
{
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_D, 4, N_LEN::N4 },
  { NOTE_E, 4, N_LEN::N4 },
  { NOTE_E, 4, N_LEN::N4 },
  { NOTE_D, 4, N_LEN::N2 },
  { NOTE_B, 4, N_LEN::N4 },
  { NOTE_B, 4, N_LEN::N4 },
  { NOTE_A, 4, N_LEN::N4 },
  { NOTE_A, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N2d },

  { NOTE_D, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_D, 4, N_LEN::N4 },
  { NOTE_E, 4, N_LEN::N4 },
  { NOTE_E, 4, N_LEN::N4 },
  { NOTE_D, 4, N_LEN::N2 },
  { NOTE_B, 4, N_LEN::N4 },
  { NOTE_B, 4, N_LEN::N4 },
  { NOTE_A, 4, N_LEN::N4 },
  { NOTE_A, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N2d },

  { NOTE_D, 4, N_LEN::N8 },
  { NOTE_D, 4, N_LEN::N8 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { REST,   4, N_LEN::N4 },

  { NOTE_G, 3, N_LEN::N4 },
  { NOTE_G, 3, N_LEN::N4 },
  { NOTE_G, 3, N_LEN::N4 },
  { REST,   4, N_LEN::N4 },

  { NOTE_G, 4, N_LEN::N8 },
  { NOTE_G, 4, N_LEN::N8 },
  { NOTE_G, 4, N_LEN::N4 },

  { NOTE_G, 3, N_LEN::N8 },
  { NOTE_G, 3, N_LEN::N8 },
  { NOTE_G, 3, N_LEN::N4 },

  { NOTE_G, 4, N_LEN::N8 },
  { NOTE_G, 4, N_LEN::N8 },
  { NOTE_G, 4, N_LEN::N8 },
  { NOTE_G, 4, N_LEN::N8 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N4 },
  { NOTE_D, 4, N_LEN::N4 },
  { NOTE_E, 4, N_LEN::N4 },
  { NOTE_E, 4, N_LEN::N4 },
  { NOTE_D, 4, N_LEN::N2 },
  { NOTE_B, 4, N_LEN::N4 },
  { NOTE_B, 4, N_LEN::N4 },
  { NOTE_A, 4, N_LEN::N4 },
  { NOTE_A, 4, N_LEN::N4 },
  { NOTE_G, 4, N_LEN::N1 },
  { REST,   4, N_LEN::N2 },
};
constexpr int len_oldMacDonald = sizeof(oldMacDonald) / sizeof(oldMacDonald[0]);

musicNote chomBueb[] =
{
  { NOTE_E,  4, N_LEN::N2 },
  { NOTE_E,  5, N_LEN::N2d },
  { NOTE_Cs, 5, N_LEN::N4 },
  { NOTE_A,  4, N_LEN::N4 },
  { NOTE_Fs, 4, N_LEN::N4 },
  { NOTE_E,  4, N_LEN::N4d },
  { NOTE_Fs, 4, N_LEN::N8 },
  { NOTE_E,  4, N_LEN::N2 },
  { REST,    4, N_LEN::N1d},  // REST is defined as NOTE_MAX
};
constexpr int len_chomBueb = sizeof(chomBueb) / sizeof(chomBueb[0]);

musicNote amLouenesee[] =
{
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N16 },
  { NOTE_B ,  4, N_LEN::N8d },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8d },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N4 },
  { REST,     4, N_LEN::N4 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N4 },
  { REST,     4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N4d },
  { REST,     4, N_LEN::N8 },
  { REST,     4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N8d },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N16 },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8d },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Cs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N4 },
  { REST,     4, N_LEN::N4 },
  { NOTE_E,   4, N_LEN::N8 },
  { NOTE_Cs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N4d },
  { REST,     4, N_LEN::N4 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N4 },
  { NOTE_B,   4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8d },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_E ,  4, N_LEN::N4 },
  { REST,     4, N_LEN::N4 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N4 },
  { REST,     4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N4d },
  { REST,     4, N_LEN::N8 },
  { REST,     4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N4 },
  { NOTE_Fs,  4, N_LEN::N16 },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8d },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Cs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N4 },
  { REST,     4, N_LEN::N4 },
  { NOTE_E,   4, N_LEN::N8 },
  { NOTE_Cs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N4d },
  { REST,     4, N_LEN::N4d },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_Cs,  5, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N4 },
  { REST,     4, N_LEN::N4 },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_Cs,  5, N_LEN::N8d },
  { NOTE_Cs,  5, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_Cs,  5, N_LEN::N4d },
  { REST,     4, N_LEN::N8 },
  { REST,     4, N_LEN::N8d },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Cs,  5, N_LEN::N8 },
  { NOTE_Cs,  5, N_LEN::N8d },
  { NOTE_Cs,  5, N_LEN::N16 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_E,   4, N_LEN::N4 },
  { REST,     4, N_LEN::N8 },
  { REST,     4, N_LEN::N8d },
  { NOTE_E,   4, N_LEN::N16 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Fs,  4, N_LEN::N8 },
  { NOTE_Gs,  4, N_LEN::N8 },
  { NOTE_A,   4, N_LEN::N4d },
  { REST,     4, N_LEN::N4d },
  { REST,     4, N_LEN::N4d },
  { REST,     4, N_LEN::N4d },
  { REST,     4, N_LEN::N8 },
  { REST,     4, N_LEN::N8 },
  { NOTE_B,   3, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_B,   4, N_LEN::N8 },
  { NOTE_A,   4, N_LEN::N16 },
  { NOTE_Gs,  4, N_LEN::N4d },
  { REST,     4, N_LEN::N4 },
  { REST,     4, N_LEN::N4 },
  { REST,     4, N_LEN::N16 },
};
constexpr int len_amLouenesee = sizeof(amLouenesee) / sizeof(amLouenesee[0]);

musicNote entertainer[] =
{
  { NOTE_D,  4, N_LEN::N8 },
  { NOTE_Eb, 4, N_LEN::N8 },
  { NOTE_E,  4, N_LEN::N8 },
  { NOTE_C,  5, N_LEN::N4 },
  { NOTE_E,  4, N_LEN::N8 },
  { NOTE_C,  5, N_LEN::N4 },
  { NOTE_E,  4, N_LEN::N8 },
  { NOTE_C,  5, N_LEN::N8 },
  { NOTE_C,  5, N_LEN::N2 },
  { REST,    5, N_LEN::N8 },
  { NOTE_C,  5, N_LEN::N8 },
  { NOTE_D,  5, N_LEN::N8 },
  { NOTE_Eb, 5, N_LEN::N8 },
  { NOTE_E,  5, N_LEN::N8 },
  { NOTE_C,  5, N_LEN::N8 },
  { NOTE_D,  5, N_LEN::N8 },
  { NOTE_E,  5, N_LEN::N4 },
  { NOTE_B,  4, N_LEN::N8 },
  { NOTE_D,  5, N_LEN::N4 },
  { NOTE_C,  5, N_LEN::N2d},
  { REST  ,  5, N_LEN::N1d},
};
constexpr int len_entertainer = sizeof(entertainer) / sizeof(entertainer[0]);

musicNote martinshorn[] =
{
  { NOTE_Cs, 4, N_LEN::N4 },
  { NOTE_Gs, 4, N_LEN::N4 },
};

musicNote postauto[] =
{
  { NOTE_Cs, 5, N_LEN::N4 },
  { NOTE_E,  4, N_LEN::N4 },
  { NOTE_A,  4, N_LEN::N4d },
  { REST,    4, N_LEN::N2d },
};
constexpr int len_postauto = sizeof(postauto) / sizeof(postauto[0]);

musicNote pentatonicScale[] =
{
  { NOTE_C,  4, N_LEN::N4 },
  { NOTE_D,  4, N_LEN::N4 },
  { NOTE_E,  4, N_LEN::N4 },
  { NOTE_G,  4, N_LEN::N4 },
  { NOTE_A,  4, N_LEN::N4 },
  { NOTE_B,  4, N_LEN::N4 },
};
constexpr int len_pentatonic = sizeof(pentatonicScale) / sizeof(pentatonicScale[0]);

musicNote chromaticScale[] = 
{
  { NOTE_C,  4, N_LEN::N4 },
  { NOTE_Cs, 4, N_LEN::N4 },
  { NOTE_D , 4, N_LEN::N4 },
  { NOTE_Eb, 4, N_LEN::N4 },
  { NOTE_E , 4, N_LEN::N4 },
  { NOTE_F , 4, N_LEN::N4 },
  { NOTE_Fs, 4, N_LEN::N4 },
  { NOTE_G , 4, N_LEN::N4 },
  { NOTE_Gs, 4, N_LEN::N4 },
  { NOTE_A,  4, N_LEN::N4 },
  { NOTE_Bb, 4, N_LEN::N4 },
  { NOTE_B , 4, N_LEN::N4 },
  { NOTE_C,  5, N_LEN::N4 },
};
constexpr int len_chromatic = sizeof(chromaticScale) / sizeof(chromaticScale[0]); 


MelodyPlayer player(PIN_SPKR, channel);
constexpr int len_martinshorn = sizeof(martinshorn) / sizeof(martinshorn[0]);

/**
 * Plays the selected melody nonstop
 */
void playMelody(char ch)
{
  beatTheBeat = false;
  player.setVolume(2);
  switch(ch)
  {
    case 'a': player.setMelody(amLouenesee, len_amLouenesee);
              Serial.printf("Playing 'Am Louenesee' "); 
    break;
    case 'c': player.setMelody(chomBueb, len_chomBueb);
              Serial.printf("Playing 'Chom Bueb' ");  
    break;
    case 'o': player.setMelody(oldMacDonald, len_oldMacDonald);
              Serial.printf("Playing 'Old Mac Donald' ");  
    break;    
    case 'e': player.setMelody(entertainer, len_entertainer);
              Serial.printf("Playing 'Entertainer' ");  
    break;
    case 'm': player.setMelody(martinshorn, len_martinshorn);
              Serial.printf("Playing 'Martinshorn cis-gis' "); 
    break;
    case 'p': player.setMelody(postauto, len_postauto);
              Serial.printf("Playing 'Postauto cis-e-a' "); 
    break;
    case 'C': player.setMelody(chromaticScale, len_chromatic);
              Serial.printf("Playing 'Chromatic Scale' "); 
    break;
    case 'P': player.setMelody(pentatonicScale, len_pentatonic);
              Serial.printf("Playing 'Pentatonic Scale' "); 
    break;
    default:
    break;
  }
}

/**
 * Beat the beats like a metronom
 */
void playBeats(char ch)
{
  beatTheBeat = true;
  player.setVolume(100);
  Serial.printf("%s", "Playing beats ");
}

/**
 * Set tempo from enumeration class TEMPO
 */
void setTempo(char ch)
{
  int32_t value = 0;

  delay(2000);
  while (Serial.available())
  {
    value = Serial.parseInt();
  }
  switch(value)
  {
    case 1: player.setTempo(TEMPO::LARGO);
            Serial.printf("Tempo set to 'LARGO' %d ", (int)TEMPO::LARGO);
    break;
    case 2: player.setTempo(TEMPO::LARGHETTO);
            Serial.printf("Tempo set to 'LARGHETTO' %d ", (int)TEMPO::LARGHETTO);
    break;
    case 3: player.setTempo(TEMPO::ADAGIO);
            Serial.printf("Tempo set to 'ADAGIO' %d ", (int)TEMPO::ADAGIO);
    break;
    case 4: player.setTempo(TEMPO::ANDANTE);
            Serial.printf("Tempo set to 'ANDANTE' %d", (int)TEMPO::ANDANTE);
    break;
    case 5: player.setTempo(TEMPO::MODERATO);
            Serial.printf("Tempo set to 'MODERATO' %d ", (int)TEMPO::MODERATO);
    break;
    case 6: player.setTempo(TEMPO::ALLEGRO);
            Serial.printf("Tempo set to 'ALLEGRO' %d ", (int)TEMPO::ALLEGRO);
    break;
    case 7: player.setTempo(TEMPO::PRESTO);
            Serial.printf("Tempo set to 'PRESTO' %d ", (int)TEMPO::PRESTO);
    break;
    case 8: player.setTempo(TEMPO::PRESTISSIMO);
            Serial.printf("Tempo set to 'PRESTISSIMO' %d ", (int)TEMPO::PRESTISSIMO);
    break;
    default:  player.setTempo(60);
              Serial.printf("Tempo set to 'Default %d' ", 60);
    break;
  }
}

/**
 * Set tempo entered as number of
 * beats per minute
 */
void setTempo1(char ch)
{
  int32_t value = 0;

  delay(2000);
  while (Serial.available())
  {
    value = Serial.parseInt();
  }
  player.setTempo((int)value); 
  Serial.printf("Tempo set to %d beats per minute ", value); 
}

void setLegato(char ch)
{
  int32_t value = 0;

  delay(2000);
  while (Serial.available())
  {
    value = Serial.parseInt();
  }  
  player.setLegato(value);
  Serial.printf("Legato set to %d ms ", value);
}

/**
 * Set volume 0..511 which corresponds
 * to a duty cycle of 0..50%
 */
void setVolume(char ch)
{
  int32_t value = 0;
  char buf[32];

  delay(2000);
  while (Serial.available())
  {
    value = Serial.parseInt();
  }
  player.setVolume(value);
  snprintf(buf, sizeof(buf), "Volume set to %d ", value);
  Serial.print(buf);
}

/**
 * Set normal playing mode
 */
void setNormal(char ch)
{
  player.setNormalMode();
  Serial.printf("%s", "Normal mode set ");
}

/**
 * Set random playing mode
 */
void setRandom(char ch)
{
  player.setRandomMode();
  Serial.printf("%s", "Random mode set ");
}

/**
 * Show the menu
 */
void showMenu(char ch)
{
  // title is packed into a raw string
  Serial.print(
  R"TITLE(
-------------------
ESP32 Melody Player
-------------------
)TITLE");

  for (int i = 0; i < nbrMenuItems; i++)
  {
  Serial.println(menu[i].txt);
  }
  Serial.print("\nPress a key: ");
}

/**
 * Selects the menu action 
 * according to the pressed key
 */
void doMenu()
{
  char key = Serial.read();
  Serial.printf(CLR_LINE);
  for (int i = 0; i < nbrMenuItems; i++)
  {
  if (key == menu[i].key)
    {
    menu[i].action(key);
    break;
  }
  } 
}

void setup()
{
  Serial.begin(115200);
  showMenu('S');
}
   
void loop() 
{
  if (Serial.available()) doMenu();
  if (beatTheBeat) 
    player.playBeats();
  else
    player.playMelody(true);
}