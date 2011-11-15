#include "Core.h"

#if RENDERING

#include "UnrealClasses.h"

#include "StaticMesh.h"
#include "MeshInstance.h"


#define SHOW_TANGENTS			1
#define SORT_BY_OPACITY			1


#define MAX_MESHMATERIALS		256


void CStatMeshInstance::Draw()
{
	guard(CStatMeshInstance::Draw);
	int i;

	/*const*/ CStaticMeshLod& Mesh = pMesh->Lods[LodNum];	//?? not 'const' because of BuildTangents(); change this?
	int NumSections = Mesh.Sections.Num();
	if (!NumSections || !Mesh.NumVerts) return;

	if (!Mesh.HasTangents) Mesh.BuildTangents();

	// copy of CSkelMeshInstance::Draw sorting code
#if SORT_BY_OPACITY
	// sort sections by material opacity
	int SectionMap[MAX_MESHMATERIALS];
	int secPlace = 0;
	for (int opacity = 0; opacity < 2; opacity++)
	{
		for (i = 0; i < NumSections; i++)
		{
			UUnrealMaterial *Mat = Mesh.Sections[i].Material;
			int op = 0;			// sort value
			if (Mat && Mat->IsTranslucent()) op = 1;
			if (op == opacity) SectionMap[secPlace++] = i;
		}
	}
	assert(secPlace == NumSections);
#endif // SORT_BY_OPACITY

	// draw mesh
	glEnable(GL_LIGHTING);
	for (i = 0; i < NumSections; i++)
	{
#if SORT_BY_OPACITY
		int MaterialIndex = SectionMap[i];
#else
		int MaterialIndex = i;
#endif
		if (UVIndex == 0)
			SetMaterial(Mesh.Sections[MaterialIndex].Material, MaterialIndex, 0);
		else
			BindDefaultMaterial();

		// check tangent space
		GLint aTangent = -1, aBinormal = -1;
		bool hasTangent = false;
		const CShader *Sh = GCurrentShader;
		if (Sh)
		{
			aTangent   = Sh->GetAttrib("tangent");
			aBinormal  = Sh->GetAttrib("binormal");
			hasTangent = (aTangent >= 0 && aBinormal >= 0);
		}
		if (hasTangent)
		{
			glEnableVertexAttribArray(aTangent);
			glEnableVertexAttribArray(aBinormal);
			glVertexAttribPointer(aTangent,  3, GL_FLOAT, GL_FALSE, sizeof(CStaticMeshVertex), &Mesh.Verts[0].Tangent);
			glVertexAttribPointer(aBinormal, 3, GL_FLOAT, GL_FALSE, sizeof(CStaticMeshVertex), &Mesh.Verts[0].Binormal);
		}

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(CStaticMeshVertex), &Mesh.Verts[0].Position);
		glNormalPointer(GL_FLOAT, sizeof(CStaticMeshVertex), &Mesh.Verts[0].Normal);
		glTexCoordPointer(2, GL_FLOAT, sizeof(CStaticMeshVertex), &Mesh.Verts[0].UV[UVIndex].U);
		//?? place this code into CIndexBuffer?
		const CStaticMeshSection &Sec = Mesh.Sections[MaterialIndex];
		if (Mesh.Indices.Is32Bit())
			glDrawElements(GL_TRIANGLES, Sec.NumFaces * 3, GL_UNSIGNED_INT, &Mesh.Indices.Indices32[Sec.FirstIndex]);
		else
			glDrawElements(GL_TRIANGLES, Sec.NumFaces * 3, GL_UNSIGNED_SHORT, &Mesh.Indices.Indices16[Sec.FirstIndex]);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);

		// disable tangents
		if (hasTangent)
		{
			glDisableVertexAttribArray(aTangent);
			glDisableVertexAttribArray(aBinormal);
		}
	}
	glDisable(GL_LIGHTING);
	BindDefaultMaterial(true);

	// draw mesh normals
	if (bShowNormals)
	{
		int NumVerts = Mesh.NumVerts;
		glBegin(GL_LINES);
		glColor3f(0.5, 1, 0);
		for (i = 0; i < NumVerts; i++)
		{
			glVertex3fv(Mesh.Verts[i].Position.v);
			CVec3 tmp;
			VectorMA(Mesh.Verts[i].Position, 2, Mesh.Verts[i].Normal, tmp);
			glVertex3fv(tmp.v);
		}
#if SHOW_TANGENTS
		glColor3f(0, 0.5f, 1);
		for (i = 0; i < NumVerts; i++)
		{
			const CVec3 &v = Mesh.Verts[i].Position;
			glVertex3fv(v.v);
			CVec3 tmp;
			VectorMA(v, 2, Mesh.Verts[i].Tangent, tmp);
			glVertex3fv(tmp.v);
		}
		glColor3f(1, 0, 0.5f);
		for (i = 0; i < NumVerts; i++)
		{
			const CVec3 &v = Mesh.Verts[i].Position;
			glVertex3fv(v.v);
			CVec3 tmp;
			VectorMA(v, 2, Mesh.Verts[i].Binormal, tmp);
			glVertex3fv(tmp.v);
		}
#endif // SHOW_TANGENTS
		glEnd();
	}

	unguard;
}


#endif // RENDERING