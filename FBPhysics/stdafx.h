/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "FBCommonHeaders/platform.h"
#if defined(_PLATFORM_WINDOWS_)
#define FB_DLL_PHYSICS __declspec(dllexport)
#define FB_DLL_FILESYSTEM __declspec(dllimport)
#else
#endif

#ifdef _DEBUG
#pragma comment(lib, "BulletCollision_vs2010_debug.lib")
#pragma comment(lib, "BulletDynamics_vs2010_debug.lib")
#pragma comment(lib, "LinearMath_vs2010_debug.lib")
#else
#pragma comment(lib, "BulletCollision_vs2010.lib")
#pragma comment(lib, "BulletDynamics_vs2010.lib")
#pragma comment(lib, "LinearMath_vs2010.lib")
#endif

#include <iostream>
#include <assert.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btTransform.h>

#include "mathConv.h"
#include "FBDebugLib/Logger.h"
#include "FBMemoryManagerLib/MemoryManager.h"
#include "FBMathLib/Math.h"
#include "FBStringLib/StringLib.h"
#include "FBStringLib/StringConverter.h"
#include "FBStringMathLib/StringMathConverter.h"
#include "FBCommonHeaders/VectorMap.h"
#include "TinyXmlLib/tinyxml2.h"