#pragma once


#include <cmath>

namespace Cy
{
	typedef struct v2
	{
		float x, y;
		v2()
		{
			
		}
		v2(const float x, const float y)
		{
			this->x = x;
			this->y = y;
		}
		v2(const v2& v)
		{
			this->x = v.x;
			this->y = v.y;
		}
		
		v2 operator+(const v2 v) const
		{
			return v;
		}
		v2 operator+(const v2& a1, const v2& a2)
		{
			return v2(a1.x + a2.x, a1.y + a2.y);
		}
		v2 operator+(const v2& a1, float scalar)
		{
			return v2(a1.x + scalar, a1.y + scalar);
		}

		v2 operator-(const v2 v) const
		{
			return v2(-v.x, -v.y);
		}
		v2 operator-(const v2& a1, const v2& a2)
		{
			return v2(a1.x - a2.x, a1.y - a2.y);
		}
		v2 operator-(const v2& a1, float scalar)
		{
			return v2(a1.x - scalar, a1.y - scalar);
		}

		v2 operator*(const v2& a1, const v2& a2)
		{
			return v2(a1.x * a2.x, a1.y * a2.y);
		}
		v2 operator*(const v2& a1, float scalar)
		{
			return v2(a1.x * scalar, a1.y * scalar);
		}

	} v2;

	typedef struct v3
	{
		float x, y, z;
		v3()
		{
			
		}
		v3(const float x, const float y, const float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		v3(const v3& v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
		}

		v3 operator+(const v3 v) const
		{
			return v;
		}
		v3 operator+(const v3& a1, const v3& a2)
		{
			return v3(a1.x + a2.x, a1.y + a2.y, a1.z + a2.z);
		}
		v3 operator+(const v3& a1, float scalar)
		{
			return v3(a1.x + scalar, a1.y + scalar, a1.z + scalar);
		}

		v3 operator-(const v3 v) const
		{
			return v3(-v.x, -v.y, -v.z);
		}
		v3 operator-(const v3& a1, const v3& a2)
		{
			return v3(a1.x - a2.x, a1.y - a2.y, a1.z - a2.z);
		}
		v3 operator-(const v3& a1, float scalar)
		{
			return v3(a1.x - scalar, a1.y - scalar, a1.z - scalar);
		}

		//element wise multiplication
		v3 operator*(const v3& a1, const v3& a2)
		{
			return v3(a1.x * a2.x, a1.y * a2.y, a1.z * a2.z);
		}
		v3 operator*(const v3& a1, float scalar)
		{
			return v3(a1.x * scalar, a1.y * scalar, a1.z * scalar);
		}
	} v3;

	typedef struct v4
	{
		float x, y, z, w;
		v4()
		{
			
		}
		v4(const float x, const float y, const float z, const float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		v4(const v4& v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
			this->w = v.w;
		}

		v4 operator+(const v4 v) const
		{
			return v;
		}
		v4 operator+(const v4& a1, const v4& a2)
		{
			return v4(a1.x + a2.x, a1.y + a2.y, a1.z + a2.z, a1.w + a2.w);
		}
		v4 operator+(const v4& a1, float scalar)
		{
			return v4(a1.x + scalar, a1.y + scalar, a1.z + scalar, a1.w + scalar);
		}

		v4 operator-(const v4 v) const
		{
			return v4(-v.x, -v.y, -v.z, -v.w);
		}
		v4 operator-(const v4& a1, const v4& a2)
		{
			return v4(a1.x - a2.x, a1.y - a2.y, a1.z - a2.z, a1.w - a2.w);
		}
		v4 operator-(const v4& a1, float scalar)
		{
			return v4(a1.x - scalar, a1.y - scalar, a1.z - scalar, a1.w - scalar);
		}

		//element wise multiplication
		v4 operator*(const v4& a1, const v4& a2)
		{
			return v4(a1.x * a2.x, a1.y * a2.y, a1.z * a2.z, a1.w * a2.w);
		}
		v4 operator*(const v4& a1, float scalar)
		{
			return v4(a1.x * scalar, a1.y * scalar, a1.z * scalar, a1.w * scalar);
		}
	} v4;

	typedef struct m2
	{
		union
		{
			v2 value[2];
			v2 x, y;
		};

	} m2;

	typedef struct m3
	{
		union
		{
			v3 value[3];
			v3 x, y, z;
		};
		m3()
		{
			
		}
	} m3;

	typedef struct m4
	{
		union
		{
			v4 value[4];
			v4 x, y, z, w;
		};
		m4()
		{
			
		}
		m4(const m4& other)
		{
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
			this->w = other.w;	
		}
		
	} m4;


	inline v2 Norm(v2 a)
	{
		float len = sqrt(a.x * a.x + a.y * a.y);
		return v2(a.x / len, a.y / len);
	}

	inline v3 Norm(v3 a)
	{
		float len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
		return v3(a.x / len, a.y / len, a.z / len);
	}
	inline v4 Norm(v4 a)
	{
		float len = sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
		return v4(a.x / len, a.y / len, a.z / len, a.w/len);
	}

	inline v3 Cross(v3 a1, v3 a2)
	{
		return v3(	a1.y * a2.z - a2.y * a1.z,
					a1.z * a2.x - a2.z * a1.x,
					a1.x * a2.y - a2.x * a1.y);
	}

	inline float Dot(v2 a1, v2 a2)
	{
		return a1.x*a2.x + a1.y*a2.y;
	}

	inline float Dot(v3 a1, v3 a2)
	{
		return a1.x*a2.x + a1.y*a2.y + a1.z*a2.z;
	}

	inline float Dot(v4 a1, v4 a2)
	{
		return a1.x*a2.x + a1.y*a2.y + a1.z*a2.z + a1.w*a2.w;
	}

	inline m4 LookAt(const v3 eye, const v3 center, const v3 up)
	{
		v3 f = v3(Norm(center - eye));
		v3 s = v3(Norm(Cross(f, up)));
		v3 u = v3(Cross(s, f));

		m4 res;
		res.x.x = s.x;
		res.y.x = s.y;
		res.z.x = s.z;

		res.x.y = u.x;
		res.y.y = u.y;
		res.z.y = u.z;

		res.x.z = -f.x;
		res.y.z = -f.y;
		res.z.z = -f.z;

		res.w.x = -Dot(s, eye);
		res.w.y = -Dot(u, eye);
		res.w.z = -Dot(f, eye);

		return res;
	}

	inline m4 Perspective(float fieldOfView, float aspect, float zNear, float zFar)
	{
		float tanHalfFieldOfView = tan(fieldOfView / 2.0f);
		m4 res;
		res.x.x = 1.0f / (aspect * tanHalfFieldOfView);
		res.y.y = 1.0f / (tanHalfFieldOfView);
		res.z.z = -(zFar + zNear) / (zFar - zNear);
		res.z.w = -1.0f;
		res.w.z = (2.0f * zFar * zNear) / (zFar - zNear);

		return res;
	}

	inline m4 Translate(m4 m, v3 v)
	{
		m4 res = m4(m);
	}

}
