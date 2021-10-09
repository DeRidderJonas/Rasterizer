#pragma once
#include "EMath.h"

class Camera final
{
public:
	Camera(float screenWidth, float screenHeight, const Elite::FPoint3& position = { 0,0,0 }, const Elite::FVector3& forward = {0,0,1}, 
		float FOVAngle = E_PI_DIV_2, float nearPlaneZ = 0.1f, float farPlaneZ = 100.f);
	const Elite::FPoint3& GetPosition() const;
	void UpdateFOV();
	void SetScreen(float width, float height);
	const Elite::FMatrix4& GetONB() const;
	const Elite::FMatrix4& GetView() const;
	const Elite::FMatrix4& GetProjection() const;
	void Move(float elapsedSec);
	float GetScreenWidth() const;
	float GetScreenHeight() const;
	bool FrustumCull(float z) const;
	void SetHandedNess(bool isLeftHanded);
private:
	float m_ScreenWidth;
	float m_ScreenHeight;
	float m_AspectRatio;
	float m_FOVAngle; //radians
	float m_FOV;
	Elite::FPoint3 m_Position;
	Elite::FVector3 m_Forward;

	float m_NearPlaneZ;
	float m_FarPlaneZ;

	float m_AngleX;
	float m_AngleY;

	Elite::FPoint2 m_MousePos;

	Elite::FMatrix4 m_ONB;
	Elite::FMatrix4 m_ONBInvert;
	Elite::FMatrix4 m_Projection{};

	bool m_IsLeftHanded = true;

	void UpdateONB();
	void UpdateProjection();

	const Elite::FVector3 GetLocalForward() const;
};

