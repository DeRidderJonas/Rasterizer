#include "pch.h"
#include "Camera.h"
#include "SDL.h"
#include "EMathUtilities.h"
#include <iostream>

Camera::Camera(float screenWidth, float screenHeight, const Elite::FPoint3& position, const Elite::FVector3& forward, float FOVAngle, float nearPlaneZ, float farPlaneZ)
	: m_ScreenWidth{screenWidth}
	, m_ScreenHeight{screenHeight}
	, m_AspectRatio{screenWidth/screenHeight}
	, m_Position{position}
	, m_Forward{forward}
	, m_NearPlaneZ{nearPlaneZ}
	, m_FarPlaneZ{farPlaneZ}
	, m_FOVAngle{FOVAngle}
	, m_FOV{}
	, m_MousePos{}
	, m_AngleX{0.f}
	, m_AngleY{0.f}
	, m_ONB{}
	, m_ONBInvert{}
{
	UpdateProjection();
}

const Elite::FPoint3& Camera::GetPosition() const
{
	return m_Position;
}

void Camera::UpdateFOV()
{
	m_FOV = float(tan(m_FOVAngle / 2.f));
}

void Camera::SetScreen(float width, float height)
{
	m_ScreenWidth = width;
	m_ScreenHeight = height;
	m_AspectRatio = width / height;
}

const Elite::FMatrix4& Camera::GetONB() const
{
	return m_ONB;
}

const Elite::FMatrix4& Camera::GetView() const
{
	return m_ONBInvert;
}

const Elite::FMatrix4& Camera::GetProjection() const
{
	return m_Projection;
}

void Camera::Move(float elapsedSec)
{
	float moveSpeed{ 10.f };
	float FOVAngleSpeed{ 3.f };
	float cameraAngleSpeed{ 1/100.f };
	const Uint8* pKeysStates = SDL_GetKeyboardState(nullptr);
	

	//Movement
	if (pKeysStates[SDL_SCANCODE_W])
	{
		m_Position += -moveSpeed * elapsedSec * GetLocalForward();
	}
	if (pKeysStates[SDL_SCANCODE_S])
	{
		m_Position += moveSpeed * elapsedSec * GetLocalForward();
	}
	if (pKeysStates[SDL_SCANCODE_A])
	{
		Elite::FVector3 sideways{ Elite::Cross(GetLocalForward(), {0,1.f,0}) };
		m_Position += moveSpeed * elapsedSec * sideways;
	}
	if (pKeysStates[SDL_SCANCODE_D])
	{
		Elite::FVector3 sideways{ Elite::Cross(GetLocalForward(), {0,1.f,0}) };
		m_Position += -moveSpeed * elapsedSec * sideways;
	}
	if (pKeysStates[SDL_SCANCODE_Q])
	{
		Elite::FVector3 vertical{ 0,1,0 };
		m_Position += -moveSpeed * elapsedSec * vertical;
	}
	if (pKeysStates[SDL_SCANCODE_E])
	{
		Elite::FVector3 vertical{ 0,1,0 };
		m_Position += moveSpeed * elapsedSec * vertical;
	}

	//FOV
	bool fovUpdated{ false };
	if (pKeysStates[SDL_SCANCODE_LEFT])
	{
		if(m_FOVAngle - FOVAngleSpeed * elapsedSec > 0.f) m_FOVAngle += -FOVAngleSpeed * elapsedSec;
		fovUpdated = true;
	}
	if (pKeysStates[SDL_SCANCODE_RIGHT])
	{
		if(double(m_FOVAngle) + double(FOVAngleSpeed) * double(elapsedSec) < E_PI) m_FOVAngle += FOVAngleSpeed * elapsedSec;
		fovUpdated = true;
	}
	if (fovUpdated) UpdateProjection();
	
	//Mouse movement
	int x{}, y{};
	auto mouseButton = SDL_GetMouseState(&x, &y);

	if (mouseButton == SDL_BUTTON(SDL_BUTTON_RIGHT) || mouseButton == SDL_BUTTON(SDL_BUTTON_LEFT) || mouseButton == 5)
	{
		if (m_MousePos.data[0] != 0 && m_MousePos.data[1] != 0)
		{
			if (mouseButton == 5)
			{
				Elite::FVector3 vertical{ Elite::Cross(GetLocalForward(), {1.f,0,0}) };
				m_Position += (m_MousePos.data[1] - y > 0 ? 1 : -1) * moveSpeed * elapsedSec * vertical;
			}
			else
			{
				m_AngleX -= (m_MousePos.data[0] - x) * cameraAngleSpeed; //Rot over Y

				if (mouseButton == SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					m_Position += (m_MousePos.data[0] - y > 0 ? 1 : -1) * moveSpeed * elapsedSec * GetLocalForward();
				}
				else m_AngleY -= (m_MousePos.data[1] - y) * cameraAngleSpeed; //Rot over X
			}
		}
		
		//Update mouse pos to current pos
		m_MousePos.data[0] = float(x);
		m_MousePos.data[1] = float(y);
	}
	else
	{
		//if no mouse button held, clear mouse pos
		m_MousePos.data[0] = 0.f;
		m_MousePos.data[1] = 0.f;
	}

	UpdateONB();
}

float Camera::GetScreenWidth() const
{
	return m_ScreenWidth;
}

float Camera::GetScreenHeight() const
{
	return m_ScreenHeight;
}

bool Camera::FrustumCull(float z) const
{
	return z < m_NearPlaneZ || z > m_FarPlaneZ;
}

void Camera::SetHandedNess(bool isLeftHanded)
{
	m_IsLeftHanded = isLeftHanded;
	UpdateProjection();
	UpdateONB();
}

void Camera::UpdateONB()
{
	Elite::FVector4 position{ m_Position };	
	Elite::FVector3 forward{ GetLocalForward() };

	if (m_IsLeftHanded)
	{
		position.z *= -1;
		forward.x *= -1;
		forward.y *= -1;
	}

	Elite::FVector3 worldUp{ 0,1,0 };
	Elite::FVector3 right{ Elite::Cross(worldUp, forward) };
	Elite::Normalize(right);
	Elite::FVector3 up{ Elite::Cross(forward, right) };
	Elite::Normalize(up);

	m_ONB = Elite::FMatrix4(Elite::FVector4{ right }, Elite::FVector4{ up }, Elite::FVector4{ forward }, Elite::FVector4{ position });
	m_ONB.data[3][3] = 1;

	m_ONBInvert = Elite::Inverse(m_ONB);
}

void Camera::UpdateProjection()
{
	UpdateFOV();

	m_Projection[0][0] = 1 / (m_AspectRatio * m_FOV);
	m_Projection[1][1] = 1 / m_FOV;
	m_Projection[2][2] = m_IsLeftHanded ? (m_FarPlaneZ / (m_FarPlaneZ - m_NearPlaneZ)) : (m_FarPlaneZ / (m_NearPlaneZ - m_FarPlaneZ));
	m_Projection[3][2] = m_IsLeftHanded ? (-(m_FarPlaneZ * m_NearPlaneZ) / (m_FarPlaneZ - m_NearPlaneZ)) : ((m_FarPlaneZ * m_NearPlaneZ) / (m_NearPlaneZ - m_FarPlaneZ));
	m_Projection[2][3] = m_IsLeftHanded ? 1.f : -1.f;
}

const Elite::FVector3 Camera::GetLocalForward() const
{
	Elite::FVector3 localForward{ m_Forward };
	localForward = Elite::MakeRotationX(m_AngleY) * localForward;
	localForward = Elite::MakeRotationY(m_AngleX) * localForward;

	return localForward;
}
