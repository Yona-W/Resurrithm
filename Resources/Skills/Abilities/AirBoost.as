[EntryPoint]
class AirBoost : Ability {
  int air, aa, other;
  
  void Initialize(dictionary@ params, dictionary@ ski) {
    params.get("air", air);
    params.get("airact", aa);
    params.get("other", other);
  }
  
  void OnStart(Result@ result) {
  }
  
  void OnFinish(Result@ result) {
  }
  
  void OnJudge(Result@ result, JudgeData data) {
    switch(data.Judge) {
    case JudgeType::JusticeCritical:
    case JudgeType::Justice:
    case JudgeType::Attack:
    case JudgeType::Miss:
      Execute(result, data.Note);
      break;
    }
  }
  
  void Execute(Result@ result, NoteType type) {
    switch (type) {
      case NoteType::Air:
        result.BoostGaugeByValue(air);
        TriggerIndicator("Boost");
        break;
      case NoteType::AirAction:
        result.BoostGaugeByValue(aa);
        TriggerIndicator("Boost");
        break;
      default:
        result.BoostGaugeByValue(other);
        TriggerIndicator("Damage");
        break;
    }
  }
}
