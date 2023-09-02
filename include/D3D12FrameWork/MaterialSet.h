#pragma once
#include "D3D12FrameWork/Common.h"
#include "D3D12FrameWork/RPBufferController.h"
#include "D3D12FrameWork/RPBufferView.h"

namespace D3D12FrameWork {
	struct MaterialDescriptor
	{
		UINT RPIdx;
		std::string MaterialName;
	};
	struct MaterialSetDescriptor
	{
		std::vector<MaterialDescriptor> MaterialDescs;
	};
	class RPBufferController;
	class RPBufferView;
	//�}�e���A���̏ڍׂ͂Ƃɂ���vector�����Ă΂������ȂƂ��������͂��邯�ǂ���Ƃ͕ʂ�pso���w�肵�Ȃ��Ⴂ���Ȃ��DPDO�����O���������킩��₷����ȁ[
	class MaterialSet
	{
		friend class MaterialFactory;
	public:
		MaterialSet() = default;
		~MaterialSet();
		MaterialSet(MaterialSet&&) = delete;
		MaterialSet(MaterialSet const&) = delete;
		//int�Ŏw�肵�ă}�e���A�����B���Ă��邯�ǎg�p����rs��enum�Ƃ��ŎB���Ă�������������
		//�f�B�X�N���v�^��ݒ肵�ď�����������������ȁ[
		RPBufferController& GetController(uint32_t _rpIdx) {
			if (_rpIdx >= m_materialControllers.size()) {
				assert(false);
			}
			return m_materialControllers[_rpIdx];
		}
		RPBufferView& GetView(uint32_t _rpIdx) {
			if (_rpIdx >= m_materialViews.size()) {
				assert(false);
			}
			return m_materialViews[_rpIdx];
		}
		size_t Size() const {
			assert(m_materialControllers.size() == m_materialViews.size());
			return m_materialViews.size();
		}

	private:
		//������rp�̏��Ԓʂ�ɕ��ׂ�D������rpbuffer�ւ̎Q�Ƃł͂Ȃ�handler�������������������D
		std::vector<RPBufferController> m_materialControllers;
		std::vector<RPBufferView> m_materialViews;
		//lambda��buffer�ɂ��Ă�MaterialSet�����񂾂Ƃ��ɔj�󂷂�B
		std::vector<unsigned int> m_lambdaIdx;
	};
}


