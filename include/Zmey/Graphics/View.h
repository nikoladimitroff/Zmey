#pragma once

#include <Zmey/Math/Math.h>

namespace Zmey
{
namespace Graphics
{

struct FrameData;

enum class ViewType : unsigned char
{
	PlayerView,
	Count
};

class View
{
public:
	View(ViewType type)
		: m_Type(type)
	{}

	void SetupView(Vector3 target, Vector3 up, Vector3 position);
	void SetupProjection(unsigned width, unsigned height, float fov, float znear, float zfar);

	void GatherData(FrameData& data);
private:
	ViewType m_Type;

	Vector3 m_LookAtTarget;
	Vector3 m_UpDirection;
	Vector3 m_Position;

	unsigned m_Width;
	unsigned m_Height;
	float m_Fov;
	float m_ZNear;
	float m_ZFar;
};
}
}