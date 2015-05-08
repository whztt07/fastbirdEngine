#include <UI/StdAfx.h>
#include <UI/KeyboardCursor.h>

namespace fastbird
{

//---------------------------------------------------------------------------------------
KeyboardCursor* KeyboardCursor::mInstance = 0;

void KeyboardCursor::InitializeKeyboardCursor()
{
	if (!mInstance)
		mInstance = FB_NEW(KeyboardCursor);
}

KeyboardCursor& KeyboardCursor::GetKeyboardCursor()
{
	assert(mInstance); 
	return *mInstance;
}
void KeyboardCursor::FinalizeKeyboardCursor()
{
	FB_SAFE_DEL(mInstance);
}


//---------------------------------------------------------------------------------------
KeyboardCursor::KeyboardCursor()
	: mVisible(false)
{
	mUIObject = gFBEnv->pEngine->CreateUIObject(false, Vec2I(gFBEnv->pRenderer->GetWidth(), gFBEnv->pRenderer->GetHeight()));
	mUIObject->SetMaterial("es/materials/KeyboardCursor.material");
	mUIObject->SetDebugString("KeyboardCursor");
}

KeyboardCursor::~KeyboardCursor()
{
	gFBEnv->pEngine->DeleteUIObject(mUIObject);
}

//---------------------------------------------------------------------------------------
void KeyboardCursor::SetNPos(const Vec2& pos)
{
	mUIObject->SetNPos(pos);
}

void KeyboardCursor::SetNSize(const Vec2& size)
{
	mUIObject->SetNSize(size);
}

IUIObject* KeyboardCursor::GetUIObject() const
{
	return mUIObject;
}

}