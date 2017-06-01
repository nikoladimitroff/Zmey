#include <Zmey/Graphics/View.h>

#include <Zmey/Graphics/FrameData.h>

namespace Zmey
{
namespace Graphics
{

void View::SetupView(Vector3 target, Vector3 up, Vector3 position)
{
	m_LookAtTarget = target;
	m_UpDirection = up;
	m_Position = position;
}

void View::SetupProjection(unsigned width, unsigned height, float fov, float znear, float zfar)
{
	m_Width = width;
	m_Height = height;
	m_Fov = fov;
	m_ZNear = znear;
	m_ZFar = zfar;
}

void View::GatherData(FrameData& data)
{
	data.Type = m_Type;
	data.ProjectionMatrix = glm::perspectiveFov(m_Fov, float(m_Width), float(m_Height), m_ZNear, m_ZFar);
	data.ViewMatrix = glm::lookAt(m_Position, m_LookAtTarget, m_UpDirection);
}
}
}