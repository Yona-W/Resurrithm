#include "SceneDebug.h"

void SceneDebug::Tick(const double delta)
{
	calc += delta;
	call++;
	if (calc >= 1.00) {
		fps = call / calc;
		calc = call = 0;
	}
}

void SceneDebug::Draw()
{
	clsDx();
	printfDx(reinterpret_cast<const char*>(L"%2.1f fps\n"), fps);
}

bool SceneDebug::IsDead()
{
	return false;
}
