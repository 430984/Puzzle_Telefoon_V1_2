// Mappen op de SD kaart van de MP3 speler
// Syntax: map_<mapnaam>
enum mappen
{
  map_overig = 1,
  map_dtmf = 2,
  map_gesprekken = 3,
};

// Geluiden in de mappen van de MP3 speler
// Syntax: geluid_<mapnaam>_<geluid>
enum geluiden
{
  // Map overige geluiden
  geluid_overig_offhook = 1,
  geluid_overig_gaatover = 2,
  geluid_overig_nummerOnbekend = 3,
  //geluid_overig_opgehangen = 4,
    
  // Map dtmf tonen
  geluid_dtmf_0 = 10,
  geluid_dtmf_1 = 1,
  geluid_dtmf_2 = 2,
  geluid_dtmf_3 = 3,
  geluid_dtmf_4 = 4,
  geluid_dtmf_5 = 5,
  geluid_dtmf_6 = 6,
  geluid_dtmf_7 = 7,
  geluid_dtmf_8 = 8,
  geluid_dtmf_9 = 9,

  // Map 'gesprekken'
  geluid_gesprek_voicemailArts = 10,
  
  geluid_gesprek_sitTeamIntro = 20,
  geluid_gesprek_sitTeamSBARRIsCorrect = 21,
  geluid_gesprek_sitTeamSBARRIsIncorrect = 22,
  geluid_gesprek_sitTeamCode = 23,
  geluid_gesprek_sitTeamSBARRTimeout = 25,
  
  
  geluid_gesprek_easteregg = 90,
  
};

// Gedefinieerde telefoonnummers
enum telefoonnummers
{
  nummer_arts = 1406,
  nummer_sitteam = 1880,
  //nummer_bms = 4444,
  //nummer_beveiliging = 8888,
  //nummer_bhvpost = 1134,

  nummer_easteregg = 4486,  // 4486 = IHVN
};

enum toetsen
{
  toets_0 = 0,
  toets_1 = 1,
  toets_2 = 2,
  toets_3 = 3,
  toets_4 = 4,
  toets_5 = 5,
  toets_6 = 6,
  toets_7 = 7,
  toets_8 = 8,
  toets_9 = 9,
  toets_F = 10,
  toets_x = 11,
  toets_c = 12,
  toets_plus = 13,
  toets_min = 14,
  toets_simp = 15,
  // Speciale functies
  toets_F_lang = 20,  // Toets F is lang ingedrukt
};

enum mp3dataindex{
  index_mp3_startFrame = 0,
  index_mp3_version = 1,
  index_mp3_frameLength = 2,
  index_mp3_command = 3,
  index_mp3_useFeedback = 4,
  index_mp3_paramHigh = 5,
  index_mp3_paramLow = 6,
  index_mp3_checkHigh = 7,
  index_mp3_checkLow = 8,
  index_mp3_endFrame = 9,
};
// Alleen relevante commando's. Voor alle commando's zie datasheet:
// http://www.picaxe.com/docs/spe033.pdf
enum mp3commands{
  cmd_mp3_playSound = 0x0F,
  cmd_mp3_stop = 0x16,
};
