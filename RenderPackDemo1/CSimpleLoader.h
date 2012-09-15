#ifndef _CSIMPLE_LOADER_H
#define _CSIMPLE_LOADER_H
#pragma once

#include <Windows.h>
#include <vector>

#include "IMeshLoader.h"

///////////////////////////////////////////////////////////////////

namespace RenderPack
{

#pragma warning(disable: 4251)

	//! Class for loading geometry data from an OBJ file
	class EXPORT CSimpleLoader
		:public IMeshLoader, public CRefCtrBase
	{
	public:
		CSimpleLoader();

		virtual ~CSimpleLoader();

		virtual bool LoadTriMesh(const wchar_t* FileName, CTriMesh** ppMesh, class CCachedFileResourceBase* pResource = NULL);
	};

	typedef CReference<CSimpleLoader> CSimpleLoaderRef;

#pragma warning(default: 4251)

}

#endif // _CSIMPLE_LOADER_H

///////////////////////////////////////////////////////////////////
