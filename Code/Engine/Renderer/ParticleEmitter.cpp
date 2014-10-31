#include <Engine/StdAfx.h>
#include <Engine/Renderer/ParticleEmitter.h>
#include <Engine/Renderer/ParticleManager.h>
#include <Engine/ICamera.h>
#include <Engine/IMeshObject.h>
#include <Engine/RenderObjects/ParticleRenderObject.h>
#include <CommonLib/Math/BVaabb.h>

namespace fastbird
{

IParticleEmitter* IParticleEmitter::CreateParticleEmitter()
{
	return FB_NEW(ParticleEmitter);
}

static const float INFINITE_LIFE_TIME = -1.0f;

//-----------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter()
	: mLifeTime(2), mCurLifeTime(0)
	, mEmitterID(0), mInActiveList(false)
	, mClonedTemplates(0)
	, mMaxSize(0.0f)
	, mStop(false)
	, mStopImmediate(false)
	, mEmitterDirection(0, 1, 0)
	, mAdam(0)
	, mManualEmitter(false)
	, mEmitterColor(1, 1, 1)
	, mLength(0.0f)
{
	mObjFlag |= OF_IGNORE_ME;
}

ParticleEmitter::~ParticleEmitter()
{
	FB_FOREACH(it, mParticles)
	{
		FB_SAFE_DEL(it->second);
	}
}

//-----------------------------------------------------------------------------
bool ParticleEmitter::Load(const char* filepath)
{
	assert(!mClonedTemplates); // do not load in cloned template
	tinyxml2::XMLDocument doc;
	int err = doc.LoadFile(filepath);
	if (err)
	{
		Log(FB_DEFAULT_DEBUG_ARG, "Error while parsing particle!");
		if (doc.ErrorID() == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
		{
			Log("particle %s is not found!", filepath);
		}
		const char* errMsg = doc.GetErrorStr1();
		if (errMsg)
			Log("\t%s", errMsg);
		errMsg = doc.GetErrorStr2();
		if (errMsg)
			Log("\t%s", errMsg);
		return false;
	}

	tinyxml2::XMLElement* pPE = doc.FirstChildElement("ParticleEmitter");
	if (!pPE)
	{
		Error("Invaild particle file. %s", filepath);
		return false;
	}

	const char* sz = pPE->Attribute("emitterLifeTime");
	if (sz)
		mLifeTime = StringConverter::parseReal(sz);
	sz = pPE->Attribute("emitterID");
	if (sz)
		mEmitterID = StringConverter::parseUnsignedInt(sz);
	else
	{
		assert(0);
		Log("Emitter id omitted! (%s)", filepath);
	}

	sz = pPE->Attribute("manualControl");
	if (sz)
		mManualEmitter = StringConverter::parseBool(sz);

	mPTemplates.clear();
	tinyxml2::XMLElement* pPT = pPE->FirstChildElement("ParticleTemplate");
	while (pPT)
	{
		mPTemplates.push_back(ParticleTemplate());
		ParticleTemplate& pt = mPTemplates.back();

		sz = pPT->Attribute("texture");
		if (sz)
			pt.mTexturePath = sz;

		sz = pPT->Attribute("geometry");
		if (sz)
		{
			pt.mGeometryPath = sz;
			assert(pt.mTexturePath.empty());
			if (!pt.mGeometryPath.empty())
			{
				pt.mMeshObject = gFBEnv->pEngine->GetMeshObject(pt.mGeometryPath.c_str());
			}
		}

		sz = pPT->Attribute("startAfter");
		if (sz)
			pt.mStartAfter = StringConverter::parseReal(sz);

		sz = pPT->Attribute("emitPerSec");
		if (sz)
			pt.mEmitPerSec = StringConverter::parseReal(sz);

		sz = pPT->Attribute("numInitialParticle");
		if (sz)
			pt.mInitialParticles = StringConverter::parseUnsignedInt(sz);

		sz = pPT->Attribute("maxParticle");
		if (sz)
			pt.mMaxParticle = StringConverter::parseUnsignedInt(sz);

		sz = pPT->Attribute("deleteWhenFull");
		if (sz)
			pt.mDeleteWhenFull = StringConverter::parseBool(sz);

		sz = pPT->Attribute("cross");
		if (sz)
			pt.mCross = StringConverter::parseBool(sz);

		sz = pPT->Attribute("preMultiAlpha");
		if (sz)
			pt.mPreMultiAlpha = StringConverter::parseBool(sz);
		
		sz = pPT->Attribute("blendMode");
		if (sz)
			pt.mBlendMode = ParticleBlendMode::ConvertToEnum(sz);

		float glow = 0.f;
		sz = pPT->Attribute("glow");
		if (sz)
			glow = StringConverter::parseReal(sz);

		sz = pPT->Attribute("posOffset");
		if (sz)
			pt.mPosOffset = StringConverter::parseVec3(sz);

		sz = pPT->Attribute("lifeTimeMinMax");
		if (sz)
			pt.mLifeMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("align");
		if (sz)
			pt.mAlign = ParticleAlign::ConverToEnum(sz);

		sz = pPT->Attribute("stretchMax");
		if (sz)
			pt.mStretchMax = StringConverter::parseReal(sz);

		sz = pPT->Attribute("DefaultDirection");
		if (sz)
			pt.mDefaultDirection = StringConverter::parseVec3(sz);

		sz = pPT->Attribute("emitTo");
		if (sz)
			pt.mEmitTo = ParticleEmitTo::ConverToEnum(sz);

		sz = pPT->Attribute("range");
		if (sz)
			pt.mRangeType = ParticleRangeType::ConverToEnum(sz);

		sz = pPT->Attribute("rangeRadius");
		if (sz)
			pt.mRangeRadius = StringConverter::parseReal(sz);

		sz = pPT->Attribute("posInterpolation");
		if (sz)
			pt.mPosInterpolation = StringConverter::parseBool(sz);

		sz = pPT->Attribute("sizeMinMax");
		if (sz)
			pt.mSizeMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("sizeRatioMinMax");
		if (sz)
			pt.mSizeRatioMinMax = StringConverter::parseVec2(sz);

		mMaxSize = std::max(mMaxSize, 
			std::max(
			pt.mSizeMinMax.x * std::max(pt.mSizeRatioMinMax.x, pt.mSizeRatioMinMax.y),
				pt.mSizeMinMax.y));

		sz = pPT->Attribute("pivot");
		if (sz)
			pt.mPivot = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("scaleVelMinMax");
		if (sz)
			pt.mScaleVelMinMax = StringConverter::parseVec2(sz);
		sz = pPT->Attribute("scaleVelRatio");
		if (sz)
			pt.mScaleVelRatio = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("scaleAccel");
		if (sz)
			pt.mScaleAccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("scaleAccelUntil");
		if (sz)
			pt.mScaleAccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("scaleDeaccel");
		if (sz)
			pt.mScaleDeaccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("scaleDeaccelAfter");
		if (sz)
			pt.mScaleDeaccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("velocityMinMax");
		if (sz)
			pt.mVelocityMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("velocityDirectionMin");
		if (sz)
			pt.mVelocityDirMin = StringConverter::parseVec3(sz);
		sz = pPT->Attribute("velocityDirectionMax");
		if (sz)
			pt.mVelocityDirMax = StringConverter::parseVec3(sz);

		sz = pPT->Attribute("accel");
		if (sz)
			pt.mAccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("accelUntil");
		if (sz)
			pt.mAccel.y = StringConverter::parseReal(sz) * 0.01f;
		sz = pPT->Attribute("deaccel");
		if (sz)
			pt.mDeaccel.x = StringConverter::parseReal(sz);
		sz = pPT->Attribute("deaccelAfter");
		if (sz)
			pt.mDeaccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("rotMinMax");
		if (sz)
			pt.mRotMinMax = Radian(StringConverter::parseVec2(sz));
		sz = pPT->Attribute("rotSpeedMin");
		if (sz)
			pt.mRotSpeedMinMax.x = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotSpeedMax");
		if (sz)
			pt.mRotSpeedMinMax.y = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotAccel");
		if (sz)
			pt.mRotAccel.x = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotAccelUntil");
		if (sz)
			pt.mRotAccel.y = StringConverter::parseReal(sz) * 0.01f;
		sz = pPT->Attribute("rotDeaccel");
		if (sz)
			pt.mRotDeaccel.x = Radian(StringConverter::parseReal(sz));
		sz = pPT->Attribute("rotDeaccelAfter");
		if (sz)
			pt.mRotDeaccel.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("fadeInUntil");
		if (sz)
			pt.mFadeInOut.x = StringConverter::parseReal(sz) * 0.01f;
		sz = pPT->Attribute("fadeOutAfter");
		if (sz)
			pt.mFadeInOut.y = StringConverter::parseReal(sz) * 0.01f;

		sz = pPT->Attribute("Intensity");
		if (sz)
			pt.mIntensityMinMax = StringConverter::parseVec2(sz);

		sz = pPT->Attribute("color");
		if (sz)
			pt.mColor = StringConverter::parseColor(sz);
		pt.mColorEnd = pt.mColor;

		sz = pPT->Attribute("colorEnd");
		if (sz)
			pt.mColorEnd = StringConverter::parseColor(sz);

		sz = pPT->Attribute("uvAnimColRow");
		if (sz)
			pt.mUVAnimColRow = StringConverter::parseVec2I(sz);

		sz = pPT->Attribute("uvAnimFramesPerSec");
		if (sz)
		{
			pt.mUVAnimFramesPerSec = StringConverter::parseReal(sz);
			if (pt.mUVAnimFramesPerSec != 0.f)
			{
				pt.mUV_INV_FPS = 1.0f / pt.mUVAnimFramesPerSec;
			}
			else
			{
				if ((pt.mUVAnimColRow.x != 1 || pt.mUVAnimColRow.y != 1) && (pt.mLifeMinMax.x == -1 || pt.mLifeMinMax.y == -1))
				{
					pt.mUV_INV_FPS = 1.0f / 4.0f;
				}
			}
		}

		sz = pPT->Attribute("uvFlow");
		if (sz)
		{
			pt.mUVFlow = StringConverter::parseVec2(sz);
		}

		if (!pt.mTexturePath.empty())
		{
			ParticleRenderObject* pro = ParticleRenderObject::GetRenderObject(pt.mTexturePath.c_str());
			assert(pro);
			pro->GetMaterial()->SetMaterialParameters(0, Vec4(glow, 0, 0, 0));
			pro->GetMaterial()->RemoveShaderDefine("_INV_COLOR_BLEND");
			pro->GetMaterial()->RemoveShaderDefine("_PRE_MULTIPLIED_ALPHA");
			BLEND_DESC desc;
			switch (pt.mBlendMode)
			{
			case ParticleBlendMode::Additive:
			{
												desc.RenderTarget[0].BlendEnable = true;
												desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
												desc.RenderTarget[0].SrcBlend = BLEND_ONE;
												desc.RenderTarget[0].DestBlend = BLEND_ONE;
			}
				break;
			case ParticleBlendMode::AlphaBlend:
			{
												  // desc.AlphaToCoverageEnable = true;
												  desc.RenderTarget[0].BlendEnable = true;
												  desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
												  desc.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
												  desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;

			}
				break;
			case ParticleBlendMode::InvColorBlend:
			{
													 BLEND_DESC desc;
													 desc.RenderTarget[0].BlendEnable = true;
													 desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
													 desc.RenderTarget[0].SrcBlend = BLEND_INV_DEST_COLOR;
													 desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
													 pro->GetMaterial()->AddShaderDefine("_INV_COLOR_BLEND", "1");
			}
				break;
			case ParticleBlendMode::Replace:
			{
											   // desc.AlphaToCoverageEnable = true;
											   desc.RenderTarget[0].BlendEnable = true;
											   desc.RenderTarget[0].BlendOp = BLEND_OP_ADD;
											   desc.RenderTarget[0].SrcBlend = BLEND_ONE;
											   desc.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
											   pro->GetMaterial()->AddShaderDefine("_PRE_MULTIPLIED_ALPHA", "1");
			}
				break;
			default:
				assert(0);
			}
			if (pt.mPreMultiAlpha)
			{
				pro->GetMaterial()->AddShaderDefine("_PRE_MULTIPLIED_ALPHA", "1");
			}
			pro->SetBlendState(desc);
			if (glow == 0.f)
			{
				pro->GetMaterial()->AddShaderDefine("_NO_GLOW", "1");
				pro->SetGlow(false);
			}
			else
			{
				pro->GetMaterial()->RemoveShaderDefine("_NO_GLOW");
				pro->SetGlow(true);
			}

			pro->GetMaterial()->ApplyShaderDefines();
		}
		
		pPT = pPT->NextSiblingElement();
	}

	/*
	Prototype doens't need to have this.
	FB_FOREACH(it, mPTemplates)
	{
	mParticles.Insert(PARTICLESS::value_type(&(*it), FB_NEW(PARTICLES)));
	mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 0.f));
	}
	*/

	return true;
}

IObject* ParticleEmitter::Clone() const
{
	ParticleEmitter* cloned = FB_NEW(ParticleEmitter);
	cloned->mAdam = (ParticleEmitter*)this;
	cloned->mClonedTemplates = &mPTemplates;
	cloned->mEmitterID = mEmitterID;
	cloned->mLifeTime = mLifeTime;
	cloned->mMaxSize = mMaxSize;
	cloned->mManualEmitter = mManualEmitter;
	// EmitterDir is not using currently.
	cloned->mEmitterDirection = mEmitterDirection;

	FB_FOREACH(it, mPTemplates)
	{
		PARTICLES* particles = FB_NEW(PARTICLES);
		particles->Init(it->mMaxParticle*2);
		cloned->mParticles.Insert(PARTICLESS::value_type(&(*it), particles));
		cloned->mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 0.f));
		cloned->mNextEmits[&(*it)] = (float)it->mInitialParticles;
		cloned->mAliveParticles.Insert(std::make_pair(&(*it), 0));
	}
	cloned->mBoundingVolumeWorld = BoundingVolume::Create(BoundingVolume::BV_AABB);
	cloned->mBoundingVolume = 0;

	return cloned;
}

void ParticleEmitter::Active(bool a)
{
	if (a && !mInActiveList)
	{
		ParticleManager::GetParticleManager().AddActiveParticle(this);
		mInActiveList = true;
		mStop = false;
		mStopImmediate = false;
		mCurLifeTime = 0.f;
		if (mClonedTemplates)
		{
			FB_FOREACH(it, (*mClonedTemplates))
			{
				mNextEmits.Insert(NEXT_EMITS::value_type(&(*it), 1.f));
				mNextEmits[&(*it)] = (float)it->mInitialParticles;
			}
		}
	}
	else if (!a && mInActiveList)
	{
		ParticleManager::GetParticleManager().RemoveDeactiveParticle(this);
		mInActiveList = false;
	}
}

void ParticleEmitter::Stop()
{
	mStop = true;
}

void ParticleEmitter::StopImmediate()
{
	mStop = true;
	mStopImmediate = true;
}

bool ParticleEmitter::IsAlive()
{
	return mInActiveList;
}

bool ParticleEmitter::Update(float dt)
{
	mCurLifeTime+=dt;
	if ((!IsInfinite() && mCurLifeTime > mLifeTime ) || mStop)
	{
		mStop = true;
		int numAlive = 0;
		if (!mStopImmediate)
		{
			FB_FOREACH(it, mParticles)
			{
				const ParticleTemplate* pt = it->first;
				PARTICLES* particles = it->second;
				PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
				for (; itParticle != itEndParticle; ++itParticle)
				{
					if (itParticle->IsInfinite())
					{
						itParticle->mLifeTime = 0.0f;
					}
					if (itParticle->IsAlive())
						++numAlive;
				}
			}
		}
		if (numAlive == 0)
		{
			mInActiveList = false;
			return false;
		}
	}

	mBoundingVolumeWorld->Invalidate();
	// update existing partices
	FB_FOREACH(it, mParticles)
	{
		const ParticleTemplate* pt = it->first;
		PARTICLES* particles = it->second;
		PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
		for (; itParticle!=itEndParticle; ++itParticle)
		{
			Particle& p = *itParticle;
			if (p.IsAlive() && p.mLifeTime!=-2.f)
			{
				if (!p.IsInfinite())
				{
					p.mCurLifeTime += dt;
					if (p.mCurLifeTime>=p.mLifeTime)
					{
						p.mLifeTime = 0.0f; // mark dead.
						p.mCurLifeTime = 0.0f;
						if (p.mMeshObject)
							p.mMeshObject->DetachFromScene();
						continue;
					}
				}
				// change velocity
				float normTime = 0;
				if (!p.IsInfinite())
				{
					normTime = p.mCurLifeTime / p.mLifeTime;
					if (normTime < pt->mAccel.y)
					{
						p.mVelocity += pt->mAccel.x * dt;
					}
					if (normTime > pt->mDeaccel.y)
					{
						p.mVelocity -= pt->mDeaccel.x * dt;
					}
				}
				
				// change pos
				p.mPos += p.mVelDir * (p.mVelocity * dt);
				if (pt->IsLocalSpace())
				{
					p.mPosWorld = mTransformation.ApplyForward(p.mPos);
				}
				else
				{
					p.mPosWorld = p.mPos;
				}

				//change rot speed
				if (!p.IsInfinite())
				{
					float sign = Sign(p.mRotSpeed);
					if (normTime < pt->mRotAccel.y)
					{
						p.mRotSpeed += sign * pt->mRotAccel.x * dt;
					}
					if (normTime > pt->mRotDeaccel.y)
					{
						p.mRotSpeed -= sign * pt->mRotDeaccel.x * dt;
					}
				}
				// change rot
				p.mRot += p.mRotSpeed * dt;
				

				// change scale speed
				if (!p.IsInfinite())
				{
					if (normTime < pt->mScaleAccel.y)
					{
						p.mScaleSpeed += pt->mScaleAccel.x * dt;
					}
					if (normTime > pt->mScaleDeaccel.y)
					{
						p.mScaleSpeed -= pt->mScaleDeaccel.x * dt;
					}
				}
				// change scale
				p.mSize += p.mScaleSpeed * dt;
				if (p.mSize.x < 0.0f)
					p.mSize.x = 0.0f;
				if (p.mSize.y < 0.0f)
					p.mSize.y = 0.0f;

				Vec3 toSidePos = GetForward() * p.mSize.x;
				mBoundingVolumeWorld->Merge(p.mPosWorld - pt->mPivot.x * toSidePos);
				mBoundingVolumeWorld->Merge(p.mPosWorld + (1.0f - pt->mPivot.x) * toSidePos);

				// update alpha
				if (!p.IsInfinite())
				{
					if (normTime < pt->mFadeInOut.x)
						p.mAlpha = normTime / pt->mFadeInOut.x;
					else if (normTime > pt->mFadeInOut.y)
						p.mAlpha = (1.0f - normTime) / (1.0f - pt->mFadeInOut.y);
					else
						p.mAlpha = 1.0f;

					// update color
					if (pt->mColor != pt->mColorEnd)
					{
						p.mColor = Lerp(pt->mColor, pt->mColorEnd, normTime) * mEmitterColor;
					}					
				}
				else
				{
					p.mAlpha = 1.0f;
				}


				// uv frame
				if ((pt->mUVAnimColRow.x > 1 || pt->mUVAnimColRow.y > 1))
				{
					p.mUVFrame += dt;
					while (p.mUVFrame > p.mUV_SPF)
					{
						p.mUVFrame -= p.mUV_SPF;
						p.mUVIndex.x += 1;
						if (p.mUVIndex.x >= pt->mUVAnimColRow.x)
						{
							p.mUVIndex.x = 0;
							p.mUVIndex.y += 1;
							if (p.mUVIndex.y >= pt->mUVAnimColRow.y)
							{
								p.mUVIndex.y = 0;
							}
						}
					}					
				}
			}
		}
	}
	assert(mBoundingVolumeWorld->GetBVType()==BoundingVolume::BV_AABB);
	BVaabb* pAABB = (BVaabb*)mBoundingVolumeWorld.get();
	pAABB->Expand(mMaxSize);

	if (!mStop)
		UpdateEmit(dt);

	CopyDataToRenderer(dt);

	return true;
}

void ParticleEmitter::UpdateEmit(float dt)
{
	// emit
	int i=0;
	FB_FOREACH(itPT, (*mClonedTemplates) )
	{
		const ParticleTemplate& pt = *itPT;
		if (pt.mStartAfter > mCurLifeTime)
			continue;

		unsigned alives = mAliveParticles[&pt];
		if (alives > pt.mMaxParticle && !pt.mDeleteWhenFull)
		{
			mNextEmits[&pt] = 0;
			continue;
		}

		float& nextEmit = mNextEmits[&pt];
		nextEmit += dt * pt.mEmitPerSec;
		float integral;
		nextEmit = modf(nextEmit, &integral);
		int num = (int)integral;
		auto itFind = mLastEmitPos.Find(&pt);
		Particle* p = 0;
		for (int i=0; i<num; i++)
		{
			p = Emit(pt);
			if (itFind != mLastEmitPos.end())
			{
				Vec3 toNew = p->mPos - itFind->second;
				float length = toNew.Normalize();
				p->mPos = itFind->second + toNew * length*((i + 1) / (float)num);
			}
		}

		if (pt.mPosInterpolation && p)
		{
			mLastEmitPos[&pt] = p->mPos;
		}
	}
}

void ParticleEmitter::CopyDataToRenderer(float dt)
{
	ICamera* pCamera = gFBEnv->pRenderer->GetCamera();
	assert(pCamera);
	if (pCamera->IsCulled(mBoundingVolumeWorld))
		return;

	FB_FOREACH(it, mParticles)
	{
		PARTICLES* particles = (it->second);
		const ParticleTemplate* pt = it->first;
		
		ParticleRenderObject* pro = 0;
		if (!pt->mTexturePath.empty())
			pro = ParticleRenderObject::GetRenderObject(pt->mTexturePath.c_str());

		if (pro && pt->mAlign)
		{
			pro->SetDoubleSided(true);
		}
		unsigned& aliveParticle = mAliveParticles[pt];
		aliveParticle = 0;
		PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
		for (; itParticle != itEndParticle; ++itParticle)
		{
			if (itParticle->IsAlive())
				aliveParticle++;
		}

		if (aliveParticle > 0)
		{
			ParticleRenderObject::Vertex* dest = 0;
			if (pro)
				dest = pro->Map(pt->mCross ? aliveParticle * 2 : aliveParticle);
			PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
			for (; itParticle != itEndParticle; ++itParticle)
			{
				Particle& p = *itParticle;
				if (p.IsAlive())
				{
					int iteration = pt->mCross ? 2 : 1;
					Vec3 vdirBackup;
					Vec3 udirBackup;
					for (int i = 0; i < iteration; i++)
					{
						Vec3 udir = p.mUDirection;
						Vec3 vdir = p.mVDirection;
						if (pt->IsLocalSpace())
						{
							if (pt->IsAlignDirection())
							{
								Mat33 toViewRot = pCamera->GetViewMat().To33();
								if (i == 0)
								{
									Vec3 worldForward = (mTransformation.GetRotation() * udir);
									udir = toViewRot * worldForward;
									udirBackup = udir;
									vdir = toViewRot  * pCamera->GetForward().Cross(worldForward).NormalizeCopy();
									vdirBackup = vdir;
								}
								else
								{
									// crossed additional plane
									udir = udirBackup;
									vdir = vdirBackup.Cross(udirBackup).NormalizeCopy();
								}
							}
						}
						Vec2 size = p.mSize;
						if (pt->mStretchMax > 0.f)
						{
							size.x += std::min(size.x*pt->mStretchMax, std::max(0.f, (GetPos() - GetPrevPos()).Length() / dt*0.1f - mDistToCam*.1f));
						}
						if (dest)
						{
							dest->mPos = p.mPosWorld;
							dest->mUDirection_Intensity = Vec4(udir.x, udir.y, udir.z, p.mIntensity);
							dest->mVDirection = vdir;
							dest->mPivot_Size = Vec4(pt->mPivot.x, pt->mPivot.y, size.x, size.y);
							dest->mRot_Alpha_uv = Vec4(p.mRot, p.mAlpha,
								p.mUVIndex.x - pt->mUVFlow.x * p.mCurLifeTime, p.mUVIndex.y - pt->mUVFlow.y * p.mCurLifeTime);
							dest->mUVStep = p.mUVStep;
							dest->mColor = p.mColor;
							dest++;
						}
						else // geometry
						{
							p.mMeshObject->SetPos(p.mPosWorld);
							p.mMeshObject->SetDir(GetForward());
						}
							
					}
				}
			}
			if (pro)
				pro->Unmap();
		}
	}
}

//-----------------------------------------------------------------------------
IParticleEmitter::Particle* ParticleEmitter::Emit(const ParticleTemplate& pt)
{
	PARTICLES& particles = *(mParticles[&pt]);
	size_t addedPos = particles.push_back(Particle());
	Particle& p = particles.GetAt(addedPos);
	switch (pt.mRangeType)
	{
	case ParticleRangeType::Point:
	{
									 if (pt.IsLocalSpace())
									 {
										 p.mPos = 0.0f;
									 }
									 else
									 {
										 p.mPos = mTransformation.GetTranslation();
									 }
	}
		break;
	case ParticleRangeType::Box:
	{
								   p.mPos = Random(Vec3(-pt.mRangeRadius), Vec3(pt.mRangeRadius));
								   if (!pt.IsLocalSpace())
								   {
									   p.mPos += mTransformation.GetTranslation();
								   }
	}
		break;
	case ParticleRangeType::Sphere:
	{
									  float r = Random(0.0f, pt.mRangeRadius);
									  float theta = Random(0.0f, PI);
									  float phi = Random(0.0f, TWO_PI);
									  float st = sin(theta);
									  float ct = cos(theta);
									  float sp = sin(phi);
									  float cp = cos(phi);
									  p.mPos = Vec3(r * st * cp, r*st*sp, r*ct);
									  if (!pt.IsLocalSpace())
									  {
										  p.mPos += mTransformation.GetTranslation();
									  }
	}
		break;
	case ParticleRangeType::Hemisphere:
	{
										  float r = Random(0.0f, pt.mRangeRadius);
										  float theta = Random(0.0f, HALF_PI);
										  float phi = Random(0.0f, TWO_PI);
										  p.mPos = SphericalToCartesian(theta, phi) * r;
										  if (!pt.IsLocalSpace())
										  {
											  p.mPos += mTransformation.GetTranslation();
										  }
	}
		break;
	case ParticleRangeType::Cone:
	{
									float tanS = Random(0.0f, PI);
									float cosT = Random(0.0f, TWO_PI);
									float sinT = Random(0.0f, TWO_PI);
									float height = Random(0.0f, pt.mRangeRadius);
									p.mPos = Vec3(height*tanS*cosT, height*tanS*sinT, height);
									if (!pt.IsLocalSpace())
									{
										p.mPos += mTransformation.GetTranslation();
									}
	}
		break;
	}
	float angle = pt.mDefaultDirection.AngleBetween(GetForward());
	Vec3 posOffset = pt.mPosOffset;
	if (pt.mPosOffset != Vec3::ZERO && pt.mEmitTo == ParticleEmitTo::WorldSpace)
	{
		if (angle > 0.01f)
		{
			Vec3 axis = pt.mDefaultDirection.Cross(GetForward());
			axis.Normalize();
			Quat matchRot(angle, axis);
			posOffset = matchRot * posOffset;
		}
	}
	p.mPos += posOffset;

	Vec3 velDir = Random(pt.mVelocityDirMin, pt.mVelocityDirMax).NormalizeCopy();	
	if (angle > 0.01f)
	{
		Vec3 axis = pt.mDefaultDirection.Cross(GetForward());
		axis.Normalize();
		Quat matchRot(angle, axis);
		velDir = matchRot * velDir;
	}

	p.mVelDir = velDir;
	p.mVelocity = Random(pt.mVelocityMinMax.x, pt.mVelocityMinMax.y);
	if (pt.mAlign == ParticleAlign::Billboard)
	{
		p.mUDirection = Vec3::UNIT_X;
		p.mVDirection = -Vec3::UNIT_Z;
	}
	else
	{
		p.mUDirection = Vec3::UNIT_Y;
		p.mVDirection = -Vec3::UNIT_Z;
	}

	p.mUVIndex = Vec2(0, 0);
	p.mUVStep = Vec2(1.0f / pt.mUVAnimColRow.x, 1.0f / pt.mUVAnimColRow.y);
	p.mUVFrame = 0.f;
	p.mLifeTime = Random(pt.mLifeMinMax.x, pt.mLifeMinMax.y);
	p.mUV_SPF = pt.mUV_INV_FPS;
	if (pt.mUVAnimFramesPerSec == 0.f && (pt.mUVAnimColRow.x != 1 || pt.mUVAnimColRow.y != 1))
	{
		int numFrames = pt.mUVAnimColRow.x * pt.mUVAnimColRow.y;
		p.mUV_SPF = p.mLifeTime / numFrames;
		assert(p.mUV_SPF > 0);
		
	}
	p.mCurLifeTime = 0.0f;
	float size = Random(pt.mSizeMinMax.x, pt.mSizeMinMax.y);
	float ratio = Random(pt.mSizeRatioMinMax.x, pt.mSizeRatioMinMax.y);
	p.mSize = Vec2(size * ratio, size);
	if (mLength != 0 && pt.mAlign == ParticleAlign::Direction)
	{
		p.mSize.x = p.mSize.x * (mLength / size);
	}

	float scalevel = Random(pt.mScaleVelMinMax.x, pt.mScaleVelMinMax.y);
	float svratio = Random(pt.mScaleVelRatio.x, pt.mScaleVelRatio.y);
	p.mScaleSpeed = Vec2(scalevel * svratio, scalevel);
	p.mRot = Random(pt.mRotMinMax.x, pt.mRotMinMax.y);
	p.mRotSpeed = Random(pt.mRotSpeedMinMax.x, pt.mRotSpeedMinMax.y);
	p.mIntensity = Random(pt.mIntensityMinMax.x, pt.mIntensityMinMax.y);
	p.mColor = pt.mColor * mEmitterColor;
	if (!pt.mGeometryPath.empty())
	{
		if (!p.mMeshObject)
			p.mMeshObject = (IMeshObject*)pt.mMeshObject->Clone();
		p.mMeshObject->AttachToScene();
		p.mMeshObject->SetPos(p.mPosWorld);
		p.mMeshObject->SetDir(GetForward());
	}
	return &p;
}

//-----------------------------------------------------------------------------
IParticleEmitter::Particle* ParticleEmitter::Emit(unsigned templateIdx)
{
	assert(templateIdx < mClonedTemplates->size());
	const ParticleTemplate& pt = (*mClonedTemplates)[templateIdx];
	return Emit(pt);
}

//-----------------------------------------------------------------------------
IParticleEmitter::Particle& ParticleEmitter::GetParticle(unsigned templateIdx, unsigned index)
{
	if (!mManualEmitter)
	{
		assert(0);
	}
	const ParticleTemplate& pt = (*mClonedTemplates)[templateIdx];
	PARTICLES& particles = *(mParticles[&pt]);
	return particles.GetAt(index);
}

//-----------------------------------------------------------------------------
void ParticleEmitter::SetBufferSize(unsigned size)
{
	FB_FOREACH(it, (*mClonedTemplates))
	{
		mParticles[&(*it)]->Init(size);
	}
}

void ParticleEmitter::SetLength(float len)
{
	if (mLength == len)
		return;
	mLength = len;
	FB_FOREACH(it, mParticles)
	{
		PARTICLES* particles = (it->second);
		const ParticleTemplate* pt = it->first;
		if (pt->mAlign == ParticleAlign::Direction)
		{
			float size = Random(pt->mSizeMinMax.x, pt->mSizeMinMax.y);
			float ratio = Random(pt->mSizeRatioMinMax.x, pt->mSizeRatioMinMax.y);
			PARTICLES::IteratorWrapper itParticle = particles->begin(), itEndParticle = particles->end();
			for (; itParticle != itEndParticle; ++itParticle)
			{				
				itParticle->mSize = Vec2(size * ratio, size);
				if (mLength != 0)
				{
					itParticle->mSize.x = itParticle->mSize.x * (mLength / size);
				}
			}
		}
	}	
}

////-----------------------------------------------------------------------------
//void ParticleEmitter::Sort()
//{
//	FB_FOREACH(it, (*mClonedTemplates))
//	{
//		PARTICLES& ps = *mParticles[&(*it)];
//		PARTICLES::VECTOR& v = ps.GetVector();
//		PARTICLES::iterator& begin = ps.GetRawBeginIter();
//		PARTICLES::iterator& end = ps.GetRawEndIter();
//		Vec3 pos = gFBEnv->pRenderer->GetCamera()->GetPos();
//		std::sort(v.begin(), v.end()-1, [&pos](const PARTICLES::value_type& v1, const PARTICLES::value_type& v2)->bool {
//			float distance1 = (v1.mPosWorld - pos).LengthSQ();
//			float distance2 = (v2.mPosWorld - pos).LengthSQ();
//			return distance2 > distance1;
//		});
//
//		begin = v.begin();
//		end = v.end()-1;
//	}
//}
}