#pragma once

///<summary>
///�̱��� �������� ���� Ŭ������ �ش� Ŭ������ ��� �޾� ����Ѵ�.
///rawPtr�̹Ƿ� �ݵ�� Finalize �ʿ�
///</summary>

template <typename T>
class Singleton
{
protected:
	Singleton() = default;

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	static T* m_pInstance;

public:
	static T* GetInstance()
	{
		if (!m_pInstance)
			m_pInstance = new T;

		return m_pInstance;
	}

	virtual void Finalize() abstract
	{
		delete m_pInstance;
	}
};

template <typename T>
T* Singleton<T>::m_pInstance = nullptr;