#include "Scene.h"
/*
Scene
シーンだよ

*/

Scene::Scene() : index(0), manager(nullptr)
{}

Scene::~Scene()
{}

void Scene::Initialize()
{}

void Scene::Tick(double delta)
{}

void Scene::OnEvent(const std::string& message)
{}

void Scene::Draw()
{}

bool Scene::IsDead()
{
	return false;
}

void Scene::Disappear()
{}

void Scene::Dispose()
{}
