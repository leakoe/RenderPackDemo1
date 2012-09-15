#define DLL_EXPORT
#include "DllHeader.h"

#define HLSL_NO_SWIZZLE_DEFINES

#include "CSimpleLoader.h"

	#pragma warning(disable: 4995)
	#pragma warning(disable: 4530)
	#include <fstream>
	#include <sstream>
	#pragma warning(default: 4995)
	#pragma warning(default: 4530)

#include "Core/CSystem.h"

#include "Util/CharUtil.h"

#include "Core/Exceptions/CException.h"
#include "Core/Exceptions/CFileNotFoundException.h"

using namespace RenderPack;
using namespace hlsl;

///////////////////////////////////////////////////////////////////
// CSimpleLoader

CSimpleLoader::CSimpleLoader()
{
}

CSimpleLoader::~CSimpleLoader()
{
}

// cache file resouce not used! No caching implemented
bool LoadTriMesh(const wchar_t* FileName, CTriMesh** ppMesh, class CCachedFileResourceBase* pResource)
{
	CTriMesh* pNewMesh = new CTriMesh();

	CTriMesh::CMeshSubsetRef rCurrSubset = new CTriMesh::CMeshSubset();
	pNewMesh->GetSubsetData().push_back(rCurrSubset);
	rCurrSubset->SetFormat(CTriMesh::PRIMITIVE_FORMAT_TRIANGLES);
	
	//temporary storage for the input data
	// This class only loads positions!
	std::vector<hlsl::float4> positions;

	char strCommand[MAX_PATH];

	std::string sPath = CCharUtil::FromWStringToString(FileName);

	std::fstream buffer(sPath.c_str());
	if(!buffer)
	{
		throw CFileNotFoundException(sPath.c_str());
		return false;
	}

	//parse the file
#pragma warning(disable: 4127)
	while(true)
#pragma warning(default: 4127)
	{
		if(buffer.eof())
			break;
		buffer>>strCommand;
		
		if(0 == strcmp( strCommand, "#" ))
		{
			//comment
		}
		else if(0 == strcmp( strCommand, "g" ))
		{
			SYSWARNING("CSimpleLoader::LoadTriMesh", 2, "unsupported token g (mesh subsets are not handled by SimpleLoader)");
		}
		else if(0 == strcmp( strCommand, "v" ))
		{
			//vertex position
			float x, y, z;
			buffer>>x>>y>>z;

			positions.push_back( hlsl::float4(x, y, z, 1.0f) );
		}
		else if(0 == strcmp( strCommand, "vt" ))
		{
			SYSWARNING("CSimpleLoader::LoadTriMesh", 2, "skipping vertex texcoord data (not supported by SimpleLoader");
		}
		else if(0 == strcmp( strCommand, "vn" ))
		{
			SYSWARNING("CSimpleLoader::LoadTriMesh", 2, "skipping vertex normal data (not supported by SimpleLoader");
		}
		else if(0 == strcmp( strCommand, "f" ))
		{
			//face
			//NOTE: the OBJ format uses 1-based arrays
			size_t iPos, iTex, iNorm;

			CTriMesh::FACE currFace;

			size_t iVertex = 0;
			bool isMore = true; // are there more vertices in the face
			while(isMore)
			{
				buffer>>iPos;

				// different rules for negative and positive indices
				if(iPos == 0)
				{
					throw CException("Invalid OBJ file: the position index cannot be 0.");
					return  false;
				}

				if( '/' == buffer.peek())
				{
					buffer.ignore();

					if('/' != buffer.peek())
					{
						//optional texture coordinate
						buffer>>iTex;
						// different rules for negative and positive indices
						if(iTex == 0)
						{
							throw CException("Invalid OBJ file: the texture index cannot be 0.");
							return  false;
						}
					}
					if('/' == buffer.peek())
					{
						buffer.ignore();

						//optional vertex normal
						buffer>>iNorm;
						// different rules for negative and positive indices
						if(iNorm == 0)
						{
							throw CException("Invalid OBJ file: the normal index cannot be 0.");
							return  false;
						}
					}
				}

				//add this vertex to the vertex buffer, but avoid duplicates
				DWORD index = (iPos > 0) ? (iPos - 1) : (positions.size() + iPos);
				currFace.m_Vertices[iVertex++] = index;

				char nextChar = buffer.peek();
				if(nextChar == ' ')
					buffer.get();

				isMore = false;
				
				//check if there is a number after the space
				nextChar = buffer.peek();
				if((nextChar >= '0' && nextChar <='9') || nextChar == '-')
					isMore = true;

				if(iVertex > 2)
				{
					pNewMesh->GetFaceData().push_back(currFace);

					rCurrSubset->Indices().push_back(currFace.m_Vertices[0]);
					rCurrSubset->Indices().push_back(currFace.m_Vertices[1]);
					rCurrSubset->Indices().push_back(currFace.m_Vertices[2]);

					currFace.m_Vertices[1] = currFace.m_Vertices[2];
					iVertex = 2;
				}
			}
		}
		if(0 == strcmp( strCommand, "usemtl" ))
		{
			SYSWARNING("CSimpleLoader::LoadTriMesh", 2, "unsupported token usemtl found, skipping");
		}
		else if(0 == strcmp(strCommand, "mtllib"))
		{
			SYSWARNING("CSimpleLoader::LoadTriMesh", 2, "unsupported token mtllib found, skipping");
		}

		buffer.ignore( 1000, '\n' );
	}

	buffer.close();

	CVertexCollection& meshVertices = pNewMesh->GetVertexData();

	//copy the position data (it always exists)
	CVertexChannelData& positionChannel = meshVertices.GetChannels()["POSITION"];
	positionChannel.GetData().reserve(positions.size());
	for(size_t iVertex = 0; iVertex < positions.size(); iVertex++)
	{
		positionChannel.GetData().push_back( positions[iVertex] );
	}
	
	*ppMesh = pNewMesh;

	return true;
}

///////////////////////////////////////////////////////////////////
