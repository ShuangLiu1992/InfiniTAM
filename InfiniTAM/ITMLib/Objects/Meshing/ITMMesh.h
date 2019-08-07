// Copyright 2014-2017 Oxford University Innovation Limited and the authors of InfiniTAM

#pragma once

#include "../Scene/ITMVoxelBlockHash.h"
#include "../../../ORUtils/Image.h"

#include <stdlib.h>

namespace ITMLib
{
	class ITMMesh
	{
	public:
		struct Triangle { Vector3f p0, p1, p2; Vector3u c0, c1, c2; };

		MemoryDeviceType memoryType;

		uint noTotalTriangles;
		static const uint noMaxTriangles_default = SDF_LOCAL_BLOCK_NUM * 32 * 16;
		uint noMaxTriangles;

		ORUtils::MemoryBlock<Triangle> *triangles;

		explicit ITMMesh(MemoryDeviceType memoryType, uint maxTriangles = noMaxTriangles_default)
		{
			this->memoryType = memoryType;
			this->noTotalTriangles = 0;
			this->noMaxTriangles = maxTriangles;

			triangles = new ORUtils::MemoryBlock<Triangle>(noMaxTriangles, memoryType);
		}

		void WriteOBJ(const char *fileName)
		{
			ORUtils::MemoryBlock<Triangle> *cpu_triangles; bool shoulDelete = false;
			if (memoryType == MEMORYDEVICE_CUDA)
			{
				cpu_triangles = new ORUtils::MemoryBlock<Triangle>(noMaxTriangles, MEMORYDEVICE_CPU);
				cpu_triangles->SetFrom(triangles, ORUtils::MemoryBlock<Triangle>::CUDA_TO_CPU);
				shoulDelete = true;
			}
			else cpu_triangles = triangles;

			Triangle *triangleArray = cpu_triangles->GetData(MEMORYDEVICE_CPU);

			FILE *f = fopen(fileName, "w+");
			if (f != NULL)
			{
				for (uint i = 0; i < noTotalTriangles; i++)
				{
					fprintf(f, "v %f %f %f\n", triangleArray[i].p0.x, triangleArray[i].p0.y, triangleArray[i].p0.z);
					fprintf(f, "v %f %f %f\n", triangleArray[i].p1.x, triangleArray[i].p1.y, triangleArray[i].p1.z);
					fprintf(f, "v %f %f %f\n", triangleArray[i].p2.x, triangleArray[i].p2.y, triangleArray[i].p2.z);
				}

				for (uint i = 0; i<noTotalTriangles; i++) fprintf(f, "f %d %d %d\n", i * 3 + 2 + 1, i * 3 + 1 + 1, i * 3 + 0 + 1);
				fclose(f);
			}

			if (shoulDelete) delete cpu_triangles;
		}

		void WritePly(const char *fileName) {
                  ORUtils::MemoryBlock<Triangle> *cpu_triangles; bool shoulDelete = false;
                  if (memoryType == MEMORYDEVICE_CUDA)
                  {
                    cpu_triangles = new ORUtils::MemoryBlock<Triangle>(noMaxTriangles, MEMORYDEVICE_CPU);
                    cpu_triangles->SetFrom(triangles, ORUtils::MemoryBlock<Triangle>::CUDA_TO_CPU);
                    shoulDelete = true;
                  }
                  else cpu_triangles = triangles;

                  std::ofstream stream(fileName, std::ios::binary);

                  Triangle *triangleArray = cpu_triangles->GetData(MEMORYDEVICE_CPU);
                  stream << "ply"
                         << '\n' << "format "
                         << "binary_little_endian 1.0"
                         << '\n' << "element vertex " << noTotalTriangles * 3
                         << '\n' << "property float x"
                         << '\n' << "property float y"
                         << '\n' << "property float z"
                         << '\n' << "property uchar red"
                         << '\n' << "property uchar green"
                         << '\n' << "property uchar blue"
                         << '\n' << "element face " << noTotalTriangles
                         << '\n' << "property list uchar int vertex_index"
                         << '\n' << "end_header" << std::endl;

                    for (uint i = 0; i < noTotalTriangles; i++)
                    {
                      stream.write( reinterpret_cast<const char*> ( &triangleArray[i].p2 ), sizeof( Vector3f ) );
                      stream.write( reinterpret_cast<const char*> ( &triangleArray[i].c2 ), sizeof( Vector3u ) );
                      stream.write( reinterpret_cast<const char*> ( &triangleArray[i].p1 ), sizeof( Vector3f ) );
                      stream.write( reinterpret_cast<const char*> ( &triangleArray[i].c1 ), sizeof( Vector3u ) );
                      stream.write( reinterpret_cast<const char*> ( &triangleArray[i].p0 ), sizeof( Vector3f ) );
                      stream.write( reinterpret_cast<const char*> ( &triangleArray[i].c0 ), sizeof( Vector3u ) );

                    }
                  for (int i = 0; i < noTotalTriangles; i++)
                  {
                    uchar n = 3;
                    stream.write( reinterpret_cast<const char*> ( &n ), sizeof( uchar ) );
                    int f[3] = {i * 3 + 0, i * 3 + 1, i * 3 + 2};
                    stream.write( reinterpret_cast<const char*> ( &f[2] ), sizeof( int ) );
                    stream.write( reinterpret_cast<const char*> ( &f[1] ), sizeof( int ) );
                    stream.write( reinterpret_cast<const char*> ( &f[0] ), sizeof( int ) );

                  }

                  if (shoulDelete) delete cpu_triangles;
		}

		void WriteSTL(const char *fileName)
		{
			ORUtils::MemoryBlock<Triangle> *cpu_triangles; bool shoulDelete = false;
			if (memoryType == MEMORYDEVICE_CUDA)
			{
				cpu_triangles = new ORUtils::MemoryBlock<Triangle>(noMaxTriangles, MEMORYDEVICE_CPU);
				cpu_triangles->SetFrom(triangles, ORUtils::MemoryBlock<Triangle>::CUDA_TO_CPU);
				shoulDelete = true;
			}
			else cpu_triangles = triangles;

			Triangle *triangleArray = cpu_triangles->GetData(MEMORYDEVICE_CPU);

			FILE *f = fopen(fileName, "wb+");

			if (f != NULL) {
				for (int i = 0; i < 80; i++) fwrite(" ", sizeof(char), 1, f);

				fwrite(&noTotalTriangles, sizeof(int), 1, f);

				float zero = 0.0f; short attribute = 0;
				for (uint i = 0; i < noTotalTriangles; i++)
				{
					fwrite(&zero, sizeof(float), 1, f); fwrite(&zero, sizeof(float), 1, f); fwrite(&zero, sizeof(float), 1, f);

					fwrite(&triangleArray[i].p2.x, sizeof(float), 1, f); 
					fwrite(&triangleArray[i].p2.y, sizeof(float), 1, f); 
					fwrite(&triangleArray[i].p2.z, sizeof(float), 1, f);

					fwrite(&triangleArray[i].p1.x, sizeof(float), 1, f); 
					fwrite(&triangleArray[i].p1.y, sizeof(float), 1, f); 
					fwrite(&triangleArray[i].p1.z, sizeof(float), 1, f);

					fwrite(&triangleArray[i].p0.x, sizeof(float), 1, f);
					fwrite(&triangleArray[i].p0.y, sizeof(float), 1, f);
					fwrite(&triangleArray[i].p0.z, sizeof(float), 1, f);

					fwrite(&attribute, sizeof(short), 1, f);

					//fprintf(f, "v %f %f %f\n", triangleArray[i].p0.x, triangleArray[i].p0.y, triangleArray[i].p0.z);
					//fprintf(f, "v %f %f %f\n", triangleArray[i].p1.x, triangleArray[i].p1.y, triangleArray[i].p1.z);
					//fprintf(f, "v %f %f %f\n", triangleArray[i].p2.x, triangleArray[i].p2.y, triangleArray[i].p2.z);
				}

				//for (uint i = 0; i<noTotalTriangles; i++) fprintf(f, "f %d %d %d\n", i * 3 + 2 + 1, i * 3 + 1 + 1, i * 3 + 0 + 1);
				fclose(f);
			}

			if (shoulDelete) delete cpu_triangles;
		}

		~ITMMesh()
		{
			delete triangles;
		}

		// Suppress the default copy constructor and assignment operator
		ITMMesh(const ITMMesh&);
		ITMMesh& operator=(const ITMMesh&);
	};
}
