//////////////////////////////////////////////////////////////////////////
// CRenderPackDemo2.h
// *******************************************************
//
// Declaration of your main application class.
// RenderPack Wizard generated file.
//
//////////////////////////////////////////////////////////////////////////



#include "Util\CSimpleApp11.h"
#include "Render\D3D11\Shaders\CD3D11InputLayout.h"
#include "Render\D3D11\Util\CD3D11MeshFactory.h"
#include "Loaders\CObjLoader.h"
#include "SceneModel\CCamera.h"
#include "SceneModel\Animation\CFreeFlightController.h"
#include "CCameraController.h"
#include "SceneModel\Resource\CTriMeshResource.h"

#include "Render\D3D11\Memory\CD3D11DirectGeometry.h"
#include "Render\D3D11\Memory\CD3D11IndexedGeometry.h"
#include "Render\D3D11\Memory\CD3D11ConstantBuffer.h"
#include "Render\D3D11\Memory\CD3D11DepthTexture2D.h"
#include "Render\D3D11\Memory\CD3D11StructuredDataBuffer.h"

#include "Render\D3D11\Util\CD3D11RenderMesh.h"
#include "Render\D3D11\Util\CommonConstants.h"
#include "FTHelper.h"
#include "NuiAPI.h"
#include "FaceTrackLib.h"


using namespace RenderPack;

class CRenderPackDemo1
	:public CSimpleApp11
{
public:

	CRenderPackDemo1();

	virtual ~CRenderPackDemo1();

	int main() {}

	// CSimpleApp11

	virtual bool Init();

	virtual HRESULT OnCreateDevice(ID3D11Device* pDevice);
	virtual void OnDestroyDevice();
	void FacetrackingFrustum(float monitorWidth, float monitorHeight, float kinectPosition, FLOAT translationsKinectXYZ[3]);

	virtual HRESULT OnSwapChainResized(ID3D11Device* pDevice, IDXGISwapChain* pSwapChain,
		const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	virtual void OnSwapChainReleasing();


	virtual void OnFrameMove(double Time, float ElapsedTime);

	virtual void OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pImmediateContext);

	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam, bool *pNeedFurtherProcessing);

	virtual void OnKeyUp(WPARAM wParam, LPARAM lParam, bool *pNeedFurtherProcessing);

	virtual void MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing);

	static void FaceTracking(LPVOID lpParam);


	void FacetrackingTranslations(FLOAT translationXYZ[3]);

	void FacetrackingAnimating(FLOAT *pCoefficients, unsigned int AUCount, FLOAT scale, FLOAT rotationXYZ[3],FLOAT translationXYZ[3]);

protected:

	void CRenderPackDemo1::ParseDataInput();

	void CRenderPackDemo1::ParseObjInput();

	// render targets
	CD3D11RenderTexture2DRef m_rTexColor;
	CD3D11DepthTexture2DRef m_rTexDepth;
	CD3D11RenderTargetConfigRef m_rColorTarget;


	hlsl::float4 *pAnimationUnits;
	hlsl::int1 * pAU_nVerts;
	hlsl::float4 pAU_weights[6];
	int nAUs;

	float3 *m_smoother;
	int pointToSmooth;
	float smoothflag;

	float3 m_translateOffset2;
	float m_MonitorWidth;
	float m_MonitorHeight;
	float m_KinectPosition;
	//Controller
	enum STATUS_EYE {
		TURN_UP = 0,
		TURN_DOWN = 1,
		TURN_LEFT = 2,
		TURN_RIGHT = 3,
		TURN_CLOSE = 4,
		TURN_AWAY = 5
	};
	float x0;
	float x1;
	float y0;
	float y1;
	float z0;
	float z1;
	bool m_TurnStatus[6];
	bool m_reset;
	bool m_IsFaceTracked;
	float3 translateOffset;
	float3 m_CurrTranslateSpeed;
	hlsl::float4x4 m_ftfrustum;

	FTHelper m_FTHelper;

	float3 m_currFTTranslationsXYZ;

	float3 m_rFaceRotation;

	HWND m_hWnd;

	//scene Model
	CTriMeshResourceRef m_rMeshResource;
	CCameraRef m_rCam;
	CCameraRef m_rFaceCam;
	CFreeFlightControllerRef m_rFreeFlight;
	CCameraControllerRef m_rCamControl;

	CD3D11RenderMeshRef m_rRenderMesh;
	CD3D11InputLayoutRef m_rMeshLayout;
	ID3D11InputLayout *m_rMeshLayoutFace;

	//vertexBuffer for the FaceMesh
	hlsl::vector<float,4> * m_vertexData;

	ID3D11Buffer* m_vertexBuffer;

	int numOfFaceVerts;
	int numOfAllVertsAU;
	//index Buffer for the FaceMesh
	int numOfFaceFaces;
	ID3D11Buffer* m_indexBuffer;
	int *m_faceIndices;


	//constant buffers
	CD3D11ConstantBufferRef m_rCBObjectTransform;
	CD3D11ConstantBufferRef m_rCBObjectFaceTransform;
	CD3D11ConstantBufferRef m_rCBAUs;
	CD3D11StructuredDataBufferRef m_rSDBAnimationUnits;

	CD3D11StructuredDataBufferRef m_rAnimationData;

	//full screen squad:
	CD3D11DirectGeometryRef m_rRenderFullQuad;
	CD3D11InputLayoutRef m_rFullQuadLayout;

	//render passes
	CD3D11RenderConfigRef m_rFullScreenPass;
	CD3D11RenderConfigRef m_rTexturedPass;
	CD3D11RenderConfigRef m_rFacePass;
};