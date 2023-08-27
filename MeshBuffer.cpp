#include "D3D12FrameWork/MeshBuffer.h"
#include "D3D12FrameWork/ShaderBindInfo.h"

namespace D3D12FrameWork {
	bool
		MeshBuffer::Init(
			ShaderInputDescs const& inputDesc,
			bool useIndex
		) {
		//‚ ‚Ü‚èİ’è‚·‚×‚«‚±‚Æ‚ª‚È‚¢‚ËBB
		m_numInstances = 1;
		auto size = inputDesc.GetSize();
		auto vb_size = inputDesc.GetElement(size - 1).Register;
		m_vertexBuffs.resize(vb_size+1);
		for (auto& vb : m_vertexBuffs) {
			vb.reset(new VertexBuffer());
		}
		if (useIndex) {//indexg‚¤‚È‚ç‰Šú‰»

		}

		return true;
	}

}
