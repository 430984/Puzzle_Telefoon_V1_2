/* CHANGELOG:
 * V1.0 (7-1-2020) 
 *    - Initiële uitgave
 *    - Luik verwijderd uit de software. Speelt nu de code als
 *      audio en laat deze zien op het LCD scherm.
 * 
 * V1.1 (13-1-2020)
 *    - Wanneer telefoon opgepakt wordt, wordt niet langer de 
 *      luidspreker uitgeschakeld. De audio zal altijd via
 *      de luidspreker spelen in tegenstelling tot de originele 
 *      situatie waar de audio enkel via de hoorn speelde.
 *  
 * V1.2 (14-1-2020)
 *      - LCD tekst aangepast: Toets "+" is nu "A"
 *                             Toets "-" is nu "B"
 *      - SIT Timeout tijd aangepast van 20 sec naar 10 sec.
 *      - Lang drukken van de F-toets eruit gehaald.
 */

#include "enums.h"
#include "src/MCP23017.h"
#include "src/MP3.h"
#include "src/luik.h"
#include <LiquidCrystal.h>

#define PIN_LUIK A3
#define PIN_HOORNDETECTIE 2

#define PIN_LCD_RS 3
#define PIN_LCD_EN 4
#define PIN_LCD_D4 5
#define PIN_LCD_D5 6
#define PIN_LCD_D6 7
#define PIN_LCD_D7 8

#define PIN_AUDIO_SHDN 9

#define PIN_LCD_BACKLIGHT 10

#define DUUR_LUIK_OPEN 2000 // 2 seconden
#define DUUR_BACKLIGHT 5000
#define SBARR_TIMEOUT  10000 // 60 seconden

#define F_TOETS_TIMEOUT 5000 // 10 seconden

#define CODE_LUIKJE "135" // let op, string!

MCP23017 keypad;
MP3 mp3;
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
Luik luik(PIN_LUIK);

enum stappen 
{
  stap_resetAlles = 0,
  stap_standby = 1,
  stap_toetsGedrukt = 2,
  stap_speelDTMF = 3,

  stap_speeltDTMF = 4,
  stap_wachtOpStilte = 5,

  // Alle nummers gekozen
  stap_nummerGekozen = 10,
  stap_speelKiestoon = 11,
  stap_speeltKiestoon = 12,
  stap_wachtOpKiestoonKlaar = 13,
  stap_kiesNummer = 14,

  // Hoorn:
  stap_hoornOpgehangen = 20,
  
  stap_hoornAfgenomen = 30,
  
  
// Bellen:
  // Arts bellen
  stap_artsBellen = 100,
  // SIT-team
  stap_sitBellen = 200,
  stap_sitSpeeltGesprek = 201,
  stap_sitWachtOpGesprekKlaar = 202,
  // Invoer van de SBARR
  stap_sitSBARRVoorbereiding = 210,
  stap_sitSBARRWachtOpToets = 211,
  stap_sitSBARRToetsGedrukt = 212,
  stap_sitSBARRVolledigeInvoer = 213,
  // Invoer timeout
  stap_sitSBARRInvoerTimeout = 215,
  // SBARR controle
  stap_sitSBARRCorrect = 220,
  stap_sitSBARRCorrectSpeeltGesprek = 221,
  stap_sitSBARRCorrectWachtOpGesprekKlaar = 222,
  stap_sitSBARRCorrectOpenLuik = 223,
  // SBARR is niet correct
  stap_sitSBARRIncorrect = 230,
  stap_sitSBARRIncorrectSpeeltGesprek = 231,
  stap_sitSBARRIncorrectWachtOpGesprekKlaar = 232,

  // Nummer is onbekend
  stap_nummerOnbekend = 900,
  // Easter egg (onze handtekening)
  stap_easteregg = 950,

  // Algemeen voor arts/easter egg: Wacht op gesprek klaar
  stap_speeltGesprek = 1000,
  stap_wachtOpGesprekKlaar = 1001,
  
};

int iIngevoerdeNummers[4];
int iStap = stap_resetAlles;

// ISR, Wordt aangeroepen als de hoorn opgehangen/afgenomen wordt
void hoornInterrupt()
{
  int iHoornStatus = digitalRead(PIN_HOORNDETECTIE);
  
  if(iHoornStatus)
  {
    // Hoorn afgenomen
    if(iStap == stap_standby)
      iStap = stap_hoornAfgenomen;
  }
  else
  {
    // Hoorn opgehangen
    //if(iStap != stap_standby || mp3.getPlayState() != 0)
    //{
      iStap = stap_hoornOpgehangen;
      //wis_ingevoerdNummer();
      //if(mp3.getPlayState() != 0)
      //  mp3.stop();
    //}
  }
}

// Overige functies 
void wis_ingevoerdNummer()
{
  for(int i = 0; i < 4; i++)
  {
    iIngevoerdeNummers[i] = -1;
  }
}

// Initialisaties
void setupVariabelen()
{
  wis_ingevoerdNummer();
}

// Initialiseert het toetsenbord
void setupKeypad()
{
  keypad.begin();
  // Alle pinnen als inputs, er zijn 16 knoppen namelijk.
  // Dit is achteraf niet nodig, in begin() wordt dit reeds gedaan.
  keypad.setPinMode(0xFFFF);
  // Pullup-weerstanden op elke pin aanzetten.
  keypad.setPullUp(0xFFFF);
}

// Initialiseert de MP3 speler
void setupMP3()
{
  int iMp3Setup = 1;
  mp3.begin();
  delay(3000);
  mp3.setVolume(16);
  while(iMp3Setup)
  {
    mp3.handle();
    if(mp3.setVolume(16) != -1)
    {
      iMp3Setup = 0;
    }
  }
}

void setupLCD()
{
  lcd.begin(16,2);
  lcd.clear();
  lcd.print(F("Initialiseren..."));
  digitalWrite(PIN_LCD_BACKLIGHT, HIGH);
}

void setupIO()
{
  //pinMode(PIN_LUIK, OUTPUT);
  pinMode(PIN_HOORNDETECTIE, INPUT_PULLUP);
  
  pinMode(PIN_AUDIO_SHDN, OUTPUT);
  pinMode(PIN_LCD_BACKLIGHT, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(PIN_HOORNDETECTIE), hoornInterrupt, CHANGE);
}

//void scherm(int iLocalStap)
void updateLCD()
{
  static int iOudeStap = -1;
  static int iVerlichtingStatus = 0;
  int iTemp; 

  static unsigned long ulVerlichtingTimer = 0;

  // Achtergrondverlichting timeout
  if(iVerlichtingStatus && (millis() >= ulVerlichtingTimer) && iStap == stap_standby)
  {
    iVerlichtingStatus = 0;
  }

  // Inhoud op het scherm aanpassen
  //if(iOudeStap != iStap)
  //{
    ulVerlichtingTimer = millis() + DUUR_BACKLIGHT;
    iOudeStap = iStap;
    // Schakelen aan de hand van de stap wat de achtergrondverlichting
    // moet doen.
    switch(iStap)
    {
      case stap_standby: 
        break;
      
      case stap_resetAlles:
        iVerlichtingStatus = 0;
        break;
      default:
        iVerlichtingStatus = 1;
        break;
    }
    
    // Schakelen aan de hand van de stap wat er op het scherm moet komen te staan
    switch(iStap)
    {
      case stap_resetAlles:
        lcd.clear(); 
        break;
              
      case stap_hoornAfgenomen: 
      //case stap_toetsGedrukt: 
      case stap_speelDTMF:       
        lcd.clear();
        lcd.print(F("Kies een nummer:"));
        lcd.setCursor(6, 1);
        for(int i = 0; i < 4; i++)
        {
          if(iIngevoerdeNummers[i] != -1) 
            lcd.print(iIngevoerdeNummers[i]);
          else
            lcd.print(F("_"));
        }
        break;
        
      case stap_nummerGekozen:
        lcd.clear();
        lcd.print(F("Belt: "));
        for(int i = 0; i < 4; i++)
          lcd.print(iIngevoerdeNummers[i]);
        break;
        
      case stap_artsBellen:
        lcd.clear();
        lcd.print(F("Voicemail..."));
        break;
        
      case stap_sitBellen:
        lcd.clear();
        lcd.print(F("SIT Team"));
        break;

      case stap_nummerOnbekend:
        lcd.clear();
        lcd.print(F("Sorry, nummer ")); 
        lcd.setCursor(0, 1); 
        lcd.print(F("is onbekend."));
        break;
        
      case stap_easteregg:
        lcd.clear();
        lcd.print(F("Ik hou van NIAZ!"));
        lcd.setCursor(6, 1);
        lcd.print(F(":-)"));
        break;

      case stap_sitSBARRVoorbereiding:
        lcd.clear();
        lcd.print(F("Wat is de"));
        lcd.setCursor(0,1);
        lcd.print(F("situatie?"));
        break;

      case stap_sitSBARRToetsGedrukt:
        //if(iIngevoerdeNummers[0] != -1)
        //{
          lcd.clear();
          lcd.print(F("Situatie: "));
          for(int i = 0; i < 4; i++)
          {
            switch(iIngevoerdeNummers[i])
            {
              case toets_x:
                lcd.print(F("X"));
                break;
              case toets_plus:
                //lcd.print(F("+"));
                lcd.print(F("A"));
                break;
              case toets_min:
                //lcd.print(F("-"));
                lcd.print(F("B"));
                break;
              case toets_c:
                lcd.print(F("C"));
                break;
              case toets_F:
                lcd.print(F("F"));
                break;
              case toets_simp:
                break;
              default:
                lcd.print(F("_"));
                break;
            }
          }
        //}
        break;

      case stap_sitSBARRCorrectOpenLuik:
        lcd.clear();
        lcd.print(F("Code voor deur"));
        lcd.setCursor(0,1);
        lcd.print(F("hieronder: 135"));
        break;
        /*
      case stap_sitSBARRInvoerTimeout:
        lcd.clear();
        lcd.print(F("Gebruik toetsen"));
        lcd.setCursor(0,1);
        lcd.print(F("alstublieft."));
        break;
        */
    }
  //}
  digitalWrite(PIN_LCD_BACKLIGHT, iVerlichtingStatus);
}

// Houdt het toetsenbord in de gaten of er een
// knop is ingedrukt.
int keyPad()
{
  static word wOudeToetsenStatus = 0;
  static int iFToetsStatus = 0;
  static unsigned long ulToetsTimer;
  
  int iToets = -1;

  // Volledig woord uitlezen (16 bits)
  word wToetsenStatus = keypad.getPins() ^0xFFFF;
  // Kijken of er bits veranderd zijn ten opzichte van de 
  // vorige keer.
  word wVeranderd = wToetsenStatus ^ wOudeToetsenStatus;

  if((wToetsenStatus >> 8) & 0x01)
  {
    if(iFToetsStatus != 2)
    {
      if(iFToetsStatus != 1)
      {
        iFToetsStatus = 1;
        ulToetsTimer = millis() + F_TOETS_TIMEOUT;
      }
      else
      {
        if(millis() >= ulToetsTimer)
        {
          iToets = toets_F_lang;
          iFToetsStatus = 2;
        }
      }
    }
  }
  else
    iFToetsStatus = 0;
  
  // Als dat zo is:
  if((wToetsenStatus ^ wOudeToetsenStatus) != 0)
  {
    // Oude status opslaan
    wOudeToetsenStatus = wToetsenStatus;
    // Elk bitje nagaan of die hoog of laag is
    for(int i = 0; i < 16; i++)
    {
      // Toetsen aan de juiste bits koppelen
      if(((wVeranderd & wToetsenStatus) & (1 << i)) && iToets == -1)
        switch(i)
        {
          case 0: // Toets F
            //iToets = toets_F;
            iToets = toets_min;
            break;
          case 1: // Toetsen 0..9
            iToets = toets_plus;
            break;
          case 2:
            iToets = toets_c;
            break;
          case 3:
            iToets = toets_simp;
            break;
          case 4:
            iToets = toets_x;
            break;
          case 5:
            iToets = toets_9;
            break;
          case 6:
            iToets = toets_8;
            break;
          case 7:
            iToets = toets_7;
            break;
          case 8:
            iToets = toets_F;
            break;
          case 9:
            iToets = toets_0;
            break;
          case 10:
            iToets = toets_1;
            break;
          case 11:
            iToets = toets_2;
            break;
          case 12:
            iToets = toets_3;
            break;
          case 13:
            iToets = toets_4;
            break;
          case 14:
            iToets = toets_5;
            break;
          case 15:
            iToets = toets_6;
            break;
          default:
            iToets = -1;
            break;
        }
    }
    
  }
  return iToets;
}

void setup() {
  setupVariabelen();
  setupIO();
  setupLCD();
  setupKeypad();
  setupMP3();
  luik.begin();
}

void loop() {
  // Variabelen
  // Lokale tijdelijke variabelen
  int iTempKeypad, iTempMp3Status, iTelefoonNummer, iTempCheckNummer;
  // Lokale statische variabelen (blijven behouden)
  //static int iStap = 0;
  static int iToetsGedrukt = -1;

  static int iLuikStatus = 0;
  static unsigned long ulLuikTimer = 0;
  static unsigned long ulSBARRTimeout = 0;

  // Handlers
  iTempKeypad = keyPad();
  iTempMp3Status = mp3.handle();
  luik.handle();

  // V1.0:
  //digitalWrite(PIN_AUDIO_SHDN, (!digitalRead(PIN_HOORNDETECTIE) && mp3.getPlayState() != 0));
  // V1.1:
  digitalWrite(PIN_AUDIO_SHDN, mp3.getPlayState() != 0);
  
  if(iTempKeypad != -1 && (iStap == stap_standby || iStap == stap_sitSBARRWachtOpToets))
  {
    iToetsGedrukt = iTempKeypad;
  }

  // Main programma
  switch (iStap)
  {
    case stap_resetAlles:
      updateLCD();
      wis_ingevoerdNummer();
      iToetsGedrukt = -1;
      if(mp3.getPlayState() != 0)
        mp3.stop();
      iStap = stap_standby;
      break;
      
    case stap_standby:
      iTempCheckNummer = 1;
      for(int i = 0; i < 4; i++)
      {
        if(iIngevoerdeNummers[i] == -1) 
          iTempCheckNummer = 0;
      }
      // Is er een volledig telefoonnummer ingevoerd?
      if(iTempCheckNummer == 1)
      {
        iStap = stap_nummerGekozen;
      }
      
      // Of is er een toets gedrukt?
      else if(iToetsGedrukt >= toets_0 && iToetsGedrukt <= toets_9)
        iStap = stap_toetsGedrukt;

      // Of is de F-toets een bepaalde tijd ingehouden?
      //else if(iToetsGedrukt == toets_F_lang)
      //  iStap = stap_sitSBARRCorrectOpenLuik;
        
      break;

    // Nummer toevoegen aan de invoerlijst
    case stap_toetsGedrukt:
      for(int i = 0; i < 4; i++)
        if(iIngevoerdeNummers[i] == -1)
        {
          iIngevoerdeNummers[i] = iToetsGedrukt;
          //updateLCD();
          iStap = stap_speelDTMF;
          break;
        }
      break;

    // MP3 speler (proberen te) starten met bijbehorende DTMF toon
    case stap_speelDTMF:
      updateLCD();
      if(mp3.playSound(map_dtmf, (iToetsGedrukt == 0 ? 10 : iToetsGedrukt)) != -1) 
        iStap = stap_speeltDTMF;
      break;

    // Wachten tot MP3 speler speelt
    case stap_speeltDTMF:
      if(mp3.getPlayState() != 0)
        iStap = stap_wachtOpStilte;
      break;

    // Wachten tot MP3 speler niet meer speelt
    case stap_wachtOpStilte:
      if(mp3.getPlayState() == 0)
      {
        iToetsGedrukt = -1;
        iStap = stap_standby;
      }
      break;

//--------------------------------------------------------------------------------------------------
// Hoorn is op de houder gehangen
    case stap_hoornOpgehangen:
      iStap = stap_resetAlles;
      break;

//--------------------------------------------------------------------------------------------------
// Hoorn is van de houder afgenomen
    case stap_hoornAfgenomen:
      updateLCD();
      if(mp3.playSound(map_overig, geluid_overig_offhook) != -1) 
        iStap = stap_standby;
      break;

//--------------------------------------------------------------------------------------------------
// Een nummer is gekozen. 
    case stap_nummerGekozen:
      updateLCD();
      iStap = stap_speelKiestoon;
      break;
      
// Als eerste wordt een kiestoon afgespeeld

    // Commando aan de MP3 speler geven
    case stap_speelKiestoon:
      if(mp3.playSound(map_overig, geluid_overig_gaatover) != -1)
        iStap = stap_speeltKiestoon;
      break;

    // Commando is goed terecht gekomen, wachten tot MP3 speler start.
    case stap_speeltKiestoon:
      if(mp3.getPlayState() != 0)
        iStap = stap_wachtOpKiestoonKlaar;
      break;

    // MP3 speler is gestart, wachten tot deze klaar is.
    case stap_wachtOpKiestoonKlaar:
      if(mp3.getPlayState() == 0)
      {
        iStap = stap_kiesNummer;
      }
      break;

    // Kijken welk nummer is ingevoerd.
    case stap_kiesNummer:
      // Controleer nummers. Moeten tussen 0 en 9 liggen.
      
      // Een telefoonnummer maken van de ingevoerde nummers zodat we aan de hand van dit getal kunnen schakelen
      iTelefoonNummer = iIngevoerdeNummers[0] * 1000 + iIngevoerdeNummers[1] * 100 + iIngevoerdeNummers[2] * 10 + iIngevoerdeNummers[3];
      // Buffer leegmaken
      wis_ingevoerdNummer();
      // Bekijken welk nummer ingevoerd is
      switch(iTelefoonNummer)
      {
        case nummer_arts:           // Is het nummer van de arts?
          iStap = stap_artsBellen;
          break;
        case nummer_sitteam:        // Is het nummer van het SIT-team?
          iStap = stap_sitBellen;
          break;
        //case nummer_bms:          // Is het nummer van de BMS?
        //  break;
        //case nummer_beveiliging:  // Is het nummer van de beveiliging?
        //  break;
        //case nummer_bhvpost:      // Is het nummer van de BHV post?
        //  break;
        case nummer_easteregg:      // Google dit: U+1F60F
          iStap = stap_easteregg;
          break;
        default:                    // Is het een ander (onbekend) nummer?
          iStap = stap_nummerOnbekend;
          break;
      }
      break;

//---------------------------------------------------------------------------------------------------
// De arts wordt gebeld, voicemail!
    // Commando aan de MP3 speler geven
    case stap_artsBellen:
      updateLCD();
      if(mp3.playSound(map_gesprekken, geluid_gesprek_voicemailArts) != -1)
        iStap = stap_speeltGesprek;
      break;

//---------------------------------------------------------------------------------------------------
// Easter egg
    // Commando aan de MP3 speler geven
    case stap_easteregg:
      updateLCD();
      if(mp3.playSound(map_gesprekken, geluid_gesprek_easteregg) != -1)
        iStap = stap_speeltGesprek;
      break;

//---------------------------------------------------------------------------------------------------
// Onbekend nummer
    // Commando aan de MP3 speler geven
    case stap_nummerOnbekend:
      updateLCD();
      if(mp3.playSound(map_overig, geluid_overig_nummerOnbekend) != -1)
        iStap = stap_speeltGesprek;
      break;
      
//---------------------------------------------------------------------------------------------------
// Algemeen bij gesprekken (behalve SIT-team)
  // Commando is goed terecht gekomen, wachten tot MP3 speler start.
    case stap_speeltGesprek:
      if(mp3.getPlayState() != 0)
        iStap = stap_wachtOpGesprekKlaar;
      break;

    // MP3 speler is gestart, wachten tot deze klaar is.
    case stap_wachtOpGesprekKlaar:
      if(mp3.getPlayState() == 0)
        iStap = stap_resetAlles;
      break;

//---------------------------------------------------------------------------------------------------
// SIT-team
    case stap_sitBellen:
      updateLCD();
      if(mp3.playSound(map_gesprekken, geluid_gesprek_sitTeamIntro) != -1)
        //iStap = stap_speeltGesprek;
        iStap = stap_sitSpeeltGesprek;
      break;

    case stap_sitSpeeltGesprek:
      if(mp3.getPlayState() != 0)
        iStap = stap_sitWachtOpGesprekKlaar;
      break;

    case stap_sitWachtOpGesprekKlaar:
      if(mp3.getPlayState() == 0)
        iStap = stap_sitSBARRVoorbereiding;
      break;

    case stap_sitSBARRVoorbereiding:
      updateLCD();
      wis_ingevoerdNummer();
      ulSBARRTimeout = millis() + SBARR_TIMEOUT;
      iStap = stap_sitSBARRWachtOpToets;
      break;
      
      
    // Invoer van de SBARR
    case stap_sitSBARRWachtOpToets:
      iTempCheckNummer = 1;
      for(int i = 0; i < 4; i++)
      {
        if(iIngevoerdeNummers[i] == -1) 
          iTempCheckNummer = 0;
      }
      // Is er een volledig telefoonnummer ingevoerd?
      if(iTempCheckNummer == 1)
      {
        iStap = stap_sitSBARRVolledigeInvoer;
      }
      // Of is er een toets gedrukt?
      else if(iToetsGedrukt > toets_9 && iToetsGedrukt < toets_simp)
      {
        iStap = stap_sitSBARRToetsGedrukt;
      }
      else if(millis() >= ulSBARRTimeout && (iIngevoerdeNummers[0] == -1 &&
                                             iIngevoerdeNummers[1] == -1 &&
                                             iIngevoerdeNummers[2] == -1 &&
                                             iIngevoerdeNummers[3] == -1))
      {
        iStap = stap_sitSBARRInvoerTimeout;
      }
      break;
      
    case stap_sitSBARRToetsGedrukt:
      for(int i = 0; i < 4; i++)
        if(iIngevoerdeNummers[i] == -1)
        {
          iIngevoerdeNummers[i] = iToetsGedrukt;
          updateLCD();
          iToetsGedrukt = -1;
          ulSBARRTimeout = millis() + SBARR_TIMEOUT;
          iStap = stap_sitSBARRWachtOpToets;
          break;
        }
      break;
      
    case stap_sitSBARRVolledigeInvoer:
      if(iIngevoerdeNummers[0] == toets_min &&
         iIngevoerdeNummers[1] == toets_c &&
         iIngevoerdeNummers[2] == toets_x &&
         iIngevoerdeNummers[3] == toets_plus)
      {
        iStap = stap_sitSBARRCorrect;
      }
      else
        iStap = stap_sitSBARRIncorrect;
      wis_ingevoerdNummer();
      break;

    case stap_sitSBARRInvoerTimeout:
      updateLCD();
      if(mp3.playSound(map_gesprekken, geluid_gesprek_sitTeamSBARRTimeout) != -1)
      {
        iStap = stap_sitSBARRVoorbereiding;
      }
      break;
      
    // SBARR controle
    case stap_sitSBARRCorrect:
      if(mp3.playSound(map_gesprekken, geluid_gesprek_sitTeamSBARRIsCorrect) != -1)
      {
        iStap = stap_sitSBARRCorrectSpeeltGesprek;
        //iStap = stap_sitSBARRCorrectOpenLuik;
      }
      break;
      
    case stap_sitSBARRCorrectSpeeltGesprek:
      if(mp3.getPlayState() != 0)
        iStap = stap_sitSBARRCorrectWachtOpGesprekKlaar;
      break;
      
    case stap_sitSBARRCorrectWachtOpGesprekKlaar:
      if(mp3.getPlayState() == 0)
        iStap = stap_sitSBARRCorrectOpenLuik;
      break;
      
    //case stap_sitSBARRCorrectOpenLuik:
      //luik.openLuik();
      //iStap = stap_resetAlles;
      //break;

    case stap_sitSBARRCorrectOpenLuik:
      if(mp3.playSound(map_gesprekken, geluid_gesprek_sitTeamCode) != -1)
        {
          updateLCD();
          iStap = stap_speeltGesprek;
          //iStap = stap_sitSBARRCorrectOpenLuik;
        }
      break;
      
    // SBARR is niet correct
    case stap_sitSBARRIncorrect:
      if(mp3.playSound(map_gesprekken, geluid_gesprek_sitTeamSBARRIsIncorrect) != -1)
      {
        iStap = stap_sitSBARRIncorrectSpeeltGesprek;
      }
      break;
      
    case stap_sitSBARRIncorrectSpeeltGesprek:
      if(mp3.getPlayState() != 0)
        iStap = stap_sitSBARRIncorrectWachtOpGesprekKlaar;
      break;
      
    case stap_sitSBARRIncorrectWachtOpGesprekKlaar:
      if(mp3.getPlayState() == 0)
        iStap = stap_sitSBARRVoorbereiding;
      break;
      
//---------------------------------------------------------------------------------------------------
    default:
      iStap = stap_resetAlles;
      break;
  }

  //scherm(iStap);
}
