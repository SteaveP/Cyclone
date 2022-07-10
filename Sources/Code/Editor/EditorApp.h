#pragma once

#include "Engine/Framework/Impl/DefaultApplication.h"

namespace Cyclone
{

class CScene;
class CSceneViewport;

namespace Render { class CRenderSceneSubsystem; class CRenderSceneView; }

class CSceneSubsystem;

class CEditorApplication : public CDefaultApplication
{
public:
	~CEditorApplication();

	virtual void DeInit() override;

protected:
	C_STATUS OnInit() override;
	C_STATUS OnBeginFrame() override;
	C_STATUS OnUpdate() override;
	C_STATUS OnUpdateUI() override;

protected:
	void ShowMenu();
	void ShowViewports();
	void ShowWorldOutliner();
	void ShowProperties();
	void ShowContentBrowser();

private:
    void DeInitImpl();

	void OnAddRenderSceneView(Ptr<Render::CRenderSceneView> View);
	void OnRemoveRenderSceneView(Ptr<Render::CRenderSceneView> View);

protected:
    CSceneSubsystem* m_SceneSys = nullptr;
    Render::CRenderSceneSubsystem* m_RenderSceneSys = nullptr;

	// #todo_editor refactor
    Vector<RawPtr> m_ViewportRenderTargetsDescriptorSet;
	Vector<bool> m_ViewportOpened;
};

} // namespace Cyclone
