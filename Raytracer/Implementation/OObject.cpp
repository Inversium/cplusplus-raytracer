#include "../Headers/OObject.h" 


bool OBox::Intersects(const RRay& Ray, RHit& OutHit) const
{
	const Vector3 LocalRayOrigin = Ray.Origin - Transform.GetPosition();
	const Vector3 m = Vector3(1.0) / Ray.Direction;
	const Vector3 n = m * LocalRayOrigin;
	const Vector3 k = m.Abs() * Extent;
	const Vector3 t1 = -n - k;
	const Vector3 t2 = -n + k;
	const double tN = t1.GetMax(); //Near point distance
	const double tF = t2.GetMin(); //Far point distance

	if (tN > tF || tF < 0.0) return false;


	//If the ray origin is inside the box (tN < 0) the actual intersection position will be at (RayDir * tF)
	//Same goes for Depth
	const bool bInsideBox = tN < 0.0;
	OutHit.Position = Transform.GetPosition() + (bInsideBox ? LocalRayOrigin + Ray.Direction * tF : LocalRayOrigin + Ray.Direction * tN);
	OutHit.Depth = bInsideBox ? tF : tN;

	OutHit.Mat = Mat;

	const Vector3 LocalHit = OutHit.Position - Transform.GetPosition();
	Vector3 Normal = {
		std::trunc(LocalHit.X / Extent.X * (1.0 + 1e-8)),
		std::trunc(LocalHit.Y / Extent.Y * (1.0 + 1e-8)),
		std::trunc(LocalHit.Z / Extent.Z * (1.0 + 1e-8))
	};
	if (bInsideBox) Normal = -Normal;
	OutHit.Normal = Normal.Normalized();

	return true;
}

void OBox::SetByMinMax(const Vector3& VMin, const Vector3& VMax)
{
	Transform.SetPosition((VMax + VMin) / 2);
	Extent = (VMax - VMin) / 2;
}

bool OSphere::Intersects(const RRay& Ray, RHit& OutHit) const
{
	RRay LocalRay;
	LocalRay.Direction = Transform.InverseTransformVector(Ray.Direction).Normalized();
	LocalRay.Origin = Transform.InverseTransformPosition(Ray.Origin);
	const Vector3 L = -LocalRay.Origin; //Vector from Ray origin to Sphere position

	const double tca = L | LocalRay.Direction;
	if (tca < 0) return false;

	const double d2 = (L | L) - tca * tca; //Distance from Sphere position to ray
	if (d2 > Radius * Radius) return false;

	const double HalfInner = sqrt(Radius * Radius - d2); //half of the ray length inside sphere

	double t0 = tca - HalfInner;
	double t1 = tca + HalfInner;

	bool bInside = false;
	if (t0 < 0)
	{
		bInside = true;
		t0 = t1;
		if (t0 < 0) return false;
	}

	const Vector3 LocalHitPoint = LocalRay.Origin + LocalRay.Direction * t0;
	const Vector3 HitPoint = Transform.TransformPosition(LocalHitPoint);
	OutHit.Mat = this->Mat;
	OutHit.Normal = bInside ? -(HitPoint - Transform.GetPosition()).Normalized() : (HitPoint - Transform.GetPosition()).Normalized();
	OutHit.Position = HitPoint;
	OutHit.Depth = t0;

	return true;
}

bool OPlane::Intersects(const RRay& Ray, RHit& OutHit) const
{
	const double Denom = Normal | Ray.Direction;
	if (abs(Denom) > 1e-10)
	{
		const double T = ((Transform.GetPosition() - Ray.Origin) | Normal) / Denom;
		if (T >= 1e-5)
		{
			OutHit.Mat = this->Mat;
			OutHit.Normal = this->Normal;
			OutHit.Position = Ray.Origin + Ray.Direction * T;
			OutHit.Depth = T;
			return true;
		}
	}
	return false;
}

OMesh::OMesh(const char* Path)
{
	LoadModel(Path);
}

bool OMesh::LoadModel(const std::string& Path)
{
	Triangles.clear();
	Vertices.clear();


	std::ifstream In;
	In.open(Path, std::ifstream::in);
	if (In.fail())
	{
		LOG("Mesh", LogType::ERROR, "Failed to load mesh {}", Path);
		return false;
	}

	std::string Line;
	while (!In.eof())
	{
		std::getline(In, Line);
		std::istringstream iss(Line.c_str());
		char Trash;
		if (Line.substr(0, 2) == "v ")
		{
			iss >> Trash;
			Vector3 V;
			iss >> V.X >> V.Y >> V.Z;
			Vertices.push_back(MakeShared<Vertex>(V, Vector3(0.0)));
		}
		else if (Line.substr(0, 3) == "vn ")
		{
			iss >> Trash >> Trash;
			Vector3 N;
			iss >> N.X >> N.Y >> N.Z;			
		}
		else if (Line.substr(0, 3) == "vt ")
		{
			iss >> Trash >> Trash;
			Vector2 UV;
			iss >> UV.X >> UV.Y;			
		}
		else if (Line.substr(0, 2) == "f ")
		{
			iss >> Trash;
			uint32_t F, T, N;
			std::string S1, S2, S3;
			auto Tri = MakeShared<Triangle>();
			iss >> S1 >> S2 >> S3;
			
			F = std::stoi(S1);
			Tri->SetVertex(0, Vertices[F - 1]);
			F = std::stoi(S2);
			Tri->SetVertex(1, Vertices[F - 1]);
			F = std::stoi(S3);
			Tri->SetVertex(2, Vertices[F - 1]);

			Triangles.push_back(Tri);
		}
	}
	In.close();

	UpdateAABB();
	UpdateSmoothNormals();

	LOG("Mesh", LogType::LOG, "Successfully loaded mesh {}, V:{}, F:{}, Extent:({})",
		Path,
		CountVerts(),
		CountFaces(),
		BBox.GetExtent().ToString());
	
	return true;
}

void OMesh::UpdateAABB()
{
	Vector3 Min(DBL_MAX), Max(-DBL_MAX);

	for (size_t i = 0; i < CountVerts(); i++)
	{
		Vector3 V = Vertices[i]->Position;
		
		Min.X = std::min(Min.X, V.X);
		Min.Y = std::min(Min.Y, V.Y);
		Min.Z = std::min(Min.Z, V.Z);

		Max.X = std::max(Max.X, V.X);
		Max.Y = std::max(Max.Y, V.Y);
		Max.Z = std::max(Max.Z, V.Z);
	}

	BBox.Min = Min;
	BBox.Max = Max;
}

AABB Triangle::GetBoundingBox() const
{
	Vector3 Max, Min;

	const Vector3 P1 = Transform.TransformPosition(Vertices[0]->Position);
	const Vector3 P2 = Transform.TransformPosition(Vertices[1]->Position);
	const Vector3 P3 = Transform.TransformPosition(Vertices[2]->Position);

	Max.X = std::max(std::max(P1.X, P2.X), P3.X);
	Max.Y = std::max(std::max(P1.Y, P2.Y), P3.Y);
	Max.Z = std::max(std::max(P1.Z, P2.Z), P3.Z);

	Min.X = std::min(std::min(P1.X, P2.X), P3.X);
	Min.Y = std::min(std::min(P1.Y, P2.Y), P3.Y);
	Min.Z = std::min(std::min(P1.Z, P2.Z), P3.Z);

	return AABB(Min, Max);
}

/* Moller-Trumbore intersection algorithm */
bool Triangle::Intersects(const RRay& Ray, RHit& OutHit) const
{
	RRay LocalRay;
	LocalRay.Origin = Transform.InverseTransformPosition(Ray.Origin);
	LocalRay.Direction = Transform.InverseTransformVector(Ray.Direction).Normalized();

	const Vertex V1 = *Vertices[0];
	const Vertex V2 = *Vertices[1];
	const Vertex V3 = *Vertices[2];

	Vector3 Edge1 = V2.Position - V1.Position;
	Vector3 Edge2 = V3.Position - V1.Position;
	Vector3 P = LocalRay.Direction ^ Edge2;

	double Det = P | Edge1;
	if (std::abs(Det) < SMALL_NUMBER) return false;

	double InvDet = 1.0 / Det;

	Vector3 T = LocalRay.Origin - V1.Position;
	const double U = (T | P) * InvDet;
	if (U < 0.0 || U > 1.0) return false;

	Vector3 Q = T ^ Edge1;
	const double V = (LocalRay.Direction | Q) * InvDet;
	if (V < 0.0 || U + V > 1.0) return false;

	if ((Edge2 | Q) * InvDet < SMALL_NUMBER) return false;

	OutHit.Mat = this->Mat;	
	OutHit.Position = Transform.TransformPosition(LocalRay.Origin + LocalRay.Direction * (Edge2 | Q) * InvDet);
	OutHit.Depth = (Ray.Origin - OutHit.Position).Length();
	
	if (bSmoothShading)
	{
		OutHit.Normal = U * V2.Normal + V * V3.Normal + (1.0 - U - V) * V1.Normal;
	}
	else
	{
		OutHit.Normal = (Edge1 ^ Edge2).Normalized();
	}
	OutHit.Normal = Transform.TransformVector(OutHit.Normal).Normalized();

	return true;
}

void OMesh::UpdateSmoothNormals()
{
	#pragma omp parallel for
	for (int32_t i = 0; i < CountFaces(); i++)
	{
		const Vector3 Normal = Triangles[i]->RawNormal();
		Triangles[i]->GetVertex(0)->Normal += Normal;
		Triangles[i]->GetVertex(1)->Normal += Normal;
		Triangles[i]->GetVertex(2)->Normal += Normal;
	}
	
	#pragma omp parallel for
	for (int32_t i = 0; i < CountVerts(); i++)
	{
		Vertices[i]->Normal = Vertices[i]->Normal.Normalized();
	}
}

bool OMesh::Intersects(const RRay& Ray, RHit& OutHit) const
{
	RRay LocalRay;
	LocalRay.Origin = Transform.InverseTransformPosition(Ray.Origin);
	LocalRay.Direction = Transform.InverseTransformVector(Ray.Direction).Normalized();


	if (!BBox.Intersects(LocalRay)) return false;

	double Distance = INFINITY;
	bool bHit = false;

	#pragma omp parallel for
	for (int32_t i = 0; i < CountFaces(); i++)
	{
		RHit TempHit;
		if (Triangles[i]->Intersects(Ray, TempHit) && TempHit.Depth < Distance)
		{
			bHit = true;
			Distance = TempHit.Depth;
			OutHit = TempHit;
		}
	}

	return bHit;	
}