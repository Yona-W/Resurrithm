[EntryPoint]
class Select : CoroutineScene {
  Skin@ skin;
  CategoryManager@ categoryManager;
  CategoryItem@ currentCategory;
  Font@ font32, font64, fontLatin;
  Image@ imgWhite, imgBarMusic, imgMusicFrame, imgLevel;

  void Initialize() {
    LoadResources();
    AddSprite(Sprite(imgWhite));
  }

  void Run() {
    @categoryManager = CategoryManager();
    @currentCategory = null;
    ExecuteScene(CharacterSelect());
    RunCoroutine(Coroutine(Main), "Select:Main");
    RunCoroutine(Coroutine(KeyInput), "Select:KeyInput");
    while(true) YieldTime(30);
  }

  void LoadResources() {
    @skin = GetSkin();
    @fontLatin = skin.GetFont("Latin128");
    @font32 = skin.GetFont("Normal32");
    @font64 = skin.GetFont("Normal64");
    @imgWhite = skin.GetImage("White");
    @imgBarMusic = skin.GetImage("CursorMusic");
    @imgMusicFrame = skin.GetImage("MusicSelectFrame");
    @imgLevel = skin.GetImage("LevelFrame");
  }

  //ここからコルーチン
  array<MusicFrame@> musics(5);
  void Main() {
    for(int i = 0; i < 5; i++) {
      @musics[i] = MusicFrame(imgMusicFrame, imgLevel, font64, font32);
      musics[i].cMain.Apply("y: 360, z:2, x:" + (-320.0 + 480 * i));
      AddSprite(musics[i].cMain);
    }
    InitCursor();
    while(true) {
      YieldFrame(1);
    }
  }

  void UpdateInfoAt(uint obj, int index) {
    if (currentCategory is null) {
      CategoryItem@ item = categoryManager.GetItem(index);
      if (item !is null) musics[obj].UpdateInfo(item.GetInstance());
    } else {
      musics[obj].UpdateInfo(currentCategory.GetItem(index));
    }
  }

  int center = 2;

  void InitCursor() {
    for(uint i = 0; i < 5; i++) UpdateInfoAt(i, ((i + 2 - center + 5) % 5 - 2));
  }

  void UpdateCursor(int adjust) {
    for(int i = 0; i < 5; i++) {
      musics[i].cMain.AbortMove();
      musics[i].cMain.Apply("x:" + (640 + 480 * ((i + 2 - center + 5) % 5 - 2)));
    }
    uint flew = (5 + center - adjust * 2) % 5;
    musics[flew].cMain.Apply("x:" + (640 + 480 * adjust * 3));

    UpdateInfoAt(flew, adjust * 2);

    center = (5 + center + adjust) % 5;
    for(int i = 0; i < 5; i++) musics[i].cMain.AddMove("x:{@end:" + (480 * -adjust) + ", time:0.2, func:out_quad}");
  }

  bool isEnabled = true;
  void KeyInput() {
    while(true) {
      if (!isEnabled) {
        YieldFrame(1);
        continue;
      }

      if (IsKeyTriggered(Key::INPUT_RETURN)) {
        if (currentCategory is null) {
          @currentCategory = categoryManager.GetItem(0);
          InitCursor();
        } else {
          Score@ score = currentCategory.GetItem(0);
          if (score is null) return;

          SetData("Player::CatIndex", categoryManager.GetIndex());
          SetData("Player::MusIndex", currentCategory.GetIndex());
          SetData("Player::ScoreIndex", currentCategory.GetSubIndex());
          if (Execute("Scripts\\Play.as")) {
            Fire("Select:End");
            Disappear();
          }
        }
      } else if (IsKeyTriggered(Key::INPUT_ESCAPE)) {
        if (currentCategory is null) {
          if (Execute("Scripts\\Title.as")) {
            Fire("Select:End");
            Disappear();
          }
        } else {
          @currentCategory = null;
          InitCursor();
        }
      } else if (IsKeyTriggered(Key::INPUT_RIGHT)) {
        if (currentCategory is null) {
          categoryManager.Next();
        } else {
          currentCategory.Next();
        }
        UpdateCursor(1);
      } else if (IsKeyTriggered(Key::INPUT_LEFT)) {
        if (currentCategory is null) {
          categoryManager.Prev();
        } else {
          currentCategory.Prev();
        }
        UpdateCursor(-1);
      } else if (IsKeyTriggered(Key::INPUT_UP)) {
        if (currentCategory !is null) {
          currentCategory.NextDiff();
        }
        UpdateCursor(0);
      } else if (IsKeyTriggered(Key::INPUT_DOWN)) {
        if (currentCategory !is null) {
          currentCategory.PrevDiff();
        }
        UpdateCursor(0);
      }

      if (IsKeyTriggered(Key::INPUT_A)) {
        SetData("AutoPlay", 1);
        ShowMessage("オートプレイ: ON");
      }
      if (IsKeyTriggered(Key::INPUT_M)) {
        SetData("AutoPlay", 0);
        ShowMessage("オートプレイ: OFF");
      }
      if (IsKeyTriggered(Key::INPUT_S)) {
        SetData("AutoPlay", 2);
        ShowMessage("オートプレイ: Air/Air-Actionのみ");
      }

      YieldFrame(1);
    }
  }

  void OnEvent(const string &in event) {
    if (event == "Select:Disable") {
      isEnabled = false;
    }
    if (event == "Select:Enable") {
      isEnabled = true;
    }
  }

  void ShowMessage(string mes) {
    TextSprite@ spmes = TextSprite(font32, mes);
    spmes.Apply("y:720, z:20, r:0, g:0, b:0");
    spmes.AddMove("y:{@end:-32, time:0.5, func:out_sine}");
    spmes.AddMove("y:{@end:32, time:0.5, wait:1.0, func:in_sine}");
    spmes.AddMove("death:{wait:2.0}");
    AddSprite(spmes);
  }
}

class CategoryManager {
  array<CategoryItem@> items;
  uint current;
  
  CategoryManager() {
    ResetItems();
  }

  void ResetItems() {
    MusicManager@ manager = GetMusicManager();
    uint size = manager.GetCategorySize();
    items.resize(0); // Clear
    items.resize(size);
    for (uint i=0; i<size; ++i) {
      @items[i] = CategoryItem(manager.GetCategory(i));
    }
    current = 0;
  }
  
  uint GetSize() { return items.length(); }
  uint GetIndex() { return current; }
  void Next() { uint size = GetSize(); if (size == 0) return; current = (current + 1) % size; }
  void Prev() { uint size = GetSize(); if (size == 0) return; current = (current + size - 1) % size; }
  
  CategoryItem@ GetItem(int offset) {
    uint size = GetSize();
    if (size == 0) return null; 
    
    // TODO: 値が大きくなった時に死にやすいので修正する
    uint index = (current + (offset + uint(abs(offset)) * size)) % size;
    return items[index];
  }
}

class CategoryItem {
  Category@ category;
  array<MusicItem@> items;
  uint current;
  
  CategoryItem(Category@ cat) {
    ResetItems(cat);
  }
  
  void ResetItems(Category@ cat) {
    @category = cat;
    
    uint size = (cat is null)? 0 : cat.MusicCount;
    items.resize(0); // Clear
    items.resize(size);
    for (uint i=0; i<size; ++i) {
      @items[i] = MusicItem(cat.GetMusic(i));
    }
    current = 0;
  }
  
  Category@ GetInstance() { return category; }
  uint GetSize() { return items.length(); }
  uint GetIndex() { return current; }
  uint GetSubIndex() { MusicItem@ item = GetRawItem(0); if (item is null) return 0; return item.GetIndex(); }
  void Next() { uint size = GetSize(); if (size == 0) return; current = (current + 1) % size; }
  void Prev() { uint size = GetSize(); if (size == 0) return; current = (current + size - 1) % size; }
  void NextDiff() { MusicItem@ item = GetRawItem(0); if (item is null) return; item.Next(); }
  void PrevDiff() { MusicItem@ item = GetRawItem(0); if (item is null) return; item.Prev(); }
  
  MusicItem@ GetRawItem(int offset) {
    uint size = GetSize();
    if (size == 0) return null; 
    
    // TODO: 値が大きくなった時に死にやすいので修正する
    uint index = (current + (offset + uint(abs(offset)) * size)) % size;
    return items[index];
  }
  
  Score@ GetItem(int offset) {
    MusicItem@ item = GetRawItem(offset);
    if (item is null) return null;
    
    return item.GetItem(0);
  }
}

class MusicItem {
  Music@ music;
  uint current;
  
  MusicItem(Music@ mus) {
    @music = mus;
    current = 0;
  }
  
  Music@ GetInstance() { return music; }
  uint GetSize() { return (music is null)? 0 : music.ScoreCount; }
  uint GetIndex() { return current; }
  void Next() { uint size = GetSize(); if (size == 0) return; current = (current + 1) % size; }
  void Prev() { uint size = GetSize(); if (size == 0) return; current = (current + size - 1) % size; }
  
  Score@ GetItem(int offset) {
    uint size = GetSize();
    if (size == 0) return null; 
    
    // TODO: 値が大きくなった時に死にやすいので修正する
    uint index = (current + (offset + uint(abs(offset)) * size)) % size;
    return music.GetScore(index);
  }
}

class MusicFrame {
  Container@ cMain;
  TextSprite@ title, artist, levelnum, leveltype;
  Sprite@ frame, jacket, level;
  
  MusicFrame(Image@ imgFrame, Image@ imgLevel, Font@ font64, Font@ font32) {
    @cMain = Container();
    
    @frame = Sprite(imgFrame);
    frame.Apply("origX:200, origY:300");
    
    @jacket = Sprite();
    jacket.Apply("x:-160, y:-260");
    jacket.HasAlpha = false;
    
    @title = TextSprite(font64, "");
    title.Apply("y:108, r:0, g:0, b:0");
    title.SetRangeScroll(300, 40, 64);
    title.SetAlignment(TextAlign::Center, TextAlign::Top);

    @artist = TextSprite(font64, "");
    artist.Apply("y:182, r:0, g:0, b:0");
    artist.SetRangeScroll(300, 40, 64);
    artist.SetAlignment(TextAlign::Center, TextAlign::Top);

    @level = Sprite(imgLevel);
    level.Apply("x:100, y:-10");
    @levelnum = TextSprite(font64, "");
    levelnum.SetAlignment(TextAlign::Center, TextAlign::Top);
    levelnum.Apply("x:148, y:26, r:0, g:0, b:0");
    @leveltype = TextSprite(font32, "");
    leveltype.SetAlignment(TextAlign::Center, TextAlign::Top);
    leveltype.Apply("x:148, y:-8, scaleX:0.8");

    cMain.AddChild(frame);
    cMain.AddChild(jacket);
    cMain.AddChild(title);
    cMain.AddChild(artist);
    cMain.AddChild(level);
    cMain.AddChild(levelnum);
    cMain.AddChild(leveltype);
  }
  
  void UpdateInfo(Category@ cat) {
    if (cat is null) return;
    
    title.SetText(cat.Name);
    jacket.Apply("alpha:0");
    artist.Apply("alpha:0");
    level.Apply("alpha:0");
    levelnum.Apply("alpha:0");
    leveltype.Apply("alpha:0");
  }
  
  void UpdateInfo(Score@ score) {
    if (score is null) return;
    
    title.SetText(score.Title);

    Image@ imgJacket = Image(score.JacketPath);
    if (imgJacket !is null) {
        int w = imgJacket.Width, h = imgJacket.Height;
        if (w > 0 && h > 0) {
          jacket.SetScale(320.0 / w, 320.0 / h);
        }
    }
    jacket.SetImage(imgJacket);
    artist.SetText(score.Artist);

    string st = "";
    int stars = score.Level;
    switch(score.Difficulty) {
      case Basic:
        leveltype.SetText("BASIC");
        levelnum.SetText("" + stars + score.DifficultyName);
        break;
      case Advanced:
        leveltype.SetText("ADVANCED");
        levelnum.SetText("" + stars + score.DifficultyName);
        break;
      case Expert:
        leveltype.SetText("EXPERT");
        levelnum.SetText("" + stars + score.DifficultyName);
        break;
      case Master:
        leveltype.SetText("MASTER");
        levelnum.SetText("" + stars + score.DifficultyName);
        break;
      case WorldsEnd:
        for(int i = 0; i < stars; i++) st += "★";
        leveltype.SetText(st);
        levelnum.SetText(score.DifficultyName);
        break;
    }
    
    jacket.Apply("alpha:1");
    artist.Apply("alpha:1");
    level.Apply("alpha:1");
    levelnum.Apply("alpha:1");
    leveltype.Apply("alpha:1");
  }
}

class CharacterSelect : CoroutineScene {
  bool isEnabled;
  Skin@ skin;
  CharacterManager@ cm;
  Character@ ch;
  CharacterImages@ cim;
  SkillManager@ sm;
  Skill@ sk;

  Shape@ spBack;
  Sprite@ spImage, spIcon;
  TextSprite@ spTitle, spInfo, spName, spSkill, spDescription;
  Container@ container;

  void Initialize() {
    @skin = GetSkin();
    @cm = GetCharacterManager();
    @sm = GetSkillManager();
    @container = Container();

    @spBack = Shape();
    spBack.Apply("x:640, y:360, width:1280, height:720, r:100, g:100, b:100, alpha:0.8");

    @spTitle = TextSprite(skin.GetFont("Normal64"), "キャラクター・スキル設定");
    spTitle.SetAlignment(TextAlign::Center, TextAlign::Top);
    spTitle.Apply("x:640, y:12");

    @spInfo = TextSprite(skin.GetFont("Normal32"), "カーソルキー左右でキャラクター変更、上下でスキル変更");
    spInfo.SetAlignment(TextAlign::Center, TextAlign::Top);
    spInfo.Apply("x:640, y:688");

    @spName = TextSprite(skin.GetFont("Normal64"), "");
    spName.SetAlignment(TextAlign::Center, TextAlign::Top);
    spName.Apply("x:320, y:600");

    @spImage = Sprite();
    spImage.Apply("x:320, y:256");

    @spIcon = Sprite();
    spIcon.Apply("x:960, y:256, origX:48, origY:48, scaleX: 1.25, scaleY:1.25");

    @spSkill = TextSprite(skin.GetFont("Normal64"), "");
    spSkill.SetAlignment(TextAlign::Center, TextAlign::Top);
    spSkill.Apply("x:960, y:360, scaleX:0.75, scaleY:0.75");

    @spDescription = TextSprite(skin.GetFont("Normal64"), "");
    spDescription.SetAlignment(TextAlign::Center, TextAlign::Top);
    spDescription.SetRich(true);
    spDescription.Apply("x:960, y:440, scaleX:0.75, scaleY:0.75");

    container.AddChild(spBack);
    container.AddChild(spTitle);
    container.AddChild(spInfo);

    container.AddChild(spName);
    container.AddChild(spImage);

    container.AddChild(spIcon);
    container.AddChild(spSkill);
    container.AddChild(spDescription);

    AddSprite(container);
    container.Apply("x:-1280");

    UpdateInfo();
    UpdateSkill();
  }

  void Run() {
    RunCoroutine(Coroutine(KeyInput), "Select:KeyInput");
    while(true) YieldTime(1);
  }

  void UpdateInfo() {
    @ch = cm.GetCharacter(0);
    if(ch is null) return;
    @cim = cm.CreateCharacterImages(0);
    if(cim is null) return;

    spName.SetText(ch.Name);
    cim.ApplyFullImage(spImage);
  }

  void UpdateSkill() {
    @sk = sm.GetSkill(0);
    if(sk is null) return;

    spSkill.SetText(sk.Name);
    spDescription.SetText(sk.GetDetail(0).Description);
    spIcon.SetImage(Image(sk.IconPath));
  }

  void Draw() {

  }

  void KeyInput() {
    while(true) {
      if (IsKeyTriggered(Key::INPUT_TAB)) {
        if (isEnabled) {
          Fire("Select:Enable");
          container.AddMove("x:{end:-1280, time:0.5, func:out_sine}");
        } else {
          Fire("Select:Disable");
          container.AddMove("x:{end:0, time:0.5, func:out_sine}");
        }
        isEnabled = !isEnabled;
      }
      if (isEnabled) {
        if (IsKeyTriggered(Key::INPUT_LEFT)) {
          cm.Previous();
          FadeChar();
        }
        if (IsKeyTriggered(Key::INPUT_RIGHT)) {
          cm.Next();
          FadeChar();
        }
        if (IsKeyTriggered(Key::INPUT_UP)) {
          sm.Previous();
          FadeSkill();
        }
        if (IsKeyTriggered(Key::INPUT_DOWN)) {
          sm.Next();
          FadeSkill();
        }
      }
      YieldFrame(1);
    }
  }

  void FadeChar() {
    spName.AddMove("alpha:{begin:1, end:0, time:0.2}");
    spImage.AddMove("alpha:{begin:1, end:0, time:0.2}");
    YieldTime(0.2);
    UpdateInfo();
    spName.AddMove("alpha:{begin:0, end:1, time:0.2}");
    spImage.AddMove("alpha:{begin:0, end:1, time:0.2}");
  }

  void FadeSkill() {
    spIcon.AddMove("alpha:{begin:1, end:0, time:0.2}");
    spSkill.AddMove("alpha:{begin:1, end:0, time:0.2}");
    spDescription.AddMove("alpha:{begin:1, end:0, time:0.2}");
    YieldTime(0.2);
    UpdateSkill();
    spIcon.AddMove("alpha:{begin:0, end:1, time:0.2}");
    spSkill.AddMove("alpha:{begin:0, end:1, time:0.2}");
    spDescription.AddMove("alpha:{begin:0, end:1, time:0.2}");
  }

  void OnEvent(const string &in event) {
    if (event == "Select:End") Disappear();
  }
}
