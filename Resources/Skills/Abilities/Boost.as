[EntryPoint]
class Boost : Ability {
  double ratio;
  
  void Initialize(dictionary@ params, dictionary@ ski) {
    params.get("bonus", ratio);
  }
  
  void OnStart(Result@ result) {
  }
  
  void OnFinish(Result@ result) {
  }
  
  void OnJudge(Result@ result, JudgeData data) {
    switch(data.Judge) {
    case JudgeType::JusticeCritical:
      result.BoostGaugeJusticeCritical(ratio);
      TriggerIndicator("Boost");
      break;
    case JudgeType::Justice:
      result.BoostGaugeJustice(ratio);
      TriggerIndicator("Boost");
      break;
    case JudgeType::Attack:
      result.BoostGaugeAttack(ratio);
      TriggerIndicator("Boost");
      break;
    case JudgeType::Miss:
      break;
    }
  }
}
