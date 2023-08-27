#pragma once
#include <list>
#include "D3D12FrameWork/SwapChain.h"
namespace D3D12FrameWork {

	class KillObject
	{
	public:
		KillObject() {}
		virtual ~KillObject() {}
		bool CanKill() {
			m_PendingTime--;
			return m_PendingTime <= 0;
		}
		void SetPendingTime(int time) {
			m_PendingTime = time;
		}

	private:
		int m_PendingTime = DX12Settings::BUFFER_COUNT;

	};

	template<typename T>
	class DeleteKillObject
		:public KillObject
	{
	public:
		DeleteKillObject(T* object)
			:KillObject()
			,m_pObject(object) {}
		~DeleteKillObject() {
			delete m_pObject;
			m_pObject = nullptr;
		}


	private:
		T* m_pObject;

	};

	template<typename T>
	class ReleaseKillObject
		:public KillObject
	{
	public:
		ReleaseKillObject(T* object)
			:KillObject()
			,m_pObject(object) {}
		~ReleaseKillObject() {
			m_pObject->Release();
			m_pObject = nullptr;
		}

	private:
		T* m_pObject;

	};


	class DeathList
	{
	public:
		~DeathList() {
			Destroy();
		}
		template<typename T>
		void PendingKill(KillObject* pPendingObj) {
			m_deathList.push_back(pPendingObj);
		}
		template<typename T>
		void PendingRelease(T* pObj) {
			auto pKObj = new ReleaseKillObject<T>(pObj);
			PendingKill<T>(pKObj);
		}
		template<typename T>
		void PendingDelete(T* pObj) {
			auto pKObj = new DeleteKillObject<T>(pObj);
			PendingKill<T>(pKObj);
		}

		void Destroy() {
			auto it = m_deathList.begin();
			while (it != m_deathList.end())
			{
				auto p = *it;
				m_deathList.erase(it);
				delete p;
				p = nullptr;
			}
		}

		void SyncKill() {
			auto it = m_deathList.begin();

			while (it != m_deathList.end())
			{
				auto p = *it;
				if (p->CanKill()) {
					it = m_deathList.erase(it);
					delete p;
					p = nullptr;
				}
				else {
					it++;
				}
			}
		}
	private:
		std::list<KillObject*> m_deathList;

	};

}
