
#include "DllHeader.h"

#include "Core\HLSL.h"
#include "Core\HLSLEx.h"
#include "Core\CSystem.h"
#include "SceneModel\Animation\CFreeFlightController.h"

#include "Util\Logging\CWinConsoleListener.h"


#include "SceneModel\CCamera.h"
#include "CCameraController.h"
#include "stdafx.h"


//#include <math.h>


	
using namespace RenderPack;
using namespace hlsl;

#define TEST_LOG



CCameraController::CCameraController(CWinConsoleListener *pMyListener)
{

	for(int i = 0; i < 6; i++) {
		m_TurnStatus[i] = false;
	}
	flag = false;
	m_ConsoleListener = pMyListener;
	m_ConsoleListener->StartLog("CCameraController.CCameraController");
	m_CurrAngleSpeed = hlsl::vector<float,4>(0.78f,1.33f,0.02f,5.0f);
	 x0 = 1.33f*(-5.f);
	 x1 = 1.33f*5.f;
	 y0 = -5.f;
	 y1 = 5.f;
	 z0 = 0.02f;
	 z1 = 5.0f;
	 m_eye = vector<float,3>(0.f, 0.f, 0.f);

//	CFreeFlightController::SetTranslationSpeed(1.f);
	CFreeFlightController::CFreeFlightController();
}


CCameraController::~CCameraController()
{
	
}

void CCameraController::SetParams(float x_0, float x_1, float y_0, float y_1, float z_0, float z_1, hlsl::float3 eye) {
	x0 = x_0;
	x1 = x_1;
	y0 = y_0;
	y1 = y_1;
	z0 = z_0;
	z1 = z_1;
	m_eye = eye;
}


//IMessageHandler

void CCameraController::OnKeyDown(WPARAM wParam, LPARAM lParam, bool *pNeedFurtherProcessing) {
	if(!m_Enabled)
		return;

	char ch;
	BYTE keyStateArray[256];
	UINT scanCode = (UINT) lParam;
	WORD word;

	GetKeyboardState(keyStateArray);
	ToAscii((UINT) wParam, scanCode, keyStateArray, &word, 0);
	ch = (char) word;


	switch(ch)
	{
		case'k':
			m_TurnStatus[TURN_DOWN] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'i':
			m_TurnStatus[TURN_UP] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'j':
			m_TurnStatus[TURN_LEFT] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'l':
			m_TurnStatus[TURN_RIGHT] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'u':
			m_TurnStatus[TURN_AWAY] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'o':
			m_TurnStatus[TURN_CLOSE] = true;
			*pNeedFurtherProcessing = false;
			break;
	}

	if(*pNeedFurtherProcessing)
		CFreeFlightController::OnKeyDown(wParam, lParam, pNeedFurtherProcessing);
}  

void CCameraController::OnKeyUp(WPARAM wParam, LPARAM lParam, bool *pNeedFurtherProcessing) {
		if(!m_Enabled)
			return;

	char ch;
	BYTE keyStateArray[256];
	UINT scanCode = (UINT) lParam;
	WORD word;

	GetKeyboardState(keyStateArray);
	ToAscii((UINT) wParam, scanCode, keyStateArray, &word, 0);
	ch = (char) word;

	switch(ch)
	{
	case'k':

		m_TurnStatus[TURN_DOWN] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'i':

		m_TurnStatus[TURN_UP] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'j':

		m_TurnStatus[TURN_LEFT] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'l':

		m_TurnStatus[TURN_RIGHT] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'u':
		m_TurnStatus[TURN_AWAY] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'o':
		m_TurnStatus[TURN_CLOSE] = false;
		*pNeedFurtherProcessing = false;
		break;
			
	}

	if(*pNeedFurtherProcessing)
		CFreeFlightController::OnKeyUp(wParam, lParam, pNeedFurtherProcessing);
}

/*void CCameraController::OnMouseMove(float MouseX, float MouseY, BYTE MouseButtonState, bool *pNeedFurtherProcessing){int zahl = 0;}
void CCameraController::OnMouseDown(float MouseX, float MouseY, BYTE ActiveButton, BYTE MouseButtonState, bool *pNeedFurtherProcessing){int zahl = 0;}
void CCameraController::OnMouseUp(float MouseX, float MouseY, BYTE ActiveButton, BYTE MouseButtonState, bool *pNeedFurtherProcessing){int zahl = 0;}*/

//IAnimatable

void CCameraController::Animate(double Time, float ElapsedTime){
		
	bool needUpdate = false;
	
	//m_TranslateSpeed = CFreeFlightController::GetTranslationSpeed();

		

	if(!m_Enabled || !m_rControlTarget)
		return;

	for(int i = 0; i < 6; i++) {	
		if(m_TurnStatus[i] == true) {
			needUpdate = true;
		}
	}

	if(!flag) {
		m_ConsoleListener->Append("1 CurrAngle ");
		m_ConsoleListener->Append(m_CurrAngleSpeed);
		m_ConsoleListener->LineBreak();
		flag = true;
	}

	float currASpeed = length(m_CurrAngleSpeed);

	float currSpeed = length(m_CurrTranslateSpeed);
	float deltaSpeed = m_TranslateSpeed * ElapsedTime;
	if(deltaSpeed < currSpeed) {
		m_CurrTranslateSpeed -= deltaSpeed * normalize(m_CurrTranslateSpeed);
		/*m_ConsoleListener->Append("deltaSpeed < currSpeed -> CurrTransSpeed ");
		m_ConsoleListener->Append(m_CurrTranslateSpeed);
		m_ConsoleListener->LineBreak()*/;
	}
	else {
		m_CurrTranslateSpeed = hlsl::float3(0);
		/*m_ConsoleListener->Append("deltaSpeed >= currSpeed -> CurrTransSpeed ");
		m_ConsoleListener->Append(m_CurrTranslateSpeed);
		m_ConsoleListener->LineBreak();*/

	}

	if(deltaSpeed >= currASpeed)
		m_CurrAngleSpeed = hlsl::float4(0.78f, 1.33f, 0.02f, 5.0f);

	m_CurrRotateSpeed -= 0.5f * m_RotateSpeed * ElapsedTime;
	if(m_CurrRotateSpeed < 0)
		m_CurrRotateSpeed = 0;


		
	m_CurrRotateSpeed += sqrtf(m_DeltaMouseX * m_DeltaMouseX + m_DeltaMouseY * m_DeltaMouseY) * m_RotateSpeed;
	float rotationAmt = sqrtf(m_DeltaMouseX * m_DeltaMouseX + m_DeltaMouseY * m_DeltaMouseY) * m_RotateSpeed;
	float3 screenMouseVector(m_DeltaMouseX, -m_DeltaMouseY, 0);
	m_DeltaMouseX = 0;
	m_DeltaMouseY = 0;
	float3 normMouseVector = length(screenMouseVector) > 0 ? normalize(screenMouseVector): float3(0, 1, 0);
	float3 zViewAxis(0, 0, 1);
	
	
	float3 viewRotAxis = cross(zViewAxis, normMouseVector);

	float4x4 viewRotMatrix = rotation<float, 4, 4>(viewRotAxis, rotationAmt);
			

	hlsl::float3 translateOffset(0,0,0);

	hlsl::float4 perspectiveOffset(0.f,0.f,0.f,0.f);

	if(m_TurnStatus[TURN_DOWN]) //k
	{

		float aspectZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetAspect();
		float nearZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetNear();
		float fovyZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFovy();
		float farZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFar();

		
		float offset = -2.0f * m_TranslateSpeed * ElapsedTime;
		

		float3 y0Vector = float3(0, y0, z0);
		float3 y1Vector = float3(0, y1, z0);
		float3 eye(0.f,(m_eye.y+offset), m_eye.z);
		m_eye = float3(m_eye.x,(m_eye.y + offset), m_eye.z);

		/***************ausgaben********************/
		m_ConsoleListener->Append("i offset ");
		m_ConsoleListener->Append(offset);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("i x_x0 ");
		m_ConsoleListener->Append(y0Vector);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("i x_x1 ");
		m_ConsoleListener->Append(y1Vector);
		m_ConsoleListener->LineBreak();
		/********************************************/


		float3 dY0Z0 = float3(0.f, 0.f, z0);
		float3 y1_eye = (y1Vector - eye);
		float3 y0_eye = (y0Vector - eye);

		/***************ausgaben********************/
		m_ConsoleListener->Append("i x1 ");
		m_ConsoleListener->Append(y1_eye);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("i x0 ");
		m_ConsoleListener->Append(y0_eye);
		m_ConsoleListener->LineBreak();
		/*****************************************/

		float alpha = 0.5f*(hlsl::acos(dot(y0_eye,y1_eye)/(length(y1_eye) * length(y0_eye))));
		float gamma = hlsl::acos(length(dY0Z0)/(length(y0_eye)));
		float beta = alpha - gamma;

		/***************ausgaben********************/
		m_ConsoleListener->Append("i dot ");
		m_ConsoleListener->Append(dot(y0_eye,y1_eye));
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("i length ");
		m_ConsoleListener->Append((length(y1_eye) * length(y0_eye)));
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("i alpha ");
		m_ConsoleListener->Append(alpha);
		m_ConsoleListener->LineBreak();
		/******************************************/
		

		float aspectN = aspectZ0*(2*alpha)/(fovyZ0);
		static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->SetProjParams((2*alpha/(fovyZ0/aspectZ0)), 2*alpha, nearZ0, farZ0);


		/***************ausgaben********************/
		m_ConsoleListener->Append("i m_eye ");
		m_ConsoleListener->Append(m_eye);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("i aspectN ");
		m_ConsoleListener->Append(aspectN);
		m_ConsoleListener->LineBreak();
		/*******************************************/
	}
	if(m_TurnStatus[TURN_UP]) //i
	{
		//aktuelle Werte
		float aspectZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetAspect();
		float nearZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetNear();
		float fovyZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFovy();
		float farZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFar();
		
		//Veränderung/Bewegung des "Auges"
		float offset = 2.0f * m_TranslateSpeed * ElapsedTime;
		
		//X-Achse wird nicht betrachtet, um die Rechnungen zu vereinfachen --> darf ich das?
		//Berechnung der verschiedenen Punkte
		float3 y0Vector = float3(0, y0, z0);
		float3 y1Vector = float3(0, y1, z0);
		float3 eye(0.f,(m_eye.y+offset), m_eye.z);
		m_eye = float3(m_eye.x,(m_eye.y + offset), m_eye.z);



		/*****************ausgaben*******************/
		m_ConsoleListener->Append("k offset ");
		m_ConsoleListener->Append(offset);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("k x_x0 ");
		m_ConsoleListener->Append(y0Vector);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("k x_x1 ");
		m_ConsoleListener->Append(y1Vector);
		m_ConsoleListener->LineBreak();

		SYSLOG("CCameraController::Animate", 1, "k offset "<<offset<<br);

		/********************************************/

		//Berechnung der Vektoren
		float3 dY0Z0 = float3(0.f, 0.f, z0);
		float3 y1_eye = (y1Vector - eye);
		float3 y0_eye = (y0Vector - eye);

		/*****************ausgaben*******************/
		m_ConsoleListener->Append("k x1 ");
		m_ConsoleListener->Append(y1_eye);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("k x0 ");
		m_ConsoleListener->Append(y0_eye);
		m_ConsoleListener->LineBreak();
		/********************************************/

		//Berechnung der Winkel, wobei alpha der 0.5 * field of View ist, und beta die Winkeldrehung
		float alpha = 0.5f*(hlsl::acos(dot(y0_eye,y1_eye)/(length(y1_eye) * length(y0_eye))));
		float gamma = hlsl::acos(length(dY0Z0)/(length(y0_eye)));
		float beta = alpha - gamma;

		/*****************ausgaben*******************/
		m_ConsoleListener->Append("k dot ");
		m_ConsoleListener->Append(dot(y0_eye,y1_eye));
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("k length ");
		m_ConsoleListener->Append((length(y1_eye) * length(y0_eye)));
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("k alpha ");
		m_ConsoleListener->Append(alpha);
		m_ConsoleListener->LineBreak();
		/********************************************/
		

		float aspectN = aspectZ0*((2*alpha)/(fovyZ0));
		//die berechneten Werte werden gesetzt, wobei noch keine Drehung miteinbezogen wird
		static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->SetProjParams((2*alpha/(fovyZ0/aspectZ0)), 2*alpha, nearZ0, farZ0);


		/*****************ausgaben*******************/
		m_ConsoleListener->Append("k aspectN ");
		m_ConsoleListener->Append(aspectN);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("k m_eyes ");
		m_ConsoleListener->Append(m_eye);
		m_ConsoleListener->LineBreak();
		/********************************************/

	}
	if(m_TurnStatus[TURN_LEFT]) //j
	{
		float aspectZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetAspect();
		float nearZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetNear();
		float fovyZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFovy();
		float farZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFar();

		float offset = -2.0f * m_TranslateSpeed * ElapsedTime;

		float3 x0Vector = float3(x0, 0, z0);
		float3 x1Vector = float3(x1, 0, z0);
		float3 eye((m_eye.x + offset), 0.f, m_eye.z);
		m_eye = float3((m_eye.x + offset), m_eye.y, m_eye.z);

		/***************ausgaben********************/
		m_ConsoleListener->Append("J offset ");
		m_ConsoleListener->Append(offset);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("J x_x0 ");
		m_ConsoleListener->Append(x0Vector);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("J x_x1 ");
		m_ConsoleListener->Append(x1Vector);
		m_ConsoleListener->LineBreak();
		/*******************************************/

		float3 dX0Z0 = float3(0.f, 0, z0);
		float3 x1_eye = (x1Vector - eye);
		float3 x0_eye = (x0Vector - eye);

		/***************ausgaben********************/
		m_ConsoleListener->Append("J x1 ");
		m_ConsoleListener->Append(x1_eye);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("J x0 ");
		m_ConsoleListener->Append(x0_eye);
		m_ConsoleListener->LineBreak();
		/*******************************************/


		float alpha = 0.5f*(hlsl::acos(dot(x0_eye,x1_eye)/(length(x1_eye) * length(x0_eye))));
		float gamma = hlsl::acos(length(dX0Z0)/(length(x0_eye)));
		float beta = alpha - gamma;

		/***************ausgaben********************/
		m_ConsoleListener->Append("J dot ");
		m_ConsoleListener->Append(dot(x0_eye,x1_eye));
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("J beta ");
		m_ConsoleListener->Append(beta);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("J alpha ");
		m_ConsoleListener->Append(alpha);
		m_ConsoleListener->LineBreak();
		/******************************************/


		static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->SetProjParams(fovyZ0/(2*alpha), fovyZ0, nearZ0, farZ0);

		/***************ausgaben********************/ 
		m_ConsoleListener->Append("J m_eye ");
		m_ConsoleListener->Append(m_eye);
		m_ConsoleListener->LineBreak();
		/*******************************************/
	}
	if(m_TurnStatus[TURN_RIGHT]) //l
	{

		float aspectZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetAspect();
		float nearZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetNear();
		float fovyZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFovy();
		float farZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFar();
		
		float offset = 2.0f * m_TranslateSpeed * ElapsedTime;
		
		float3 x0Vector = float3(x0, 0, z0);
		float3 x1Vector = float3(x1, 0, z0);
		float3 eye((m_eye.x + offset), 0.f, m_eye.z);
		m_eye = float3((m_eye.x + offset), m_eye.y, m_eye.z);

		/***************ausgaben********************/
		m_ConsoleListener->Append("l offset ");
		m_ConsoleListener->Append(offset);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("l x_x0 ");
		m_ConsoleListener->Append(x0Vector);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("l x_x1 ");
		m_ConsoleListener->Append(x1Vector);
		m_ConsoleListener->LineBreak();
		/*******************************************/

		float3 dX0Z0 = float3(0.f, 0, z0);//(dX,0,z0)-eye
		float3 x1_eye = (x1Vector - eye);
		float3 x0_eye = (x0Vector - eye);

		/***************ausgaben********************/
		m_ConsoleListener->Append("l x1 ");
		m_ConsoleListener->Append(x1_eye);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("l x0 ");
		m_ConsoleListener->Append(x0_eye);
		m_ConsoleListener->LineBreak();
		/********************************************/

		float alpha = 0.5f*(hlsl::acos(dot(x0_eye,x1_eye)/(length(x1_eye) * length(x0_eye))));
		float gamma = hlsl::acos(length(dX0Z0)/(length(x0_eye)));
		float beta = alpha - gamma;

		/***************ausgaben********************/
		m_ConsoleListener->Append("l dot ");
		m_ConsoleListener->Append(dot(x0_eye,x1_eye));
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("l length ");
		m_ConsoleListener->Append((length(x1_eye) * length(x0_eye)));
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("l alpha ");
		m_ConsoleListener->Append(alpha);
		m_ConsoleListener->LineBreak();
		/******************************************/
		

		static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->SetProjParams(fovyZ0/(2*alpha), fovyZ0, nearZ0, farZ0);
	
		/***************ausgaben********************/
		m_ConsoleListener->Append("l m_eye ");
		m_ConsoleListener->Append(m_eye);
		m_ConsoleListener->LineBreak();
		/*******************************************/
	}
	if(m_TurnStatus[TURN_AWAY]) {//u
		float aspectZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetAspect();
		float nearZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetNear();
		float fovyZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFovy();
		float farZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFar();
		if ( (fovyZ0 - 2.0f * m_TranslateSpeed *ElapsedTime) > 0.0f) {
			fovyZ0 -= 2.0f * m_TranslateSpeed *ElapsedTime;
			//nearZ0 -= 2.0f *m_TranslateSpeed * ElapsedTime;
		}
		static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->SetProjParams(aspectZ0, fovyZ0, nearZ0, farZ0);
		m_ConsoleListener->Append("U z0 ");
		m_ConsoleListener->Append(perspectiveOffset);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("U CurrAngle ");
		m_ConsoleListener->Append(m_CurrAngleSpeed);
		m_ConsoleListener->LineBreak();
		m_ConsoleListener->Append("u offset ");
		m_ConsoleListener->Append(m_CurrTranslateSpeed);
		m_ConsoleListener->LineBreak();
	}
	if(m_TurnStatus[TURN_CLOSE]) {//o z0 verkleinern -> Fovy vergrößern
		float aspectZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetAspect();
		float nearZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetNear();
		float fovyZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFovy();
		float farZ0 = static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->GetFar();
		if ( (fovyZ0 + 2.0f * m_TranslateSpeed *ElapsedTime) < 3.14f) {
			fovyZ0 += 2.0f * m_TranslateSpeed *ElapsedTime;
			//nearZ0 += 2.0f * m_TranslateSpeed * ElapsedTime;
		}
		static_cast<RenderPack::CCamera*>(m_rControlTarget.GetPtr())->SetProjParams(aspectZ0, fovyZ0, nearZ0, farZ0);

	}


	m_CurrAngleSpeed += perspectiveOffset;
	m_CurrTranslateSpeed += translateOffset;

	/*if(length(m_CurrAngleSpeed) > m_TranslateSpeed)
		m_CurrAngleSpeed = m_TranslateSpeed * normalize(m_CurrAngleSpeed);
*/
	if(length(m_CurrTranslateSpeed) > m_TranslateSpeed)
		m_CurrTranslateSpeed = m_TranslateSpeed * normalize(m_CurrTranslateSpeed);

	if(!needUpdate && length(m_CurrTranslateSpeed) == 0.0f )
		return;

	translateOffset = m_CurrTranslateSpeed * ElapsedTime;
					
	perspectiveOffset = m_CurrAngleSpeed * ElapsedTime;




	hlsl::float4x4 viewAngleMatrix = translation<float,4,4>(hlsl::vector<float, 3>(translateOffset.x,translateOffset.y, translateOffset.z));

	hlsl::float4x4 objTransform = *m_rControlTarget->GetLocalTransform();

	hlsl::float4x4 objTransformInv = invert(objTransform);

	hlsl::float4x4 newLocalTransform = mul( (viewRotMatrix,viewAngleMatrix), objTransform);



	
	m_rControlTarget->SetLocalTransform(newLocalTransform);
		
		if(needUpdate)
			CFreeFlightController::Animate(Time, ElapsedTime);


	
}