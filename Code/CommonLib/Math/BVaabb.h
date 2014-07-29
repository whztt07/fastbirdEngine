#pragma once
#include <CommonLib/Math/BoundingVolume.h>
#include <CommonLib/Math/AABB.h>
namespace fastbird
{
	class BVaabb: public BoundingVolume
	{
	public:
		BVaabb();

		
		virtual int GetBVType() const {return BV_AABB; }
		virtual void SetCenter (const Vec3& center);
		virtual void SetRadius (float fRadius);
		virtual const Vec3& GetCenter () const;
		virtual float GetRadius () const;

		virtual void ComputeFromData(const Vec3* pVertices, size_t numVertices);
		virtual void StartComputeFromData();
		virtual void AddComputeData(const Vec3* pVertices, size_t numVertices);
		virtual void EndComputeFromData();
		virtual void TransformBy(const Transformation& transform,
			BoundingVolume* result);
		virtual int WhichSide(const Vec3& min, const Vec3& max) const;
		virtual bool TestIntersection(const Ray3& ray) const;
		virtual bool TestIntersection(BoundingVolume* pBV) const;

		virtual void Merge(const BoundingVolume* pBV);
		virtual void Merge(const Vec3& pos);
		virtual BoundingVolume& operator= (const BoundingVolume& other);

		virtual fastbird::Vec3 GetSurfaceTo(const Vec3& target, Vec3& normal);
		virtual void Invalidate(){ mAABB.Invalidate(); }

		void SetAABB(const AABB& aabb);
		const AABB& GetAABB() const { return mAABB; }

		void Expand(float e);
	

	private:
		AABB mAABB;
		Vec3 mCenter;
	};
}