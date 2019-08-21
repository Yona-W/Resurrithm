[EntryPoint]
class Play : CoroutineScene {
  Skin@ skin;
  Image@ imgWhite;
  Image@ imgGCEmpty, imgGCFull, imgGBBack, imgGBFill, imgGBFront;
  Font@ font32, font64, fontLatin;
  ScenePlayer@ player;
  CharacterInfo charinfo;
  TextSprite@ txtCombo;
  ScoreInfo@ scoreinfo;
  Sprite@ spBack;
  MovieSprite@ spMovie;
  double movieoffset = 0.0;

  double judgeLineBegin, judgeLineWidth;
  double judgeLineY;

  void Initialize() {
    LoadResources();
    ExecuteScene(charinfo);
  }

  void Run() {
    RunCoroutine(Coroutine(RunPlayer), "Player:RunPlayer");
    RunCoroutine(Coroutine(Main), "Player:Main");
    RunCoroutine(Coroutine(KeyInput), "Player:KeyInput");
    YieldTime(1);
    while(true) YieldTime(30);
  }

  void Draw() {

  }

  void LoadResources() {
    @skin = GetSkin();
    @fontLatin = skin.GetFont("Latin128");
    @font32 = skin.GetFont("Normal32");
    @font64 = skin.GetFont("Normal64");
    @imgWhite = skin.GetImage("TitleBack");
    @imgGCEmpty = skin.GetImage("GaugeCountEmpty");
    @imgGCFull = skin.GetImage("GaugeCountFull");
    @imgGBBack = skin.GetImage("GaugeBarBack");
    @imgGBFill = skin.GetImage("GaugeBarFill");
    @imgGBFront = skin.GetImage("GaugeBarFront");
    @imgJC = skin.GetImage("JusticeCritical");
    @imgJ = skin.GetImage("Justice");
    @imgA = skin.GetImage("Attack");
    @imgM = skin.GetImage("Miss");
    if(!IsMoverFunctionRegistered("txtCombo_scale")) {
      RegisterMoverFunction("txtCombo_scale", "1.63265*(1.0625-pow(progress - 0.5, 4))");
    }
  }

  void SetPlayerResource() {
    player.SetResource("LaneHoldLight", skin.GetImage("*Lane-HoldLight"));
    player.SetResource("Tap", skin.GetImage("*Note-Tap"));
    player.SetResource("ExTap", skin.GetImage("*Note-ExTap"));
    player.SetResource("Air", skin.GetImage("*Note-Air"));
    player.SetResource("AirUp", skin.GetImage("*Note-AirUp"));
    player.SetResource("AirDown", skin.GetImage("*Note-AirDown"));
    player.SetResource("Flick", skin.GetImage("*Note-Flick"));
    player.SetResource("HellTap", skin.GetImage("*Note-HellTap"));
    player.SetResource("Hold", skin.GetImage("*Note-Hold"));
    player.SetResource("HoldStep", skin.GetImage("*Note-HoldStep"));
    player.SetResource("HoldStrut", skin.GetImage("*Note-HoldStrut"));
    player.SetResource("Slide", skin.GetImage("*Note-Slide"));
    player.SetResource("SlideStep", skin.GetImage("*Note-SlideStep"));
    player.SetResource("SlideStrut", skin.GetImage("*Note-SlideStrut"));
    player.SetResource("AirAction", skin.GetImage("*Note-AirAction"));
    SetPlayerSoundResource("Tap", false);
    SetPlayerSoundResource("ExTap", false);
    SetPlayerSoundResource("Flick", false);
    SetPlayerSoundResource("Air", false);
    SetPlayerSoundResource("AirDown", false);
    SetPlayerSoundResource("AirAction", false);
    SetPlayerSoundResource("AirHoldLoop", true);
    SetPlayerSoundResource("HoldLoop", true);
    SetPlayerSoundResource("HoldStep", false);
    SetPlayerSoundResource("SlideLoop", true);
    SetPlayerSoundResource("SlideStep", false);
    SetPlayerSoundResource("Metronome", false);
    player.SetResource("EffectTap", skin.GetAnime("*Anime-Tap"));
    player.SetResource("EffectExTap", skin.GetAnime("*Anime-ExTap"));
    player.SetResource("EffectAirAction", skin.GetAnime("*Anime-AirAction"));
    player.SetResource("EffectSlideTap", skin.GetAnime("*Anime-SlideTap"));
    player.SetResource("EffectSlideLoop", skin.GetAnime("*Anime-SlideLoop"));

    Container@ ctnLane = Container();
    player.SetLaneSprite(ctnLane);

    {
      Sprite@ sp = Sprite(skin.GetImage("*Lane-Ground"));
      sp.Apply("x:0, y:0, z:0");
      ctnLane.AddChild(sp);
    }

    @txtCombo = TextSprite(skin.GetFont("Combo192"), "");
    txtCombo.Apply("x:512, y:3200, z:1, r:255, g:255, b:255");
    txtCombo.SetAlignment(TextAlign::Center, TextAlign::Center);
    ctnLane.AddChild(txtCombo);

    {
      dictionary dict = {
        { "z", 1 },
        { "r", 255 },
        { "g", 255 },
        { "b", 255 },
        { "width", 2 },
        { "height", 4224 }
      };
      SettingItem@ stItem = GetSettingItem("Graphic", "DivisionLine");
      int divcnt = (stItem !is null)? parseInt(stItem.GetItemText()) : 8;
      for(int i=0; i<=divcnt; ++i) {
        Shape@ line = Shape();
        line.Apply(dict);
        line.Type = ShapeType::BoxFill;
        line.SetPosition(1024*i/divcnt, 2112);
        ctnLane.AddChild(line);
      }
    }

    {
      ScenePlayerMetrics metrics;
      player.GetMetrics(metrics);
      Image@ imgLine = skin.GetImage("*Lane-JudgeLine");
      Sprite@ sp = Sprite(imgLine);
      sp.Apply("x:0, y:3840, z:3, origY:"+(imgLine.Height/2));
      ctnLane.AddChild(sp);
    }
  }

  void SetPlayerSoundResource(string name, bool isLoop) {
    Sound@ se = skin.GetSound("*Sound-" + name);
    se.SetVolume(parseFloat(GetSettingItem("Sound", "Volume" + name).GetItemText()));
    se.SetLoop(isLoop);
    player.RegisterResource(name, se);
  }

  void RunPlayer() {
    @player = ScenePlayer();
    SetPlayerResource();
    player.Initialize();

    ScenePlayerMetrics metrics;
    player.GetMetrics(metrics);
    judgeLineBegin = metrics.JudgeLineLeftX + (1280 / 2);
    judgeLineWidth = metrics.JudgeLineRightX - metrics.JudgeLineLeftX;
    judgeLineY = metrics.JudgeLineLeftY;

    player.SetAbility(GetSkillManager().GetSkill(0).GetDetail(0));
    player.SetJudgeCallback(JudgeCallback(OnJudge));
    charinfo.InitInfo(player);
    player.Apply("z:5");
    AddSprite(player);
    int scoreIndex = GetIntData("Player::ScoreIndex");
    CategoryInfo@ cat = GetMusicManager().GetCategoryInfo(GetIntData("Player::CatIndex"));
    MusicInfo@ mus = (cat is null)? null : cat.GetMusicInfo(GetIntData("Player::MusIndex"));
    @scoreinfo = (mus is null)? null : mus.GetScoreInfo(GetIntData("Player::ScoreIndex"));
    if (scoreinfo is null) {
      WriteLog(Severity::Critical, "譜面データの取得に失敗しました");
      Fire("Player:Exit");
      return;
    }

    SetBackgroundSprite(scoreinfo);

    player.Load(scoreinfo.Path);
    while(!player.IsLoadCompleted()) YieldFrame(1);
    if (player.IsScoreLoaded()) {
      SetMusicInfo();
      Fire("Player:Ready");
      player.GetReady();
      YieldTime(5);
      player.Play();
    } else {
      WriteLog(Severity::Error, "譜面のロードに失敗しました");
      Fire("Player:Exit");
      return;
    }
  }

  Sprite@ spTopCover, spCustomBack;
  array<Sprite@> spGaugeCounts(10);
  Sprite@ spBarBack, spBarFront, spJacket;
  ClipSprite@ spBarFill;
  TextSprite@ txtScore, txtMaxCombo, txtScoreInfo, txtMaxComboInfo;
  TextSprite@ txtTitle, txtArtist, txtLevel;
  SynthSprite@ spJudges;
  void Main() {
    @spTopCover = Sprite(skin.GetImage("PlayerTopCover"));
    spTopCover.Apply("z:10");
    // スコアなど
    @txtMaxCombo = TextSprite(font64, "0");
    txtMaxCombo.Apply("x:630, y:8, scaleX: 0.6, scaleY: 0.6, z:15");
    txtMaxCombo.SetAlignment(TextAlign::Right, TextAlign::Top);

    @txtMaxComboInfo = TextSprite(font64, "Max Combo");
    txtMaxComboInfo.Apply("x:390, y:10, scaleX: 0.5, scaleY: 0.5, z:15");

    @txtScore = TextSprite(font64, "0");
    txtScore.Apply("x:890, y:8, scaleX: 0.6, scaleY: 0.6, z:15");
    txtScore.SetAlignment(TextAlign::Right, TextAlign::Top);

    @txtScoreInfo = TextSprite(font64, "Score");
    txtScoreInfo.Apply("x:650, y:10, scaleX: 0.5, scaleY: 0.5, z:15");

    @txtLevel = TextSprite(font64, "");
    txtLevel.Apply("x:1276, y: 4, z: 15");
    txtLevel.SetAlignment(TextAlign::Right, TextAlign::Top);

    @txtTitle = TextSprite(font64, "");
    txtTitle.Apply("x:1008, y: 14, z: 15");
    txtTitle.SetRangeScroll(210, 20, 32);

    @txtArtist = TextSprite(font32, "");
    txtArtist.Apply("x: 1008, y: 74, z: 15");
    txtArtist.SetRangeScroll(210, 20, 32);

    @spJacket = Sprite();
    spJacket.Apply("x: 908, y: 4, z: 15");
    spJacket.HasAlpha = false;

    @spJudges = SynthSprite(768, 24);
    spJudges.Transfer(skin.GetImage("JudgeJC"), 0, 0);
    spJudges.Transfer(skin.GetImage("JudgeJ"), 192, 0);
    spJudges.Transfer(skin.GetImage("JudgeA"), 416, 0);
    spJudges.Transfer(skin.GetImage("JudgeM"), 564, 0);
    array<TextSprite@> spCounts = {
      TextSprite(font32, "0"),
      TextSprite(font32, "0"),
      TextSprite(font32, "0"),
      TextSprite(font32, "0")
    };
    spCounts[0].Apply("x: 520, y: -3, scaleX: 0.8, scaleY: 0.6, z: 15");
    spCounts[1].Apply("x: 650, y: -3, scaleX: 0.8, scaleY: 0.6, z: 15");
    spCounts[2].Apply("x: 790, y: -3, scaleX: 0.8, scaleY: 0.6, z: 15");
    spCounts[3].Apply("x: 888, y: -3, scaleX: 0.8, scaleY: 0.6, z: 15");
    spJudges.Apply("x:640, y:0, z:15, scaleX: 0.6666, scaleY: 0.58, origX: 384");
    for(int i = 0; i < 4; i++) spCounts[i].SetAlignment(TextAlign::Right, TextAlign::Top);
    // ゲージ
    for(int i = 0; i < 10; i++) {
      @spGaugeCounts[i] = Sprite(imgGCEmpty);
      spGaugeCounts[i].Apply("x:" + (384 + i * 32) + ", y:90, z:11, scaleX:0.5, scaleY:0.3");
      AddSprite(spGaugeCounts[i]);
    }
    @spBarBack = Sprite(imgGBBack);
    @spBarFront = Sprite(imgGBFront);
    @spBarFill = ClipSprite(512, 64);
    spBarFill.Transfer(imgGBFill, 0, 0);
    spBarFill.SetRange(0, 0, 0, 1);
    spBarBack.Apply("x:384, y:40, z:15, scaleY: 0.75");
    spBarFill.Apply("x:384, y:40, z:16, scaleY: 0.75");
    spBarFront.Apply("x:384, y:40, z:17, scaleY: 0.75");

    AddSprite(spTopCover);
    AddSprite(txtMaxCombo);
    AddSprite(txtScore);
    AddSprite(txtMaxComboInfo);
    AddSprite(txtScoreInfo);
    AddSprite(txtTitle);
    AddSprite(txtArtist);
    AddSprite(txtLevel);
    AddSprite(spJacket);
    AddSprite(spJudges);
    for(int i = 0; i < 4; i++) AddSprite(spCounts[i]);
    AddSprite(spBarBack);
    AddSprite(spBarFill);
    AddSprite(spBarFront);

    DrawableResult dsNow, dsPrev;
    MoverObject @txtComboMover = MoverObject();
    txtComboMover.Apply("time:0.2, func:txtCombo_scale");
    while(true) {
      player.GetCurrentResult(dsNow);
      if (dsNow.FulfilledGauges > dsPrev.FulfilledGauges) {
        if (dsNow.FulfilledGauges > 10) {
          // TODO: 満杯のときの処理
        } else {
          for(uint i = dsPrev.FulfilledGauges; i < dsNow.FulfilledGauges; i++) {
            spGaugeCounts[i].SetImage(imgGCFull);
            spGaugeCounts[i].Apply("scaleX:0.8");
            spGaugeCounts[i].AddMove("scaleX:{end:0.5, time:0.3}");
            spGaugeCounts[i].AddMove("scaleY:{end:0.3, time:0.3}");
          }
        }
      }
      if (dsNow.CurrentGaugeRatio != dsPrev.CurrentGaugeRatio) {
        spBarFill.AddMove("u2:{end:" + dsNow.CurrentGaugeRatio + ", time:0.1, func:out_sine}");
      }
      if (dsNow.Score != dsPrev.Score) {
        txtScore.SetText(formatInt(dsNow.Score, "", 8));
      }
      if (dsNow.MaxCombo != dsPrev.MaxCombo) {
        txtMaxCombo.SetText(formatInt(dsNow.MaxCombo, "", 5));
      }
      if (dsNow.Combo > dsPrev.Combo && dsNow.Combo >= 5) {
        txtCombo.SetText("" + dsNow.Combo);
        txtCombo.AbortMove(true);
        txtCombo.AddMove("scale", txtComboMover);
      }


      spCounts[0].SetText("" + dsNow.JusticeCritical);
      spCounts[1].SetText("" + dsNow.Justice);
      spCounts[2].SetText("" + dsNow.Attack);
      spCounts[3].SetText("" + dsNow.Miss);
      dsPrev = dsNow;
      YieldFrame(1);
    }
  }

  void SetMusicInfo() {
    if (scoreinfo is null) return;
    
    Image@ imgJacket = Image(scoreinfo.JacketPath);
    if (imgJacket !is null) {
        int w = imgJacket.Width, h = imgJacket.Height;
        if (w > 0 && h > 0) {
          spJacket.SetScale(98.0 / w, 98.0 / h);
        }
    }
    spJacket.SetImage(imgJacket);
    txtTitle.SetText(scoreinfo.Title);
    txtArtist.SetText(scoreinfo.Artist);
    txtLevel.SetText("" + scoreinfo.Level);
  }

  bool isPausing;
  void KeyInput() {
    while(true) {
      if (IsKeyTriggered(Key::INPUT_ESCAPE)) {
        if (Execute("Scripts\\Select.as")) {
          Fire("Player:End");
          Disappear();
        }
      }
      if (IsKeyTriggered(Key::INPUT_LEFT)) {
        player.MovePositionByMeasure(-1);
      }
      if (IsKeyTriggered(Key::INPUT_RIGHT)) {
        player.MovePositionByMeasure(1);
      }
      if (IsKeyTriggered(Key::INPUT_SPACE)) {
        if (isPausing) {
          player.Resume();
        } else {
          player.Pause();
        }
        isPausing = !isPausing;
      }
      if (IsKeyTriggered(Key::INPUT_R) && IsKeyHeld(Key::INPUT_LCONTROL)) {
        player.Reload();
      }

      YieldFrame(1);
    }
  }

  void SetBackgroundSprite(ScoreInfo@ score) {
    if (score !is null) {
      if (score.MoviePath != "") {
        Movie@ movie = Movie(score.MoviePath);
        if (movie !is null && movie.Width > 0 && movie.Height > 0) {
          WriteLog(Severity::Info, "カスタム背景動画: " + score.MoviePath);
          @spMovie = MovieSprite(movie);
          movieoffset = score.MovieOffset;
          spMovie.Apply("z:-29");
          spMovie.SetScale(1280.0 / movie.Width, 720.0 / movie.Height);
          AddSprite(spMovie);

          // TODO: このやり方は不適切 譜面再生と同期するようにする
          RunCoroutine(Coroutine(PlayBackgroundMovie), "Player:PlayBackgroundMovie");

          return;
        }
      }

      if (score.BackgroundPath != "") {
        Image@ imgBack = Image(score.BackgroundPath);
        if (imgBack !is null && imgBack.Width > 0 && imgBack.Height > 0) {
          WriteLog(Severity::Info, "カスタム背景画像: " + score.BackgroundPath);
          @spBack = Sprite(imgBack);
          spBack.Apply("z:-29");
          spBack.SetScale(1280.0 / imgBack.Width, 720.0 / imgBack.Height);
          AddSprite(spBack);

          return;
        }
      }
    }

    @spBack = Sprite(imgWhite);
    spBack.Apply("z:-29");
    spBack.SetScale(1280.0 / imgWhite.Width, 720.0 / imgWhite.Height);
    AddSprite(spBack);
  }

  void PlayBackgroundMovie() {
    if (spMovie is null) return;

    if (movieoffset < 0.0) {
      spMovie.SetTime(-movieoffset);
    } else if (movieoffset > 0.0) {
      YieldTime(-movieoffset / 1000.0);
    }
    spMovie.Play();
  }

  void OnEvent(const string &in event) {
    if (event == "Player:Exit") {
      Execute("Scripts\\Select.as");
      Disappear();
    }
  }

  // 判定発生ごとに呼ばれるコールバック
  Image@ imgJC, imgJ, imgA, imgM;
  void OnJudge(JudgeData data, const string &in ex) {
    Sprite@ sp;
    switch (data.Judge) {
      case JudgeType::JusticeCritical:
        @sp = Sprite(imgJC);
        break;
      case JudgeType::Justice:
        @sp = Sprite(imgJ);
        break;
      case JudgeType::Attack:
        @sp = Sprite(imgA);
        break;
      case JudgeType::Miss:
        @sp = Sprite(imgM);
        break;
    }
    sp.Apply(dictionary = {
      { "x", judgeLineBegin + judgeLineWidth / 16 * (data.Left + (data.Right - data.Left) / 2) },
      { "y" , judgeLineY - 170 },
      { "z", 10 },
      { "origX", 96 },
      { "origY", 32 },
      { "scaleX", 0.5 },
      { "scaleY", 0.5 },
      { "alpha", 0 }
    });
    sp.AddMove("y:{@end:-150, time:0.5, func:out_sine}");
    sp.AddMove("alpha:{begin:0.0, end:1.0, time:0.5}");
    sp.AddMove("death:{wait:0.6}");
    AddSprite(sp);
  }
}

class CharacterInfo : CoroutineScene {
  Skin@ skin;
  dictionary icons;
  Container@ container;
  Sprite@ spBack, spCharacter, spIcon;
  TextSprite@ spSkill, spDescription;

  void Initialize() {
    @skin = GetSkin();
    // InitInfo();
    InitReady();
  }

  void InitInfo(ScenePlayer@ player) {
    player.SetSkillCallback(SkillCallback(OnSkill));

    @container = Container();
    @spBack = Sprite(skin.GetImage("CharacterBack"));
    spBack.Apply("z:-1");

    @spCharacter = Sprite();
    spCharacter.Apply("x:8, y: 6");
    CharacterImages@ cimg = GetCharacterManager().CreateCharacterImages();
    if (cimg !is null) cimg.ApplySmallImage(spCharacter);

    @spSkill = TextSprite(skin.GetFont("Normal32"), "");
    spSkill.Apply("x:11, y: 180, r: 0, g: 0, b: 0, scaleX: 0.75, scaleY: 0.75");
    Skill@ skill = GetSkillManager().GetSkill(0);
    if(skill !is null) spSkill.SetText(skill.Name);

    @spDescription = TextSprite(skin.GetFont("Normal32"), "");
    spDescription.Apply("x:11, y: 208, r: 0, g: 0, b: 0, scaleX: 0.5, scaleY: 0.5");
    if(skill !is null) spDescription.SetText(skill.GetDetail(0).Description);
    spDescription.SetRich(true);

    container.AddChild(spBack);
    container.AddChild(spCharacter);
    container.AddChild(spSkill);
    container.AddChild(spDescription);

    @spIcon = Sprite();
    if(skill !is null) spIcon.SetImage(Image(skill.IconPath));
    spIcon.Apply("x:217, y:180, scaleX:0.75, scaleY:0.75");
    container.AddChild(spIcon);

    dictionary@ indicators = skill.GetDetail(0).Indicators;
    array<string>@ keys = indicators.getKeys();
    for(uint i = 0; i < keys.length(); ++i) {
      Sprite@ icon = Sprite(Image(string(indicators[keys[i]])));
      int ix = 276 - (i * 28);
      icon.Apply("scaleX:0.25, scaleY:0.25, origX:48, origY:48, y:164, x:" + ix);
      @icons[keys[i]] = icon;
      container.AddChild(icon);
    }

    container.Apply("x:-296, y: 110, z:30");
    AddSprite(container);
  }

  Sprite@ spReadyBack, spReadyFront;
  TextSprite@ spReady;
  void InitReady() {
    @spReadyBack = Sprite(skin.GetImage("Ready1"));
    spReadyBack.Apply("origX:640, origY:72, x:640, y:360, z:40, scaleY:0");

    @spReadyFront = Sprite(skin.GetImage("Ready2"));
    spReadyFront.Apply("origY:72, x:-1280, y:360, z:41");

    @spReady = TextSprite(skin.GetFont("Normal64"), "ARE YOU READY?");
    spReady.SetAlignment(TextAlign::Center, TextAlign::Center);
    spReady.Apply("x:640, y:360, z:41, scaleX:0");

    AddSprite(spReadyBack);
    AddSprite(spReadyFront);
    AddSprite(spReady);
  }

  void ReadyString() {
    spReady.AddMove("scale:{end:1.4, time:0.3, wait:0.7, func:out_back}");
    spReady.AddMove("scaleY:{end:0, time:0.1, wait:2.0, func:out_sine}");
    YieldTime(2.1);
    spReady.SetText("START!");
    spReady.AddMove("scale:{end:1.4, time:0.3, wait:0.7, func:out_back}");
    spReady.AddMove("scaleY:{end:1.4, time:0.5, wait:2, func:out_sine}");
    spReady.AddMove("alpha:{begin:1, end:0, time:0.5, wait:2}");
    spReady.AddMove("death:{wait:2.5}");
  }

  void Run() {
    while(true) YieldFrame(100);
  }

  void OnSkill(const string &in index) {
    Sprite@ target;
    if(icons.get(index, @target)) { 
      target.AbortMove(true);
      target.Apply("scaleX:0.3, scaleY: 0.3");
      target.AddMove("scale:{end:0.25, time: 0.2}");
    }
  }

  void OnEvent(const string &in event) {
    if (event == "Player:End") Disappear();
    if (event == "Player:Ready") {
      container.AddMove("x:{@end:296, time:0.5, func:out_quad}");
      spReadyBack.AddMove("scale:{end:1, time:0.3, func:out_sine}");
      spReadyFront.AddMove("x:{@end:1280, time:0.5, wait:0.3, func:out_sine}");
      RunCoroutine(Coroutine(ReadyString), "CharInfo:Ready");
      // 4.1秒後にフェードアウト開始
      spReadyBack.AddMove("scaleY:{end:0, time:0.3, wait:4.1, func:in_back}");
      spReadyFront.AddMove("alpha:{begin:1, end:0, time:0.3, wait:4.1}");
      spReadyBack.AddMove("death:{wait:4.4}");
      spReadyFront.AddMove("death:{wait:4.4}");
    }
  }
}
