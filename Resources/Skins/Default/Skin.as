[EntryPoint]
void InitializeSkin(Skin@ skin) {
  skin.LoadImage("*Note-Tap", "Images\\Note-Tap.png");
  skin.LoadImage("*Note-ExTap", "Images\\Note-ExTap.png");
  skin.LoadImage("*Note-Air", "Images\\Note-Air.png");
  skin.LoadImage("*Note-AirUp", "Images\\Air-Up.png");
  skin.LoadImage("*Note-AirDown", "Images\\Air-Down.png");
  skin.LoadImage("*Note-Flick", "Images\\Note-Flick.png");
  skin.LoadImage("*Note-HellTap", "Images\\Note-HellTap.png");
  skin.LoadImage("*Note-Hold", "Images\\Note-Hold.png");
  skin.LoadImage("*Note-HoldStep", "Images\\Note-HoldStep.png");
  skin.LoadImage("*Note-HoldStrut", "Images\\Note-HoldStrut.png");
  skin.LoadImage("*Note-Slide", "Images\\Note-Slide.png");
  skin.LoadImage("*Note-SlideStep", "Images\\Note-SlideStep.png");
  skin.LoadImage("*Note-SlideStrut", "Images\\Note-SlideStrut.png");
  skin.LoadImage("*Note-AirAction", "Images\\Note-AirAction.png");
  skin.LoadImage("*Lane-Ground", "Images\\Lane-Ground.png");
  skin.LoadImage("*Lane-JudgeLine", "Images\\Lane-JudgeLine.png");
  skin.LoadImage("*Lane-HoldLight", "Images\\Lane-HoldLight.png");
  
  skin.LoadImage("White", "Images\\White.png");               //つなぎ
  skin.LoadImage("TitleBack", "Images\\Background.png");          //1280x720
  skin.LoadImage("LogoSeaurchin", "Images\\SeaurchinLogo.png");   //320x320
  skin.LoadImage("LogoDxLib", "Images\\DxLogo.jpg");              //204^2
  skin.LoadImage("LogoAngelScript", "Images\\aslogo.png");        //311x135
  skin.LoadImage("PlayerTopCover", "Images\\PlayerTopCover.png"); //1280x106
  skin.LoadImage("GaugeCountEmpty", "Images\\GaugeCountEmpty.png");
  skin.LoadImage("GaugeCountFull", "Images\\GaugeCountFull.png");
  skin.LoadImage("GaugeBarBack", "Images\\GaugeBarBack.png");
  skin.LoadImage("GaugeBarFill", "Images\\GaugeBarFill.png");
  skin.LoadImage("GaugeBarFront", "Images\\GaugeBarFront.png");
  skin.LoadImage("JudgeJC", "Images\\Judge-JC.png");
  skin.LoadImage("JudgeJ", "Images\\Judge-J.png");
  skin.LoadImage("JudgeA", "Images\\Judge-A.png");
  skin.LoadImage("JudgeM", "Images\\Judge-M.png");
  skin.LoadImage("JusticeCritical", "Images\\JusticeCritical.png");
  skin.LoadImage("Justice", "Images\\Justice.png");
  skin.LoadImage("Attack", "Images\\Attack.png");
  skin.LoadImage("Miss", "Images\\Miss.png");
  
  skin.LoadImage("MusicSelectFrame", "Images\\MusicSelect-Frame.png");
  skin.LoadImage("LevelFrame", "Images\\LevelIndicator.png");
  skin.LoadImage("CursorSetting", "Images\\Setting-Cursor.png");          // 1024x48
  skin.LoadImage("CursorMenu", "Images\\MenuCursor.png");                 // 320x64
  skin.LoadImage("CursorCategory", "Images\\MusicSelect-Category.png");   // 480x40
  skin.LoadImage("CursorMusic", "Images\\MusicSelect-Music.png");         // 480x40
  
  skin.LoadImage("CharacterBack", "Images\\CharacterBack.png");           // 314x302
  skin.LoadImage("Ready1", "Images\\Ready1.png");                  // 1280x144
  skin.LoadImage("Ready2", "Images\\Ready2.png");                  // 1280x144
  
  skin.LoadFont("Normal32", "ＭＳ ゴシック", 28, 1, FontType::AntiAliasing);
  skin.LoadFont("Normal64", "ＭＳ ゴシック", 62, 1, FontType::AntiAliasing);
  skin.LoadFont("Latin128", "ＭＳ ゴシック", 96, 1, FontType::AntiAliasing);
  skin.LoadFont("Combo192", "ＭＳ ゴシック", 192, 1, FontType::AntiAliasing);
  
  skin.LoadAnime("*Anime-Tap", "Images\\Effect-Tap.png", 8, 8, 256, 256, 60, 0.0166);
  skin.LoadAnime("*Anime-ExTap", "Images\\Effect-ExTap.png", 8, 4, 256, 256, 30, 0.0166);
  skin.LoadAnime("*Anime-AirAction", "Images\\Effect-AirAction.png", 8, 6, 256, 256, 48, 0.008);
  skin.LoadAnime("*Anime-SlideTap", "Images\\Effect-SlideTap.png", 8, 8, 256, 256, 60, 0.0166);
  skin.LoadAnime("*Anime-SlideLoop", "Images\\Effect-SlideLoop.png", 4, 4, 256, 256, 16, 0.0166);
  
  skin.LoadSound("*Sound-Tap", "Sounds\\Tap.wav");
  skin.LoadSound("*Sound-ExTap", "Sounds\\ExTap.wav");
  skin.LoadSound("*Sound-Flick", "Sounds\\Flick.wav");
  skin.LoadSound("*Sound-Air", "Sounds\\Air.wav");
  skin.LoadSound("*Sound-AirDown", "Sounds\\AirDown.wav");
  skin.LoadSound("*Sound-AirAction", "Sounds\\AirAction.wav");
  skin.LoadSound("*Sound-AirHoldLoop", "Sounds\\AirHoldLoop.wav");
  skin.LoadSound("*Sound-HoldLoop", "Sounds\\SlideLoop.wav");
  skin.LoadSound("*Sound-HoldStep", "Sounds\\HoldStep.wav");
  skin.LoadSound("*Sound-SlideLoop", "Sounds\\SlideLoop.wav");
  skin.LoadSound("*Sound-SlideStep", "Sounds\\SlideStep.wav");
  skin.LoadSound("*Sound-Metronome", "Sounds\\Metronome.wav");
  skin.LoadSound("SoundCursor", "Sounds\\button01a.mp3");

  Execute("Scripts\\Title.as");
}
