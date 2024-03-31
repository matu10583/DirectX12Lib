#pragma once
/*
#include "D3D12FrameWork/ComPtr.h"
#include "D3D12FrameWork/Common.h"
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <span>
#include <string>
#include <fbxsdk.h>

namespace D3D12FrameWork {

	template<typename T>
class ModelLoader
{
	template<typename U, typename = void>
	struct HasPosition :std::false_type {};
	template<typename U>
	struct HasPosition<U, std::void_t<typename U::Position>> :std::true_type {};

public:
	struct ModelData
	{
		std::vector<T> Vertices;
		std::vector<unsigned short> Indices;
	};

	static bool Load(std::filesystem::path const& file_path,
		std::span<T>& _outVertices,
		std::span<unsigned short>& _outIndices) {
		if (!std::filesystem::exists(file_path)) {
			assert(false);
			return false;
		}
		if (m_modelDataMap.count(file_path.string())) {
			auto& mdata = m_modelDataMap[file_path.string()];
			_outVertices = std::span{ mdata.Vertices };
			_outIndices = std::span{ mdata.Indices };
		}



		//別スレッドにしたい
		m_modelLoaders[file_path.extension().string()](
			file_path.string().c_str(), _outVertices, _outIndices
			);
		return true;
	}
private:
	ModelLoader() {}
	~ModelLoader() {}
	DECLMOVECOPY(ModelLoader);
	static std::unordered_map<std::string, std::function<bool(char const*, std::span<T>&,
		std::span<unsigned short>&)>>
		m_modelLoaders;
	static std::unordered_map<std::string, ModelData> m_modelDataMap;

	static bool LoadFBX(char const* _pFname, std::span<T>& _outVertices,
		std::span<unsigned short>& _outIndices) {
		fbxsdk::FbxManager* fbx_Manager = fbxsdk::FbxManager::Create();
		//iosettingsを作製
		FbxIOSettings* ioSettings = FbxIOSettings::Create(fbx_Manager, IOSROOT);
		//importerを生成
		auto importer = FbxImporter::Create(fbx_Manager, "");
		if (importer->Initialize(_pFname, -1, fbx_Manager->GetIOSettings()) == false) {
			return false;
		}
		//シーン作製
		FbxScene* scene = FbxScene::Create(fbx_Manager, "");
		importer->Import(scene);
		importer->Destroy();

		//三角ポリゴンへコンバート
		FbxGeometryConverter geometryConverter(fbx_Manager);
		if (!geometryConverter.Triangulate(scene, true)) {
			assert(false);
			return false;
		}
		//メッシュ取得
		auto mesh = scene->GetSrcObject<FbxMesh>();
		if (!mesh) {
			assert(false);
			return false;
		}
		//uvセット名の取得
		FbxStringList uvSetNameList;
		mesh->GetUVSetNames(uvSetNameList); const char* uvSetName = uvSetNameList.GetStringAt(0);

		auto& modelData = m_modelDataMap[_pFname];
		std::vector<T>& vertices = modelData.Vertices;
		std::vector<unsigned short>& indices = modelData.Indices;

		//頂点リストの生成。
		vertices.reserve(mesh->GetControlPointsCount());

		for (auto polIdx = 0u; polIdx < mesh->GetPolygonCount(); polIdx++) {
			for (auto polVertIdx = 0u; polVertIdx < mesh->GetPolygonSize(polIdx); polVertIdx++) {
				//頂点のindex
				auto vert = mesh->GetPolygonVertex(polIdx, polVertIdx);

			}
		}

		fbx_Manager->Destroy();
		return true;
	}
};

template<typename T>
std::unordered_map<std::string, typename ModelLoader<T>::ModelData>
ModelLoader<T>::m_modelDataMap = {};

template<typename T>
std::unordered_map<std::string, std::function<bool(char const*, std::span<T>&,
	std::span<unsigned short>&)>>
	ModelLoader<T>::m_modelLoaders = {
	{".fbx", ModelLoader<T>::LoadFBX}
};

}


*/