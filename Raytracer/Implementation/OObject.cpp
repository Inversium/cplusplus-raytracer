#include "../Headers/OObject.h" 

const RMaterialPBR RMaterialPBR::Metal = RMaterialPBR(Vector3(1.0), 0.25, 1.0, 1.0, 0.0, Vector3(0.0));
const RMaterialPBR RMaterialPBR::RedPlastic = RMaterialPBR(Vector3(1.0, 0.0, 0.0), 0.0, 0.0, 1.0, 0.0, Vector3(0.0));
const RMaterialPBR RMaterialPBR::YellowRubber = RMaterialPBR(Vector3(1.0, 1.0, 0.0), 1.0, 0.0, 1.0, 0.0, Vector3(0.0));
const RMaterialPBR RMaterialPBR::BluePlastic = RMaterialPBR(Vector3(0.1, 0.1, 1.0), 0.0, 0.0, 1.0, 0.0, Vector3(0.0));
const RMaterialPBR RMaterialPBR::Mirror = RMaterialPBR(Vector3(1.0), 0.0, 1.0, 1.0, 0.0, Vector3(0.0));
const RMaterialPBR RMaterialPBR::Glass = RMaterialPBR(Vector3(0.0), 0.0, 0.0, 1.4, 1.0, Vector3(0.0));
const RMaterialPBR RMaterialPBR::Diamond = RMaterialPBR(Vector3(0.0), 0.0, 0.0, 2.417, 1.0, Vector3(0.0));


bool OBox::Intersects(const RRay Ray, RHit& OutHit) const
{
	const Vector3 LocalRayOrigin = Ray.Origin - Transform.GetPosition();
	const Vector3 m = Vector3(1.0) / Ray.Direction;
	const Vector3 n = m * LocalRayOrigin;
	const Vector3 k = Vector3::Abs(m) * Extent;
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
	OutHit.Normal = {
		std::trunc(LocalHit.X / Extent.X * (1.0 + 1e-8)),
		std::trunc(LocalHit.Y / Extent.Y * (1.0 + 1e-8)),
		std::trunc(LocalHit.Z / Extent.Z * (1.0 + 1e-8))
	};
	if (bInsideBox) OutHit.Normal = -OutHit.Normal;

	return true;
}

void OBox::SetByMinMax(const Vector3& VMin, const Vector3& VMax)
{
	Transform.SetPosition((VMax + VMin) / 2);
	Extent = (VMax - VMin) / 2;
}

bool OSphere::Intersects(const RRay Ray, RHit& OutHit) const
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

bool OPlane::Intersects(const RRay Ray, RHit& OutHit) const
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

OMesh::OMesh(std::string Path)
{
	LoadModel(Path);
}

Vertex OMesh::GetVertex(uint32_t Index) const
{
	return Vertices[Index];
}

Vertex OMesh::GetVertex(uint32_t TriangleIndex, uint8_t LocalIndex) const
{
	return *Triangles[TriangleIndex].Vertices[LocalIndex];
}

bool OMesh::LoadModel(std::string Path)
{
	std::ifstream In;
	In.open(Path, std::ifstream::in);
	if (In.fail())
	{
		LOG("Mesh", LogType::ERROR, "Failed to load mesh %s", Path.c_str());
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
			Vertices.push_back(Vertex(V, Vector3(0.0)));
		}
		else if (Line.substr(0, 3) == "vn ")
		{
			iss >> Trash >> Trash;
			Vector3 N;
			iss >> N.X >> N.Y >> N.Z;
			//Normals.push_back(N.Normalized());
		}
		else if (Line.substr(0, 3) == "vt ")
		{
			iss >> Trash >> Trash;
			Vector2 UV;
			iss >> UV.X >> UV.Y;
			//UVs.push_back(UV);
		}
		else if (Line.substr(0, 2) == "f ")
		{
			iss >> Trash;
			uint32_t F, T, N;
			std::string S1, S2, S3;
			Triangle Tri;
			iss >> S1 >> S2 >> S3;
			
			F = std::stoi(S1);
			Tri.Vertices[0] = &Vertices[F - 1];
			F = std::stoi(S2);
			Tri.Vertices[1] = &Vertices[F - 1];
			F = std::stoi(S3);
			Tri.Vertices[2] = &Vertices[F - 1];

			Triangles.push_back(Tri);
		}
	}
	In.close();

	UpdateAABB();

	LOG("Mesh", LogType::LOG, "Successfully loaded mesh %s, V:%d, F:%d, Extent:(%s)",
		Path.c_str(),
		NVerts(),
		NFaces(),
		AABB.Extent.ToString().c_str());

	

	return true;
}

void OMesh::UpdateAABB()
{
	Vector3 Min(GetVertex(0).Position), Max(GetVertex(0).Position);

	//#pragma omp parallel for reduction(min,max:Min,Max)
	for (size_t i = 1; i < NVerts(); i++)
	{
		Vector3 V = GetVertex(i).Position;
		
		Min.X = std::min(Min.X, V.X);
		Min.Y = std::min(Min.Y, V.Y);
		Min.Z = std::min(Min.Z, V.Z);

		Max.X = std::max(Max.X, V.X);
		Max.Y = std::max(Max.Y, V.Y);
		Max.Z = std::max(Max.Z, V.Z);
	}

	AABB.SetByMinMax(Min, Max);
	AABB.Transform.SetPosition(AABB.Transform.GetPosition() + this->Transform.GetPosition());
}

uint32_t OMesh::NVerts() const
{
	return Vertices.size();
}

uint32_t OMesh::NFaces() const
{
	return Triangles.size();
}

/* Moller-Trumbore intersection algorithm */
bool OMesh::TriangleIntersect(const uint32_t FaceIndex, const RRay Ray, RHit& OutHit) const
{
	Vector3 Edge1 = GetVertex(FaceIndex, 1).Position - GetVertex(FaceIndex, 0).Position;
	Vector3 Edge2 = GetVertex(FaceIndex, 2).Position - GetVertex(FaceIndex, 0).Position;
	Vector3 P = Ray.Direction ^ Edge2;

	double Det = P | Edge1;
	if (std::abs(Det) < SMALL_NUMBER) return false;

	double InvDet = 1.0 / Det;

	Vector3 T = Ray.Origin - GetVertex(FaceIndex, 0).Position;
	double U = (T | P) * InvDet;
	if (U < 0.0 || U > 1.0) return false;

	Vector3 Q = T ^ Edge1;
	double V = (Ray.Direction | Q) * InvDet;
	if (V < 0.0 || U + V > 1.0) return false;

	if ((Edge2 | Q) * InvDet < SMALL_NUMBER) return false;

	OutHit.Mat = this->Mat;
	OutHit.Depth = (Edge2 | Q) * InvDet;
	OutHit.Position = Ray.Origin + Ray.Direction * OutHit.Depth;
	OutHit.Normal = (Edge1 ^ Edge2);

	return true;
}

bool OMesh::Intersects(const RRay Ray, RHit& OutHit) const
{
	RRay LocalRay;
	LocalRay.Origin = Transform.InverseTransformPosition(Ray.Origin);
	LocalRay.Direction = Transform.InverseTransformVector(Ray.Direction).Normalized();

	if (!AABB.Intersects(LocalRay, OutHit)) return false;

	double Distance = INFINITY;
	bool bHit = false;

	//#pragma omp parallel for
	for (size_t i = 0; i < NFaces(); i++)
	{
		RHit TempHit;
		if (TriangleIntersect(i, LocalRay, TempHit) && TempHit.Depth < Distance)
		{
			bHit = true;
			Distance = TempHit.Depth;
			OutHit = TempHit;
		}
	}

	if (bHit)
	{
		OutHit.Position = Transform.TransformPosition(OutHit.Position);
		OutHit.Normal = Transform.TransformVector(OutHit.Normal).Normalized();
		OutHit.Depth = (OutHit.Position - Ray.Origin).Length();
		return true;
	}

	return false;	
}