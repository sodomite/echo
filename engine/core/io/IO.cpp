#include "engine/core/Util/AssertX.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/Util/TimeProfiler.h"
#include "IO.h"
#include "engine/core/Util/LogManager.h"

namespace Echo
{
	__ImplementSingleton(IO);

	// ���캯��
	IO::IO()
	{
		__ConstructSingleton;

		m_resFileSystem = EchoNew(FileSystem);
		m_userFileSystem = EchoNew(FileSystem);
	}

	// ��������
	IO::~IO()
	{
		reset();

		__DestructSingleton;
	}

	// ����������Դ·��
	void IO::setResPath(const String& resPath)
	{
		m_resFileSystem->setPath(resPath, "Res://");
	}

	// �����û���Դ·��
	void IO::setUserPath(const String& userPath)
	{
		m_userFileSystem->setPath(userPath, "User://");
	}

	// ����Դ
	DataStream* IO::open(const String& resourceName)
	{
		if (StringUtil::StartWith(resourceName, "Res://"))
		{
			DataStream* stream = m_resFileSystem->open(resourceName);
			if (!stream)
			{

			}

			return stream;
		}
		else if (StringUtil::StartWith(resourceName, "User://"))
		{

		}

		//// ����Դ
		//String  lcResourceName = resourceName;
		//if (!PathUtil::IsAbsolutePath(resourceName))
		//{
		//	StringUtil::LowerCase(lcResourceName);
		//}

		//// �������д浵
		//if ( !pArch)
		//	pArch = getArchiveByFileName(lcResourceName.c_str());

		//// ����Դ
		//if (pArch)
		//{
		//	DataStream* stream = pArch->open(lcResourceName);
		//	if (stream == nullptr)
		//	{
		//		EchoLogError("[DataStream* ResourceGroupManager::openResource:%d]::Cannot open a resource [%s] in [%s] ", __LINE__, resourceName.c_str(), archiveName); 
		//		return nullptr; 
		//	}

		//	return stream;
		//}

		EchoLogError("Cannot locate a resource [%s] ResourceGroupManager::openResource", resourceName.c_str());

		return  nullptr;
	}

	// ����
	void IO::reset()
	{
		EchoSafeDeleteContainer(m_archives, Archive);
		m_resourceIndexCaseSensitive.clear();
	}

	// ������Դ�浵
	void IO::addArchive(const String& name, const String& locType, bool isOverWrite)
	{
		TIME_PROFILE
		(
			// Get archive
			Archive* pArch = Archive::load(name, locType);
			{
				EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

				// Add to location list
				m_archives.push_back(pArch);

				// Index resources
				StringArray vec = pArch->list();
				for (StringArray::iterator it = vec.begin(); it != vec.end(); ++it)
					addToIndex(*it, pArch, isOverWrite);

				EchoLogInfo("Added resource [%s] location of type %s", name.c_str(), locType.c_str());
			}
		)
	}

	// �Ƴ���Դ�浵
	void IO::removeArchive(const String& name)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

		// Remove from location list
		for (ArchiveList::iterator li = m_archives.begin(); li != m_archives.end(); ++li)
		{
			Archive* pArch = *li;
			if (pArch->getName() == name)
			{
				// �Ƴ�����
				for (FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.begin(); rit != m_resourceIndexCaseSensitive.end();)
				{
					if (rit->second == pArch)
					{
						m_resourceIndexCaseSensitive.erase(rit++);
					}
					else
					{
						++rit;
					}
				}
				m_archives.erase(li);

				break;
			}
		}

		EchoLogInfo("Removed resource [%s] location", name.c_str());
	}

	// ��ȡ�ļ����ڵĴ浵
	Archive* IO::getArchiveByFileName(const char* fileName)
	{
		String  lcResourceName = fileName;
		StringUtil::LowerCase(lcResourceName);
		Archive*pArch = NULL;	
		{
			EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
			FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find(lcResourceName);
			if (rit != m_resourceIndexCaseSensitive.end())
			{
				pArch = rit->second;
			}
		}

		return pArch;
	}

	// ���ݴ浵���ƻ�ȡ�浵
	Archive* IO::getArchiveByName(const char* archiveName)
	{
		for (Archive* archive : m_archives)
		{
			if (archive->getName() == archiveName)
				return archive;
		}

		return NULL;
	}

	void IO::openResourceAsync(const String& resourceName, EchoOpenResourceCb callback)
	{
		String  lcResourceName = resourceName;
		StringUtil::LowerCase(lcResourceName);
		Archive* pArch = getArchiveByFileName(lcResourceName.c_str());
		if (pArch)
		{
			pArch->addAsyncCompleteCallback(resourceName, callback); 
			pArch->open(lcResourceName);
		}
	}

	// �Ƿ�Ϊ�첽��Դ
	bool IO::isAsync(const char* fileName)
	{
		Archive* archive = getArchiveByFileName(fileName);

		return archive ? archive->isAsync(fileName) : false;
	}

	// �ж���Դ�Ƿ����
	bool IO::isResourceExists(const String& resourceName)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

		// Try indexes first
		String lcResourceName = resourceName;
		StringUtil::LowerCase(lcResourceName);
		FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find(lcResourceName);
		if (rit != m_resourceIndexCaseSensitive.end())
		{
			// Found in the index
			return true;
		}

		return false;
	}

	// ��ȡ�ļ�λ��
	String IO::getFileLocation(const String& filename)
	{
		String ret;
        {
			String lowerCaseName = filename;
			StringUtil::LowerCase(lowerCaseName);

			EE_LOCK_AUTO_MUTEX

			Archive *pArch = NULL;
            FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find( lowerCaseName);
			if (rit != m_resourceIndexCaseSensitive.end())
			{
				// Found in the index
				pArch = rit->second;
                ret = pArch->location( lowerCaseName);
				return ret;
			}
        }

		EchoLogError("getFileLocation [%s] failed", filename.c_str());

		return ret; 
	}

	// ��ȡָ����׺���������ļ�
	void IO::listFilesWithExt( StringArray& oFiles, const char* extWithDot)
	{
		oFiles.clear();

		// �ֽ��׺
		StringArray exts = StringUtil::Split( extWithDot, "|");

		EE_LOCK_AUTO_MUTEX
		for( FileArchiveMapping::iterator itf = m_resourceIndexCaseSensitive.begin(); itf!=m_resourceIndexCaseSensitive.end(); itf++)
		{
			String fileExt = PathUtil::GetFileExt(itf->first, true);
			for (size_t i = 0; i < exts.size(); i++)
			{
				if (exts[i] == fileExt)
					oFiles.push_back(itf->first);
			}
		}
	}

		// ��ȡָ����׺���������ļ�
	void IO::foreach_archive_unsafe(std::function<bool (const Echo::String&, const int index)> callback)
	{
//		EE_LOCK_AUTO_MUTEX
		int i = 0;
		for( FileArchiveMapping::iterator itf = m_resourceIndexCaseSensitive.begin(); itf!=m_resourceIndexCaseSensitive.end(); itf++)
		{
			bool r = callback(itf->first, i++);
			if (!r)
				break;
		}
	}

	void IO::listFileWithPathAndExt(StringArray& oFiles, const char* path, const char* extWithDot)
	{
		oFiles.clear();

		// �ֽ��׺
		StringArray exts = StringUtil::Split(extWithDot, "|");
		
		String realPath = path;
		PathUtil::FormatPath(realPath, true);
		EE_LOCK_AUTO_MUTEX
		for (FileArchiveMapping::iterator itf = m_resourceIndexCaseSensitive.begin(); itf != m_resourceIndexCaseSensitive.end(); itf++)
		{
			String fileExt = PathUtil::GetFileExt(itf->first, true);
			String filePath = getFileLocation(itf->first);
			filePath = PathUtil::GetFileDirPath(filePath);
			PathUtil::FormatPathAbsolut(filePath, true);
			PathUtil::FormatPathAbsolut(realPath, true);
			for (size_t i = 0; i < exts.size(); i++)
			{
				if (exts[i] == fileExt && StringUtil::StartWith(filePath, realPath))
					oFiles.push_back(itf->first);
			}
		}
	}


	// ��������
	void IO::addToIndex(const String& filename, Archive* arch, bool isOverWrite)
	{
		EE_LOCK_AUTO_MUTEX
		Echo::String fileName = filename;
		StringUtil::LowerCase(fileName);
		if (m_resourceIndexCaseSensitive.find(fileName) == m_resourceIndexCaseSensitive.end() || isOverWrite)
		{
			// internal, assumes mutex lock has already been obtained
			m_resourceIndexCaseSensitive[fileName] = arch;
		}
		else
		{
			EchoLogError("file [%s] is not unique!", fileName.c_str());
		}
	}

	// ��ȡ�浵
	Archive* IO::FindFileArchive(const String& filename)
	{
		EE_LOCK_AUTO_MUTEX
		FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find(filename);
		if(rit != m_resourceIndexCaseSensitive.end())
		{
			return rit->second;
		}

		return NULL;
	}

	// �Ƴ�������
	void IO::removeFromIndex(const String& filename)
	{
		EE_LOCK_AUTO_MUTEX

		FileArchiveMapping::iterator it = m_resourceIndexCaseSensitive.find(filename);
		if (it != this->m_resourceIndexCaseSensitive.end())
			m_resourceIndexCaseSensitive.erase(it);
	}

	// �����ļ�(�����������ļ�)
	void IO::addFile(const String& archiveType, const String& fullPath)
	{
#ifdef ECHO_EDITOR_MODE
		Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath);
		StringUtil::LowerCase(fileName);
		if ( !isResourceExists(fileName) && !fileName.empty())
		{
			for (ArchiveList::iterator it = m_archives.begin(); it != m_archives.end(); it++)
			{
				Archive* archive = *it;
				if (archive->getType() == archiveType && archive->isCanAddFile(fullPath))
				{
					archive->addFile(fullPath);
					addToIndex(fileName, archive);
					break;
				}
			}
		}
		else if (fileName.empty())
		{
			EchoLogError("add file [%s] failed...", fileName.c_str());
		}
#endif
	}

	// �Ƴ��ļ�
	void IO::removeFile(const String& _fileName)
	{
		String fileName = _fileName;
		StringUtil::LowerCase(fileName);

		Archive* archive = FindFileArchive(fileName);
		if (archive)
		{
			archive->removeFile(fileName.c_str());

			// �Ƴ�����
			removeFromIndex(fileName);
		}		
	}
}
