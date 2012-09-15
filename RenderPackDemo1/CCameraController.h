	#ifndef _CCAMERA_CONTROLLER_H
	#define _CCAMERA_CONTROLLER_H

	#pragma once
	#include "Util\Logging\CWinConsoleListener.h"
	#include "SceneModel\Animation\CFreeFlightController.h"

#include "stdafx.h"



	namespace RenderPack {

		
		class CCameraController
			:public CFreeFlightController {
			
		public:
			CWinConsoleListener *m_ConsoleListener;
		protected:
			float x0;
			float x1;
			float y0;
			float y1;
			float z0;
			float z1;
			hlsl::float3 m_eye;
			bool flag;
			hlsl::float4 m_CurrAngleSpeed;
			enum STATUS_EYE {
				TURN_UP = 0,
				TURN_DOWN = 1,
				TURN_LEFT = 2,	
				TURN_RIGHT = 3,
				TURN_CLOSE = 4,
				TURN_AWAY = 5
			};
			
		public:
			
				CCameraController(CWinConsoleListener *pMyListener);
				virtual ~CCameraController();
				// IMessageHandler
			
				
				virtual void OnKeyDown(WPARAM wParam, LPARAM lParam, bool *pNeedFurtherProcessing);
				virtual void OnKeyUp(WPARAM wParam, LPARAM lParam, bool *pNeedFurherProcessing);
//				
				/*virtual void OnMouseMove(float MouseX, float MouseY, BYTE MouseButtonState, bool *pNeedFurtherProcessing);
				virtual void OnMouseDown(float MouseX, float MouseY, BYTE ActiveButton, BYTE MouseButtonState, bool *pNeedFurtherProcessing);
				virtual void OnMouseUp(float MouseX, float MouseY, BYTE ActiveButton, BYTE MouseButtonState, bool *pNeedFurtherProcessing);*/
				// IAnimatable

				virtual void Animate(double Time, float ElapsedTime);

				//
				
			void SetParams(float x_0, float x_1, float y_0, float y_1, float z_0, float z_1, hlsl::float3 eye); 

			void GetParams(float& x_0, float& x_1, float y_0, float y_1, float z_0, float z_1, hlsl::float3 eye);

		protected:

			
			bool m_TurnStatus[6];

			};

			typedef CReference<CCameraController> CCameraControllerRef;
	

	}
	#endif // _CCAMERA_CONTROLLER_H