[EntryPoint]
class Title : CoroutineScene {
  Skin@ skin;
  Font@ fontLatin, font32, font64;
  Image@ imgWhite, imgDxLib, imgBoost, imgFreeType, imgAngelScript, imgSeaurchin;
  Image@ imgCursorMenu;

  Container@ cntLogos;
  bool introWorking;

  void Initialize() {
    LoadResources();
    AddSprite(Sprite(imgWhite));
  }

  void Run() {
    @cntLogos = Container();
    AddSprite(cntLogos);

    if (!ExistsData("LogoShown")) {
      SetData("LogoShown", true);
      RunCoroutine(Coroutine(Intro), "Title:Intro");
      introWorking = true;
    } else {
      introWorking = false;
    }
    while(introWorking && !IsKeyTriggered(Key::INPUT_RETURN)) YieldFrame(1);
    cntLogos.AddMove("alpha:{begin:1, end:0, time:0.75}");
    cntLogos.AddMove("death:{wait:0.75}");

    RunCoroutine(Coroutine(TitleRipple), "Title:Ripple");
    RunCoroutine(Coroutine(KeyInput), "Title:KeyInput");
    while(true) YieldTime(30);
  }

  void Draw() {

  }

  void LoadResources() {
    @skin = GetSkin();
    @fontLatin = skin.GetFont("Latin128");
    @font32 = skin.GetFont("Normal32");
    @font64 = skin.GetFont("Normal64");

    @imgWhite = skin.GetImage("White");
    @imgDxLib = skin.GetImage("LogoSdl");
    @imgBoost = skin.GetImage("LogoBoost");
    @imgFreeType = skin.GetImage("LogoFreeType");
    @imgAngelScript = skin.GetImage("LogoAngelScript");
    @imgSeaurchin = skin.GetImage("LogoResurrithm");
    @imgCursorMenu = skin.GetImage("CursorMenu");
    @smCursor = skin.GetSound("SoundCursor");
    @mixSE = GetDefaultMixer("SE");
  }

  //ここからコルーチン
  void Intro() {

    array<MoverObject@> moverList(3);
    @moverList[0] = MoverObject();
    moverList[0].Apply("begin:0, end:1, time:1");

    @moverList[1] = MoverObject();
    moverList[1].Apply("begin:1, end:0, time:1, wait:3");

    @moverList[2] = MoverObject();
    moverList[2].Apply("wait", 4);

    array<string> moverKeyList = { "alpha", "alpha", "death", "x" };

    array<Sprite@> dxl = {
      TextSprite(font64, "Powered by"),
      Sprite(imgDxLib)
    };
    dxl[0].Apply("x:400, y:100, r:0, g:0, b: 0, alpha:0");
    dxl[1].Apply("x:700, y:200, origY:100, alpha:0");
    dxl[1].HasAlpha = false;
    for(int i = 0; i < dxl.length(); i++) {
      for(uint j = 0; j < moverList.length(); ++j) {
        dxl[i].AddMove(moverKeyList[j], moverList[j]);
      }
      cntLogos.AddChild(dxl[i]);
    }

    array<Sprite@> logo = {
      Sprite(imgBoost),
      Sprite(imgFreeType),
      Sprite(imgAngelScript)
    };
    logo[0].Apply("x:200, y:300, alpha:0");
    logo[1].Apply("x:516, y:300, alpha:0");
    logo[2].Apply("x:206, y:500, alpha:0");
    for(int i = 0; i < logo.length(); i++) {
      for(uint j = 0; j < moverList.length(); ++j) {
        logo[i].AddMove(moverKeyList[j], moverList[j]);
      }
      cntLogos.AddChild(logo[i]);
    }

    YieldTime(4.0);
    introWorking = false;
  }

  Sprite@ spLogo, spCursor;
  ClipSprite@ spTitle;
  array<Sprite@> menu(3);
  Sound@ smCursor;
  SoundMixer@ mixSE;
  int mcur = 0;
  void TitleRipple() {
    @spLogo = Sprite(imgSeaurchin);
    spLogo.Apply("origX:160, origY:160, x:640, y:360, z:2, alpha:0");
    spLogo.AddMove("alpha:{begin:0, end:1, time:0.5}");
    AddSprite(spLogo);

    @spTitle = ClipSprite(1000, 256);
    TextSprite @buffer = TextSprite(font64, "Resurrithm");
    TextSprite @buffer2 = TextSprite(font32, "Based on Seaurchin");
    buffer.Apply("r:0, g:0, b:0");
    buffer2.Apply("r:0, g:0, b:0, y:55");
    spTitle.Transfer(buffer);
    spTitle.Transfer(buffer2);
    spTitle.Apply("origY:64, origX:0, x:700, y:360, z:1, alpha:0");
    spTitle.SetRange(0, 0, 0, 1);
    AddSprite(spTitle);

    @menu[0] = TextSprite(font64, "Start");
    @menu[1] = TextSprite(font64, "Settings");
    @menu[2] = TextSprite(font64, "Exit");

    @spCursor = Sprite(imgCursorMenu);
    spCursor.Apply("x:-320, y:" + (400 + 64 * mcur));
    AddSprite(spCursor);

    for(int i = 0; i < 3; i++) {
      menu[i].Apply(dictionary = {
        {"x", -50}, {"y", 400 + 64 * i},
        {"origX", 160},
        {"r", 0}, {"g", 0}, {"b", 0}
      });
      AddSprite(menu[i]);
    }

    array<MoverObject@> moverList(3);
    @moverList[0] = MoverObject();
    moverList[0].Apply("end", 0);
    moverList[0].Apply("time", 1);

    @moverList[1] = moverList[0].Clone();
    moverList[1].Apply("end", 1);

    @moverList[2] = moverList[1].Clone();
    moverList[2].Clear();
    moverList[2].Apply("wait", 1.0);

    dictionary moveList = {
      { "alpha", moverList[0] },
      { "scale", moverList[1] },
      { "death", moverList[2] }
    };

    while(true) {
      YieldTime(2);
      auto ripple = spLogo.Clone();
      ripple.Apply("alpha:0.5");
      array<string> keys = moveList.getKeys();
      for(uint i = 0; i < keys.length(); ++i) {
        ripple.AddMove(keys[i], cast<MoverObject@>(moveList[keys[i]]));
      }
      AddSprite(ripple);
    }
  }

  void KeyInput() {
    while(true) {
      if (IsKeyTriggered(Key::INPUT_RETURN)) {
        spLogo.AddMove("x:{func:out_sine, @end:-260, time:0.5}");
        spLogo.AddMove("scaleX:{func:out_sine, end:0.4, time:0.5}");
        spLogo.AddMove("scaleY:{func:out_sine, end:0.4, time:0.5}");
        spLogo.AddMove("y:{func:out_sine, @end:-160, time:0.5, wait:0.5}");
        spTitle.AddMove("alpha:{func:out_sine, @end:1, time:0.5}");
        spTitle.AddMove("x:{func:out_sine, @end:-200, time:0.5}");
        spTitle.AddMove("u2:{func:out_sine, end:1.0, time:0.5}");
        spTitle.AddMove("v2:{func:out_sine, end:1.0, time:0.5}");
        spTitle.AddMove("y:{func:out_sine, @end:-140, time:0.5, wait:0.5}");
        YieldTime(1.0);
        break;
      }
      YieldFrame(1);
    }
    for(int i = 0; i < 3; i++) menu[i].AddMove("x:{@end:720, time:0.25, func:out_sine}");
    spCursor.AddMove("x:{@end:800, time:0.25, func:out_sine}");
    while(true) {
      if (IsKeyTriggered(Key::INPUT_UP)) {
        mcur = (mcur + 2) % 3;
        mixSE.Play(smCursor);
        spCursor.AddMove("x:{time:0.1, end:480}");
        dictionary dict = { { "time", 0.1 }, { "end", 400 + 64 * mcur }, { "func", "in_sine" } };
        spCursor.AddMove("y", dict);
      }
      if (IsKeyTriggered(Key::INPUT_DOWN)) {
        mcur = (mcur + 1) % 3;
        mixSE.Play(smCursor);
        spCursor.AddMove("x:{time:0.1, end:480}");
        dictionary dict = { { "time", 0.1 }, { "end", 400 + 64 * mcur } };
        spCursor.AddMove("y", dict);
      }
      if (IsKeyTriggered(Key::INPUT_RETURN)) {
        switch(mcur) {
          case 0:
            if (Execute("Select.as")) Disappear();
            break;
          case 1:
            if (Execute("Setting.as")) Disappear();
            break;
          case 2:
            ExitApplication();
            break;
        }

      }
      YieldFrame(1);
    }
  }
}
